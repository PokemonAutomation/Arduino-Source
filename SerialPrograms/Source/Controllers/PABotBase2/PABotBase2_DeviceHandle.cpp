/*  PABotBase2 Device Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/PABotBase2/PABotBase2CC_MessageDumper.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Controllers/ControllerTypeStrings.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "PABotBase2_DeviceHandle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace PABotBase2{



DeviceHandle::DeviceHandle(
    CancellableScope* parent,
    Logger& logger,
    ReliableStreamConnectionPushing& connection
)
    : m_logger(logger)
    , m_connection(connection)
    , m_command_queue(logger, *this, connection, m_message_loggers)
{
    connection.add_listener(*this);
    if (parent){
        attach(*parent);
    }
}
DeviceHandle::~DeviceHandle(){
    {
        std::unique_lock<Mutex> lg(m_lock);
        try{
            m_logger.log(
                "DeviceHandle::~DeviceHandle(): Waiting for " + std::to_string(m_pending_requests.size()) + " request(s) to finish."
            );
        }catch (...){}
        bool ok = m_cv.wait_for(
            lg, std::chrono::milliseconds(100),
            [this]{
                return m_pending_requests.empty();
            }
        );
        if (!ok){
            try{
                m_logger.log(
                    "DeviceHandle::~DeviceHandle(): Timed out waiting for " + std::to_string(m_pending_requests.size()) + " request(s) to finish.",
                    COLOR_RED
                );
            }catch (...){}
        }
    }

    detach();
    cancel(nullptr);
    m_connection.remove_listener(*this);
}


void DeviceHandle::add_message_handler(
    uint8_t opcode,
    std::function<void(const MessageHeader*)> handler
){
    auto ret = m_message_handlers.emplace(opcode, std::move(handler));
    if (!ret.second){
        throw InternalProgramError(
            &m_logger, PA_CURRENT_FUNCTION,
            "Duplicate Message Opcode: 0x" + tostr_hex(opcode)
        );
    }
}
bool DeviceHandle::cancel(std::exception_ptr exception) noexcept{
    if (CancellableScope::cancel(std::move(exception))){
        return true;
    }
    {
        std::lock_guard<Mutex> lg(m_lock);
    }
    m_cv.notify_all();
    return false;
}


std::string DeviceHandle::dump_pending_requests() const{
    std::string ret = "Pending Requests:\n";
    std::lock_guard<Mutex> lg(m_lock);
    for (const auto& item : m_pending_requests){
        ret += std::to_string(item.first);
        ret += " : ";
        ret += tostr_hexbytes(item.second.data(), item.second.size());
        ret += "\n";
    }
    return ret;
}




void DeviceHandle::throw_incompatible_protocol(){
    m_logger.log(
        "[MLC]: Remote Protocol: " + std::to_string(m_device_protocol) + " (incompatible)",
        COLOR_RED
    );

    throw SerialProtocolException(
        m_logger, PA_CURRENT_FUNCTION,
        "Incompatible MLC protocol. Device: " + std::to_string(m_device_protocol) + "<br>"
        "Please flash your microcontroller (e.g. ESP32, Pico W...) <br>"
        "with the .bin/.uf2/.hex that came with this version of the program.<br>" +
        make_text_url(ONLINE_DOC_URL_BASE + "SetupGuide/Reflash.html", "See documentation for more details.")
    );
}
void DeviceHandle::query_protocol(){
    const std::map<pabb_ProgramID, uint8_t>* PROGRAMS;
    {
        m_device_protocol = query_u32(PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION);
        const std::map<uint32_t, std::map<pabb_ProgramID, uint8_t>>& SUPPORTED_VERSIONS =
            SerialPABotBase::SUPPORTED_VERSIONS2();
        auto iter = SUPPORTED_VERSIONS.find(m_device_protocol / 100);
        if (iter == SUPPORTED_VERSIONS.end()){
            throw_incompatible_protocol();
        }
        PROGRAMS = &iter->second;
    }
    {
        m_device_id = query_u32(PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER);
        m_logger.log("[MLC]: Device ID: 0x" + tostr_hex(m_device_id), COLOR_BLUE);
        auto iter = PROGRAMS->find(m_device_id);
        if (iter == PROGRAMS->end()){
            m_logger.Logger::log(
                "Unrecognized Program ID: (0x" + tostr_hex(m_device_id) + ") for this protocol version. "
                "Compatibility is not guaranteed.",
                COLOR_RED
            );
            iter = PROGRAMS->find(PABB_PID_UNSPECIFIED);
            if (iter == PROGRAMS->end()){
                throw_incompatible_protocol();
            }
        }else{
            uint8_t minor_version = iter->second;
            if (m_device_protocol % 100 < minor_version){
                throw_incompatible_protocol();
            }
        }
    }
    m_logger.log(
        "[MLC]: Remote Protocol: " + std::to_string(m_device_protocol) + " (compatible)",
        COLOR_BLUE
    );
}
void DeviceHandle::query_controller_list(){
    std::string response = query_data(PABB2_MESSAGE_OPCODE_CONTROLLER_LIST);
    if (response.size() % sizeof(pabb_ControllerID) != 0){
        throw InternalProgramError(
            &m_logger,
            PA_CURRENT_FUNCTION,
            "Controller list query expects a response length divisible by " + std::to_string(sizeof(pabb_ControllerID)) + "."
        );
    }

    size_t length = response.size() / sizeof(pabb_ControllerID);
    const pabb_ControllerID* list = (const pabb_ControllerID*)response.c_str();

    std::string str;
    bool first = true;
    for (size_t c = 0; c < length; c++){
        pabb_ControllerID id;
        memcpy(&id, list + c, sizeof(pabb_ControllerID));
        if (!first){
            str += ", ";
        }
        first = false;
        str += "0x" + tostr_hex(id);
        if (SerialPABotBase::controller_is_valid(id)){
            m_controller_list.emplace_back(SerialPABotBase::id_to_controller_type(id));
        }
    }
    m_logger.Logger::log("Checking Controller List... (" + str + ")");
}
void DeviceHandle::query_command_queue(){
    uint32_t command_queue_size = query_u32(PABB2_MESSAGE_OPCODE_CQ_CAPACITY);
    m_logger.log("[MLC]: Command Queue Size: " + std::to_string(command_queue_size), COLOR_BLUE);

    //  Clip the command queue sizes.
    command_queue_size = std::max<uint32_t>(command_queue_size, 4);
    command_queue_size = std::min<uint32_t>(command_queue_size, 255);

    //  Don't let it get too large since we don't need it.
    command_queue_size = std::min<uint8_t>(
        command_queue_size,
        GlobalSettings::instance().COMMAND_QUEUE_LIMIT
    );

    m_logger.Logger::log("Setting queue size to: " + std::to_string(command_queue_size));
    m_command_queue.set_command_queue_size((uint8_t)command_queue_size);
}
void DeviceHandle::set_logging_flag(uint32_t flag){
    PABotBase2::Message_u32 message;
    message.message_bytes = sizeof(message);
    message.opcode = PABB2_MESSAGE_OPCODE_SET_LOGGING_FLAG;
    message.data = flag;
    send_request_with_no_response(message);
}
void DeviceHandle::connect(){
    query_protocol();

    m_device_firmware_version = query_u32(PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION);
    m_logger.log("[MLC]: Firmware Version: " + std::to_string(m_device_firmware_version), COLOR_BLUE);

    m_device_name = query_data(PABB2_MESSAGE_OPCODE_DEVICE_NAME);
    m_logger.log("[MLC]: Device Name: " + m_device_name, COLOR_BLUE);

    query_controller_list();
    query_command_queue();

    if (m_device_firmware_version >= 2026050100){
        set_logging_flag(GlobalSettings::instance().DEVICE_LOGGING_FLAG);
    }
}
void DeviceHandle::try_set_controller_type(
    ControllerType controller_type,
    bool clear_settings
) noexcept{
    PABotBase2::Message_u32 message;
    message.message_bytes = sizeof(message);
    message.opcode = clear_settings
        ? PABB2_MESSAGE_OPCODE_RESET_TO_CONTROLLER
        : PABB2_MESSAGE_OPCODE_CHANGE_CONTROLLER_MODE;
    try{
        message.data = SerialPABotBase::controller_type_to_id(controller_type);
        try_send_request_with_response(message, std::chrono::milliseconds(100));
    }catch (...){}
}

ControllerType DeviceHandle::refresh_controller_type(){
    m_logger.log("Reading Controller Mode...");
    uint32_t type_id = query_u32(PABB2_MESSAGE_OPCODE_READ_CONTROLLER_MODE);

    ControllerType current_controller = SerialPABotBase::id_to_controller_type(type_id);
    m_logger.log(
        "Reading Controller Mode... Mode = " +
        CONTROLLER_TYPE_STRINGS.get_string(current_controller)
    );
//    m_current_controller.store(current_controller, std::memory_order_release);
    return current_controller;
}

void DeviceHandle::send_request_with_no_response(MessageHeader& request){
    request.id = 0;
    std::unique_lock<Mutex> lg(m_lock);
    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &request);
    m_connection.reliable_send_all_or_nothing(&request, request.message_bytes, WallDuration::max());
}
std::optional<uint8_t> DeviceHandle::try_send_request_with_no_response(
    MessageHeader& request, WallDuration timeout
) noexcept{
    request.id = 0;
    std::unique_lock<Mutex> lg(m_lock);
    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &request);
    return m_connection.reliable_send_all_or_nothing(&request, request.message_bytes, timeout);
}
uint8_t DeviceHandle::send_request_with_response(MessageHeader& request){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        throw_if_cancelled();

        request.id = m_request_seqnum;

        //  Wait until the slot is available.
        auto iter = m_pending_requests.find(request.id);
        if (iter != m_pending_requests.end()){
            m_cv.wait(lg);
            continue;
        }

        m_pending_requests[request.id];
        m_request_seqnum++;
        break;
    }

    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &request);

    try{
        m_connection.reliable_send_all_or_nothing(&request, request.message_bytes, WallDuration::max());
    }catch (...){
        m_pending_requests.erase(request.id);
        m_request_seqnum--;
        throw;
    }

    return request.id;
}
std::optional<uint8_t> DeviceHandle::try_send_request_with_response(
    MessageHeader& request, WallDuration timeout
) noexcept{
    std::unique_lock<Mutex> lg(m_lock);
    try{
        while (true){
            throw_if_cancelled();

            request.id = m_request_seqnum;

            //  Wait until the slot is available.
            auto iter = m_pending_requests.find(request.id);
            if (iter != m_pending_requests.end()){
                m_cv.wait(lg);
                continue;
            }

            m_pending_requests[request.id];
            m_request_seqnum++;
            break;
        }
    }catch (...){
        return {};
    }

    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &request);

    try{
        if (m_connection.reliable_send_all_or_nothing(&request, request.message_bytes, timeout)){
            return request.id;
        }
    }catch (...){
        m_pending_requests.erase(request.id);
        m_request_seqnum--;
    }

    return {};
}
std::string DeviceHandle::wait_for_request_response(uint8_t id, WallDuration timeout){
    WallClock deadline = timeout == WallDuration::max()
        ? WallClock::max()
        : current_time() + timeout;
    std::unique_lock<Mutex> lg(m_lock);
    while (current_time() < deadline){
        throw_if_cancelled();

        //  Request doesn't exist.
        auto iter = m_pending_requests.find(id);
        if (iter == m_pending_requests.end()){
            throw InternalProgramError(
                &m_logger,
                PA_CURRENT_FUNCTION,
                "Attempted to wait for a request ID that doesn't exist."
            );
        }

        if (iter->second.empty()){
            m_cv.wait_until(lg, deadline);
            continue;
        }

        std::string str = std::move(iter->second);
        m_pending_requests.erase(iter);
        return str;
    }
    return "";
}

uint32_t DeviceHandle::query_u32(uint8_t opcode){
    MessageHeader request;
    request.message_bytes = sizeof(MessageHeader);
    request.opcode = opcode;

    send_request_with_response(request);

    std::string response = wait_for_request_response(request.id);
    if (response.size() != sizeof(Message_u32)){
        throw InternalProgramError(
            &m_logger,
            PA_CURRENT_FUNCTION,
            "Expected response length of: " + std::to_string(sizeof(Message_u32))
        );
    }

    const Message_u32* msg = (const Message_u32*)response.c_str();
    return msg->data;
}
std::string DeviceHandle::query_data(uint8_t opcode){
    MessageHeader request;
    request.message_bytes = sizeof(MessageHeader);
    request.opcode = opcode;

    send_request_with_response(request);

    std::string response = wait_for_request_response(request.id);
    if (response.size() < sizeof(MessageHeader)){
        throw InternalProgramError(
            &m_logger,
            PA_CURRENT_FUNCTION,
            "Response length of " + std::to_string(response.size()) + " is too short."
        );
    }

    const MessageHeader* msg = (const MessageHeader*)response.c_str();
    return std::string((const char*)(msg + 1), msg->message_bytes - sizeof(MessageHeader));
}





void DeviceHandle::on_recv(const void* data, size_t bytes){
//    cout << "DeviceHandle::on_recv()" << endl;

    if (m_stream_corrupted){
        return;
    }

    m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + bytes);

    while (true){
        //  Header is incomplete.
        if (m_buffer.size() < sizeof(MessageHeader)){
            return;
        }

        //  Message is incomplete.
        uint16_t message_size;
        std::copy(m_buffer.begin(), m_buffer.begin() + 2, (char*)&message_size);
        if (message_size < sizeof(MessageHeader)){
            m_logger.log("[MLC]: Corrupted Stream: Message Length =" + std::to_string(message_size), COLOR_RED);
            m_stream_corrupted = true;
            return;
        }
        if (m_buffer.size() < message_size){
            return;
        }

        auto iter_s = m_buffer.begin();
        auto iter_e = iter_s + message_size;
        std::string message(iter_s, iter_e);
        m_buffer.erase(iter_s, iter_e);

        const MessageHeader* header = (const MessageHeader*)message.c_str();

        m_message_loggers.log_recv(m_logger, GlobalSettings::instance().LOG_EVERYTHING, header);

        //  Now we can process the message.
        switch (header->opcode){
        case PABB2_MESSAGE_OPCODE_INVALID:
        case PABB2_MESSAGE_OPCODE_REQUEST_DROPPED:
            continue;
        case PABB2_MESSAGE_OPCODE_RET:
        case PABB2_MESSAGE_OPCODE_RET_U32:
        case PABB2_MESSAGE_OPCODE_RET_DATA:{
            {
                std::lock_guard<Mutex> lg(m_lock);
                auto iter = m_pending_requests.find(header->id);
                if (iter == m_pending_requests.end()){
                    m_logger.log("[MLC]: Received request response for unknown ID: " + std::to_string(header->id));
                    continue;
                }
                iter->second = std::move(message);
            }
            m_cv.notify_all();
            continue;
        }
        case PABB2_MESSAGE_OPCODE_LOG_STRING:
        case PABB2_MESSAGE_OPCODE_LOG_LABEL_H32:
        case PABB2_MESSAGE_OPCODE_LOG_LABEL_U32:
        case PABB2_MESSAGE_OPCODE_LOG_LABEL_I32:
            continue;
        case PABB2_MESSAGE_OPCODE_CQ_COMMAND_FINISHED:{
            m_command_queue.report_command_finished(*header);
            continue;
        }
        }

        auto iter = m_message_handlers.find(header->opcode);
        if (iter == m_message_handlers.end()){
            return;
        }

//        MessageHandler& handler = *iter->second;
//        handler.assert_is_valid(m_logger, header);
//        handler.on_recv(m_logger, header);

        iter->second(header);
    }


}







}
}
