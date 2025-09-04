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
#include "SerialPABotBase_PostConnectActions.h"
#include "SerialPABotBase_Connection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{





SerialPABotBase_Connection::SerialPABotBase_Connection(
    Logger& logger,
    const QSerialPortInfo* port,
    std::optional<ControllerType> change_controller,
    bool clear_settings
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
        [=, this]{ thread_body(change_controller, clear_settings); }
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



ControllerModeStatus SerialPABotBase_Connection::controller_mode_status() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_mode_status;
}



const std::set<pabb_ProgramID>& SerialPABotBase_Connection::get_programs_for_protocol(uint32_t protocol){
    //  (protocol_requested / 100) == (protocol_device / 100)
    //  (protocol_requested % 100) <= (protocol_device % 100)
    auto iter = SUPPORTED_VERSIONS().upper_bound(protocol);
    if (iter == SUPPORTED_VERSIONS().begin()){
        throw SerialProtocolException(
            m_logger, PA_CURRENT_FUNCTION,
            "Incompatible protocol. Device: " + std::to_string(protocol) + "<br>"
            "Please flash the .hex/.bin that came with this version of the program."
        );
    }
    --iter;
    if (iter->first < protocol / 100 * 100){
        throw SerialProtocolException(
            m_logger, PA_CURRENT_FUNCTION,
            "Incompatible protocol. Device: " + std::to_string(protocol) + "<br>"
            "Please flash the .hex/.bin that came with this version of the program."
        );
    }

    return iter->second;
}

#if 0
const std::vector<ControllerType>& SerialPABotBase_Connection::get_controllers_for_program(
    const std::map<uint32_t, std::vector<ControllerType>>& available_programs,
    uint32_t program_id
){
    auto iter = available_programs.find(program_id);
    if (iter == available_programs.end()){
        throw SerialProtocolException(
            m_logger, PA_CURRENT_FUNCTION,
            "Unrecognized Program ID: " + std::to_string(program_id) + "<br>"
            "Please install the firmware that came with this version of the program."
        );
    }
    return iter->second;
}
#endif

void SerialPABotBase_Connection::process_queue_size(){
    m_logger.log("Requesting queue size...");
    uint8_t queue_size = device_queue_size(*m_botbase);
    m_logger.Logger::log("Requesting queue size... Queue Size = " + std::to_string(queue_size));

    //  For now we don't need to use that much queue size.
    queue_size = std::min<uint8_t>(queue_size, 32);

    m_logger.Logger::log("Setting queue size to: " + std::to_string(queue_size));
    m_botbase->set_queue_limit(queue_size);
}
ControllerType SerialPABotBase_Connection::get_controller_type(
    const std::vector<ControllerType>& available_controllers
){
    m_logger.log("Reading Controller Mode...");
    ControllerType current_controller = ControllerType::None;
    if (available_controllers.size() == 1){
        current_controller = available_controllers[0];
    }else if (available_controllers.size() > 1){
        uint32_t type_id = read_controller_mode(*m_botbase);
//        cout << "type_id = " << type_id << endl;
        current_controller = id_to_controller_type(type_id);
    }
    m_logger.Logger::log("Reading Controller Mode... Mode = " + CONTROLLER_TYPE_STRINGS.get_string(current_controller));
    return current_controller;
}



ControllerModeStatus SerialPABotBase_Connection::process_device(
    std::optional<ControllerType> change_controller,
    bool clear_settings
){
    //  Protocol Version
    {
        m_logger.Logger::log("Checking Protocol Version...");
        m_protocol = protocol_version(*m_botbase);
        m_logger.Logger::log("Checking Protocol Version... (" + std::to_string(m_protocol) + ")");
    }
    const std::set<pabb_ProgramID>& PROGRAMS = get_programs_for_protocol(m_protocol);

    //  Program ID
    {
        m_logger.Logger::log("Checking Program ID...");
        m_program_id = program_id(*m_botbase);
        m_logger.Logger::log("Checking Program ID... (0x" + tostr_hex(m_program_id) + ")");
    }
    if (PROGRAMS.find(m_program_id) == PROGRAMS.end()){
#if 1
        m_logger.Logger::log(
            "Unrecognized Program ID: 0x" + tostr_hex(m_program_id) + " for this protocol version. "
            "Compatibility is not guaranteed.",
            COLOR_RED
        );
#else
        throw SerialProtocolException(
            m_logger, PA_CURRENT_FUNCTION,
            "Unrecognized Program ID: 0x" + tostr_hex(m_program_id) + "<br>"
            "Please install the firmware that came with this version of the program."
        );
#endif
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
        m_controller_list = controller_list(*m_botbase);
        std::string str;
        bool first = true;
        for (pabb_ControllerID id : m_controller_list){
            if (!first){
                str += ", ";
            }
            first = false;
            str += "0x" + tostr_hex(id);
        }
        m_logger.Logger::log("Checking Controller List... (" + str + ")");
    }

    std::vector<ControllerType> controllers;
    for (pabb_ControllerID id : m_controller_list){
        controllers.emplace_back(id_to_controller_type(id));
    }

    //  Queue Size
    process_queue_size();

    //  Current Controller
    ControllerType current_controller;
    {
        m_logger.log("Reading Controller Mode...");
        uint32_t type_id = read_controller_mode(*m_botbase);
        current_controller = id_to_controller_type(type_id);
        m_logger.Logger::log("Reading Controller Mode... Mode = " + CONTROLLER_TYPE_STRINGS.get_string(current_controller));
    }

//    //  Controller Type
//    ControllerType current_controller = get_controller_type(controllers);

    //  Run any post-connection actions specific to this program.
    ControllerModeStatus ret{current_controller, controllers};
    run_post_connect_actions(
        ret,
        m_program_id, m_device_name,
        *m_botbase,
        change_controller,
        clear_settings
    );
    return ret;
}



void SerialPABotBase_Connection::thread_body(
    std::optional<ControllerType> change_controller,
    bool clear_settings
){
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
        ControllerModeStatus mode_status;
        std::string error;
        try{
            mode_status = process_device(change_controller, clear_settings);
            std::lock_guard<std::mutex> lg(m_lock);
            m_mode_status = mode_status;

            //  Stop pending commands.
            m_botbase->stop_all_commands();
        }catch (InvalidConnectionStateException&){
            return;
        }catch (SerialProtocolException& e){
            error = e.message();
        }catch (ConnectionException& e){
            error = e.message();
        }
        if (error.empty()){
//            std::string text = "Program: " + program_name(m_program_id) + " (" + std::to_string(m_version) + ")";
//            std::string text = program_name(m_program_id) + " (" + std::to_string(m_version) + ")";
            std::string text = m_program_name + " (" + std::to_string(m_version) + ")";
            set_status_line0(text, theme_friendly_darkblue());
            declare_ready(mode_status);
        }else{
            m_ready.store(false, std::memory_order_relaxed);
            set_status_line0(error, COLOR_RED);
//            signal_pre_not_ready();
            m_botbase->stop();
            return;
        }
    }
}



}
}
