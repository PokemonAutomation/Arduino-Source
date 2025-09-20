/*  Serial Port (PABotBase) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#include <QSerialPortInfo>
#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/PrettyPrint.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "ClientSource/Connection/SerialConnection.h"
#include "ClientSource/Connection/PABotBase.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/ControllerTypeStrings.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h"
#include "SerialPABotBase.h"
#include "SerialPABotBase_Connection.h"

namespace PokemonAutomation{
namespace SerialPABotBase{





SerialPABotBase_Connection::SerialPABotBase_Connection(
    Logger& logger,
    const QSerialPortInfo* port,
    bool set_to_null_controller
)
    : m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
{
    set_status_line0("Not Connected", COLOR_RED);

    //  No port selected.
    if (port == nullptr || port->isNull()){
        return;
    }

    //  Prolific is banned.
    if (port->description().indexOf("Prolific") != -1){
        QMessageBox box;
        box.critical(
            nullptr,
            "Error",
            "Cannot select Prolific controller.<br><br>"
            "Prolific controllers do not work for Arduino and similar microcontrollers.<br>"
            "You were warned of this in the setup instructions. Please buy a CP210x controller instead."
        );
        std::string text = "Cannot connect to Prolific controller.";
        m_logger.log(text, COLOR_RED);
        set_status_line0(text, COLOR_RED);
        return;
    }

    m_device_name = port->portName().toStdString();
    std::string error;
    try{
        set_status_line0("Connecting...", COLOR_DARKGREEN);
        std::unique_ptr<SerialConnection> connection(new SerialConnection(port->systemLocation().toStdString(), PABB_BAUD_RATE));
        m_botbase.reset(new PABotBase(m_logger, std::move(connection), nullptr));
    }catch (const ConnectionException& e){
        error = e.message();
    }catch (const SerialProtocolException& e){
        error = e.message();
    }catch (const ProgramCancelledException& e){
        error = e.message();
    }
    if (!error.empty()){
        set_status_line0("Unable to open port.", COLOR_RED);
        return;
    }

    m_status_thread = std::thread(
        run_with_catch,
        "SerialPABotBase_Connection::thread_body()",
        [=, this]{ thread_body(set_to_null_controller); }
    );
}
SerialPABotBase_Connection::~SerialPABotBase_Connection(){
    m_ready.store(false, std::memory_order_release);
//    signal_pre_not_ready();
    if (m_botbase == nullptr){
        return;
    }
    m_botbase->stop();
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }
    if (m_status_thread.joinable()){
        m_status_thread.join();
    }
    m_botbase.reset();
}



BotBaseController* SerialPABotBase_Connection::botbase(){
    BotBaseController* ret = m_botbase.get();
    if (ret == nullptr){
        m_logger.log("SerialPABotBase_Connection::botbase() called with null botbase...", COLOR_RED);
    }
    return ret;
}
ControllerType SerialPABotBase_Connection::refresh_controller_type(){
    m_logger.log("Reading Controller Mode...");
    uint32_t type_id = read_controller_mode(*botbase());
    ControllerType current_controller = id_to_controller_type(type_id);
    m_logger.log(
        "Reading Controller Mode... Mode = " +
        CONTROLLER_TYPE_STRINGS.get_string(current_controller)
    );
    m_current_controller.store(current_controller, std::memory_order_release);
    return current_controller;
}



void SerialPABotBase_Connection::process_queue_size(){
    m_logger.log("Requesting queue size...");
    uint8_t queue_size = device_queue_size(*m_botbase);
    m_logger.Logger::log("Requesting queue size... Queue Size = " + std::to_string(queue_size));

    //  For now we don't need to use that much queue size.
    queue_size = std::min<uint8_t>(queue_size, 32);

    m_logger.Logger::log("Setting queue size to: " + std::to_string(queue_size));
    m_botbase->set_queue_limit(queue_size);
}

void SerialPABotBase_Connection::throw_incompatible_protocol(){
    throw SerialProtocolException(
        m_logger, PA_CURRENT_FUNCTION,
        "Incompatible protocol. Device: " + std::to_string(m_protocol) + "<br>"
        "Please flash the .hex/.bin that came with this version of the program."
    );
}
ControllerType SerialPABotBase_Connection::process_device(bool set_to_null_controller){
    //  Protocol Version
    const std::map<pabb_ProgramID, uint8_t>* PROGRAMS;
    {
        m_logger.Logger::log("Checking Protocol Version...");
        m_protocol = protocol_version(*m_botbase);
        m_logger.Logger::log("Checking Protocol Version... (" + std::to_string(m_protocol) + ")");
        auto iter = SUPPORTED_VERSIONS().find(m_protocol / 100);
        if (iter == SUPPORTED_VERSIONS().end()){
            throw_incompatible_protocol();
        }
        PROGRAMS = &iter->second;
    }

    //  Program ID
    {
        m_logger.Logger::log("Checking Program ID...");
        m_program_id = program_id(*m_botbase);
        m_logger.Logger::log("Checking Program ID... (0x" + tostr_hex(m_program_id) + ")");
        auto iter = PROGRAMS->find(m_program_id);
        if (iter == PROGRAMS->end()){
            m_logger.Logger::log(
                "Unrecognized Program ID: (0x" + tostr_hex(m_program_id) + ") for this protocol version. "
                "Compatibility is not guaranteed.",
                COLOR_RED
            );
            iter = PROGRAMS->find(PABB_PID_UNSPECIFIED);
            if (iter == PROGRAMS->end()){
                throw_incompatible_protocol();
            }
        }
        if (m_protocol % 100 < iter->second){
            throw_incompatible_protocol();
        }
    }

    //  Firmware Version
    {
        m_logger.Logger::log("Checking Firmware Version...");
        m_version = program_version(*m_botbase);
        m_logger.Logger::log("Checking Firmware Version... (" + std::to_string(m_version) + ")");
    }

    //  Program Name
    {
        m_logger.Logger::log("Checking Program Name...");
        m_program_name = program_name(*m_botbase);
        m_logger.Logger::log("Checking Program Name... (" + m_program_name + ")");
    }

    //  Controller List
    {
        m_logger.Logger::log("Checking Controller List...");
        std::string str;
        bool first = true;
        for (pabb_ControllerID id : SerialPABotBase::controller_list(*m_botbase)){
            if (!first){
                str += ", ";
            }
            first = false;
            str += "0x" + tostr_hex(id);
            m_controller_list.emplace_back(id_to_controller_type(id));
        }
        m_logger.Logger::log("Checking Controller List... (" + str + ")");
    }

    //  Queue Size
    process_queue_size();

    //  Current Controller
    ControllerType current_controller = refresh_controller_type();

    if (set_to_null_controller && current_controller != ControllerType::None){
        m_botbase->issue_request_and_wait(
            DeviceRequest_change_controller_mode(PABB_CID_NONE),
            nullptr
        );
        current_controller = refresh_controller_type();
    }

    return current_controller;
}



void SerialPABotBase_Connection::thread_body(bool set_to_null_controller){
    using namespace PokemonAutomation;

    m_botbase->set_sniffer(&m_logger);

    //  Connect
    {
        std::string error;
        try{
            m_botbase->connect();
        }catch (InvalidConnectionStateException&){
            m_botbase->stop();
            set_status_line0("");
            return;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (!error.empty()){
            m_botbase->stop();
            set_status_line0(error, COLOR_RED);
            return;
        }
    }

    //  Check protocol and version.

    {
        std::string error;
        try{
            process_device(set_to_null_controller);

            //  Stop pending commands.
            m_botbase->stop_all_commands();

            std::string text = m_program_name + " (" + std::to_string(m_version) + ")";
            set_status_line0(text, theme_friendly_darkblue());
            declare_ready();
        }catch (InvalidConnectionStateException&){
            return;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (!error.empty()){
//            m_ready.store(false, std::memory_order_relaxed);
            set_status_line0(error, COLOR_RED);
//            signal_pre_not_ready();
            m_botbase->stop();
            return;
        }
    }
}



}
}
