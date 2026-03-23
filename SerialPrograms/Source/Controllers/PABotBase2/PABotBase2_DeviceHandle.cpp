/*  PABotBase2 Device Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/StreamConnections/PABotBase2_MessageDumper.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Controllers/SerialPABotBase/SerialPABotBase.h"
#include "PABotBase2_DeviceHandle.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace PABotBase2{



DeviceHandle::DeviceHandle(
    CancellableScope* parent,
    Logger& logger,
    ReliableStreamConnectionPushing& connection
)
    : m_logger(logger)
    , m_connection(connection)
{
    connection.add_listener(*this);
    if (parent){
        attach(*parent);
    }
}
DeviceHandle::~DeviceHandle(){
    detach();
    cancel(nullptr);
    m_connection.remove_listener(*this);
}

bool DeviceHandle::cancel(std::exception_ptr exception) noexcept{
    bool already_cancelled = CancellableScope::cancel(std::move(exception));
    if (already_cancelled){
        return true;
    }
    {
        std::lock_guard<Mutex> lg(m_lock);
        for (auto& item : m_pending_requests){
            item.second.cv.notify_all();
        }
    }
    m_cv.notify_all();
    return false;
}



void DeviceHandle::throw_incompatible_protocol(){
    m_logger.log(
        "[MLC]: Remote Protocol: " + std::to_string(m_device_protocol) + " (incompatible)",
        COLOR_RED
    );

    throw SerialProtocolException(
        m_logger, PA_CURRENT_FUNCTION,
        "Incompatible MLC protocol. Device: " + std::to_string(m_device_protocol) + "<br>"
        "Please flash your microcontroller (e.g. ESP32, Pico W, Arduino) <br>"
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
        m_logger.log("[MLC]: Device ID: " + tostr_hex(m_device_id), COLOR_BLUE);
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
        }
    }
    m_logger.log(
        "[MLC]: Remote Protocol: " + std::to_string(m_device_protocol) + " (compatible)",
        COLOR_RED
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
    m_command_queue_size = (uint8_t)query_u32(PABB2_MESSAGE_OPCODE_CQ_CAPACITY);
    m_logger.log("[MLC]: Command Queue Size: " + std::to_string(m_command_queue_size), COLOR_BLUE);

    //  For now we don't need to use that much queue size.
    m_command_queue_size = std::min<uint8_t>(m_command_queue_size, 32);

    m_logger.Logger::log("Setting queue size to: " + std::to_string(m_command_queue_size));
}
void DeviceHandle::connect(){
    query_protocol();

    m_device_firmware_version = query_u32(PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION);
    m_logger.log("[MLC]: Firmware Version: " + std::to_string(m_device_firmware_version), COLOR_BLUE);

    m_device_name = query_data(PABB2_MESSAGE_OPCODE_DEVICE_NAME);
    m_logger.log("[MLC]: Device Name: " + m_device_name, COLOR_BLUE);

    query_controller_list();
    query_command_queue();
}


void DeviceHandle::send_request(pabb2_MessageHeader& request){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        throw_if_cancelled();

        request.id = m_seqnum;

        //  Wait until the slot is available.
        auto iter = m_pending_requests.find(request.id);
        if (iter != m_pending_requests.end()){
            m_cv.wait(lg);
            continue;
        }

        m_pending_requests[request.id];
        m_seqnum++;
        break;
    }

    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("[MLC]: Sending: " + tostr(&request), COLOR_DARKGREEN);
    }

    m_connection.reliable_send(&request, request.message_bytes);
}
std::string DeviceHandle::wait_for_response(uint8_t id){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        throw_if_cancelled();

        auto iter = m_pending_requests.find(id);
        if (iter == m_pending_requests.end()){
            throw InternalProgramError(
                &m_logger,
                PA_CURRENT_FUNCTION,
                "Attempted to wait for an ID that doesn't exist."
            );
        }

        if (iter->second.response.empty()){
            iter->second.cv.wait(lg);
            continue;
        }

        std::string str = std::move(iter->second.response);
        m_pending_requests.erase(iter);
        return str;
    }
}

uint32_t DeviceHandle::query_u32(uint8_t opcode){
    pabb2_MessageHeader request;
    request.message_bytes = sizeof(pabb2_MessageHeader);
    request.opcode = opcode;

    send_request(request);

    std::string response = wait_for_response(request.id);
    if (response.size() != sizeof(pabb2_Message_Response_u32)){
        throw InternalProgramError(
            &m_logger,
            PA_CURRENT_FUNCTION,
            "Expected response length of: " + std::to_string(sizeof(pabb2_Message_Response_u32))
        );
    }

    const pabb2_Message_Response_u32* msg = (const pabb2_Message_Response_u32*)response.c_str();
    return msg->data;
}
std::string DeviceHandle::query_data(uint8_t opcode){
    pabb2_MessageHeader request;
    request.message_bytes = sizeof(pabb2_MessageHeader);
    request.opcode = opcode;

    send_request(request);

    std::string response = wait_for_response(request.id);
    if (response.size() < sizeof(pabb2_Message_Response_Data)){
        throw InternalProgramError(
            &m_logger,
            PA_CURRENT_FUNCTION,
            "Response length of " + std::to_string(response.size()) + " is too short."
        );
    }

    const pabb2_Message_Response_Data* msg = (const pabb2_Message_Response_Data*)response.c_str();
    return std::string((const char*)(msg + 1), msg->message_bytes - sizeof(pabb2_Message_Response_Data));
}





void DeviceHandle::on_recv(const void* data, size_t bytes){
    cout << "DeviceHandle::on_recv()" << endl;
    m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + bytes);

    while (true){
        //  Header is incomplete.
        if (m_buffer.size() < sizeof(pabb2_MessageHeader)){
            return;
        }

        //  Message is incomplete.
        uint16_t message_size;
        std::copy(m_buffer.begin(), m_buffer.begin() + 2, (char*)&message_size);
        if (m_buffer.size() < message_size){
            return;
        }

        auto iter_s = m_buffer.begin();
        auto iter_e = iter_s + message_size;
        std::string message(iter_s, iter_e);
        m_buffer.erase(iter_s, iter_e);

        const pabb2_MessageHeader* header = (const pabb2_MessageHeader*)message.c_str();

        bool log_everything = GlobalSettings::instance().LOG_EVERYTHING;

        if (log_everything){
            m_logger.log("[MLC]: Receive: " + tostr(header), COLOR_PURPLE);
        }

        //  Now we can process the message.
        switch (header->opcode){
        case PABB2_MESSAGE_OPCODE_INVALID:
        case PABB2_MESSAGE_OPCODE_REQUEST_DROPPED:
            if (!log_everything){
                m_logger.log("[MLC]: Receive: " + tostr(header), COLOR_PURPLE);
            }
            continue;
        case PABB2_MESSAGE_OPCODE_RET:
        case PABB2_MESSAGE_OPCODE_RET_U32:
        case PABB2_MESSAGE_OPCODE_RET_DATA:
        {
            std::lock_guard<Mutex> lg(m_lock);
            auto iter = m_pending_requests.find(header->id);
            if (iter == m_pending_requests.end()){
                m_logger.log("[MLC]: Received response for unknown ID: " + std::to_string(header->id));
                continue;
            }
            iter->second.response = std::move(message);
            iter->second.cv.notify_all();
            continue;
        }
        }

        //  TODO: Process device-specific messages.
    }


}







}
}
