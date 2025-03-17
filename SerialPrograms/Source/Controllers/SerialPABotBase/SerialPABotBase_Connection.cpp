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
#include "ClientSource/Libraries/MessageConverter.h"
#include "ClientSource/Connection/SerialConnection.h"
#include "ClientSource/Connection/PABotBase.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/ControllerTypeStrings.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_ESP32.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "SerialPABotBase.h"
#include "SerialPABotBase_Connection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{





SerialPABotBase_Connection::SerialPABotBase_Connection(
    Logger& logger,
    const QSerialPortInfo* port,
    std::optional<ControllerType> change_controller
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
            "Prolific controllers do not work for Arduino and similar microntrollers.<br>"
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
        [=, this]{ thread_body(change_controller); }
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


void SerialPABotBase_Connection::update_with_capabilities(const ControllerFeatures& capabilities){
    Logger& logger = m_logger;

    do{
        if (capabilities.contains(ControllerFeature::QueryCommandQueueSize)){
            break;
        }

        //  Program Version
        logger.log("Checking Firmware Version...");
        uint32_t version = program_version(*m_botbase);
        logger.log("Checking Firmware Version... Version = " + std::to_string(version));

        //  REMOVE: Temporary for migration.
        if (m_protocol / 100 == 20210526 && version == 2023121900){
            break;
        }

        logger.log("Queue size not supported. Defaulting to size 4.", COLOR_RED);
        return;
    }while (false);

    logger.log("Device supports queue size. Requesting queue size...", COLOR_BLUE);
    uint8_t queue_limit = device_queue_size(*m_botbase);
    logger.log("Setting queue size to: " + std::to_string(queue_limit), COLOR_BLUE);
    m_botbase->set_queue_limit(queue_limit);
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


ControllerModeStatus SerialPABotBase_Connection::read_device_specs(
    std::optional<ControllerType> change_controller
){
    Logger& logger = m_logger;


    //  Protocol
    logger.log("Checking device protocol compatibility...");
    m_protocol = protocol_version(*m_botbase);
    logger.log("Checking device protocol compatibility... Protocol = " + std::to_string(m_protocol));

    //  (protocol_requested / 100) == (protocol_device / 100)
    //  (protocol_requested % 100) <= (protocol_device / 100)
    auto protocol_iter = SUPPORTED_VERSIONS.upper_bound(m_protocol);
    if (protocol_iter == SUPPORTED_VERSIONS.begin()){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Incompatible protocol. Device: " + std::to_string(m_protocol) + "<br>"
            "Please install the firmware that came with this version of the program."
        );
    }
    --protocol_iter;
    if (protocol_iter->first < m_protocol / 100 * 100){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Incompatible protocol. Device: " + std::to_string(m_protocol) + "<br>"
            "Please install the firmware that came with this version of the program."
        );
    }


    //  Program ID
    logger.log("Checking Program ID...");
    m_program_id = program_id(*m_botbase);
    logger.log("Checking Program ID... Program ID = " + std::to_string(m_program_id));

    const std::map<uint32_t, std::map<ControllerType, ControllerFeatures>>& PROGRAM_IDS = protocol_iter->second;
    auto program_iter = PROGRAM_IDS.find(m_program_id);
    if (program_iter == PROGRAM_IDS.end()){
        throw SerialProtocolException(
            logger, PA_CURRENT_FUNCTION,
            "Unrecognized Program ID: " + std::to_string(m_program_id) + "<br>"
            "Please install the firmware that came with this version of the program."
        );
    }


    //  Controller Type
    logger.log("Reading Controller Mode...");
    ControllerType current_controller = ControllerType::None;
    if (program_iter->second.size() == 1){
        current_controller = program_iter->second.begin()->first;
    }else if (program_iter->second.size() > 1){
        uint32_t type_id = read_controller_mode(*m_botbase);
        current_controller = id_to_controller_type(type_id);
    }
    logger.log("Reading Controller Mode... Mode = " + CONTROLLER_TYPE_STRINGS.get_string(current_controller));


    if (change_controller && program_iter->second.size() > 1){
        ControllerType desired_controller = change_controller.value();
        switch (desired_controller){
        case ControllerType::NintendoSwitch_WiredProController:
        case ControllerType::NintendoSwitch_WirelessProController:
        case ControllerType::NintendoSwitch_LeftJoycon:
        case ControllerType::NintendoSwitch_RightJoycon:{
            NintendoSwitch::ControllerProfile profile =
                PokemonAutomation::NintendoSwitch::ConsoleSettings::instance().CONTROLLER_SETTINGS.get_or_make_profile(
                    m_device_name,
                    desired_controller
                );

            NintendoSwitch_ControllerColors colors;
            {
                Color color(profile.body_color);
                colors.body[0] = color.red();
                colors.body[1] = color.green();
                colors.body[2] = color.blue();
            }
            {
                Color color(profile.button_color);
                colors.buttons[0] = color.red();
                colors.buttons[1] = color.green();
                colors.buttons[2] = color.blue();
            }
            {
                Color color(profile.left_grip);
                colors.left_grip[0] = color.red();
                colors.left_grip[1] = color.green();
                colors.left_grip[2] = color.blue();
            }
            {
                Color color(profile.right_grip);
                colors.right_grip[0] = color.red();
                colors.right_grip[1] = color.green();
                colors.right_grip[2] = color.blue();
            }

            m_botbase->issue_request_and_wait(
                MessageControllerWriteSpi(
                    desired_controller,
                    0x00006050, sizeof(NintendoSwitch_ControllerColors),
                    &colors
                ),
//                MessageControllerSetColors(desired_controller, colors),
                nullptr
            );
        }
        default:;
        }


        uint32_t native_controller_id = controller_type_to_id(desired_controller);
        m_botbase->issue_request_and_wait(
            DeviceRequest_change_controller_mode(native_controller_id),
            nullptr
        );

        //  Re-read the controller.
        logger.log("Reading Controller Mode...");
        uint32_t type_id = read_controller_mode(*m_botbase);
        current_controller = id_to_controller_type(type_id);
        logger.log("Reading Controller Mode... Mode = " + CONTROLLER_TYPE_STRINGS.get_string(current_controller));
    }

    return {current_controller, program_iter->second};
}



void SerialPABotBase_Connection::thread_body(
    std::optional<ControllerType> change_controller
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
            mode_status = read_device_specs(change_controller);
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
//            std::string text = "Program: " + program_name(m_program_id) + " (" + std::to_string(m_protocol) + ")";
            std::string text = program_name(m_program_id) + " (" + std::to_string(m_protocol) + ")";
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
