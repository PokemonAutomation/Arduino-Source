/*  SerialPABotBase: Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "Controllers/ControllerTypeStrings.h"
#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch_SerialPABotBase_Controller.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;




SerialPABotBase_Controller::SerialPABotBase_Controller(
    Logger& logger,
    ControllerType controller_type,
    SerialPABotBase::SerialPABotBase_Connection& connection
)
    : ControllerWithScheduler(logger)
    , m_handle(connection)
    , m_serial(connection.botbase())
{
    if (!connection.is_ready()){
        return;
    }

    //  Check compatibility.

    ControllerModeStatus mode_status = connection.controller_mode_status();
    logger.log(
        "SerialPABotBase_Controller(): ControllerType = " +
        CONTROLLER_TYPE_STRINGS.get_string(mode_status.current_controller)
    );

    std::map<ControllerType, ControllerFeatures>& controllers = mode_status.supported_controllers;
    auto iter = controllers.find(controller_type);
    if (iter != controllers.end()){
        m_supported_features = std::move(iter->second);
    }
}




void SerialPABotBase_Controller::cancel_all_commands(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->stop_all_commands();
    this->clear_on_next();
}
void SerialPABotBase_Controller::replace_on_next_command(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->next_command_interrupt();
    this->clear_on_next();
}


void SerialPABotBase_Controller::wait_for_all(const Cancellable* cancellable){
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);

        ThrottleScope scope(m_logging_throttler);
        if (scope){
            m_logger.log("wait_for_all()", COLOR_DARKGREEN);
        }

        if (!is_ready()){
            throw InvalidConnectionStateException(error_string());
        }

        this->issue_wait_for_all(cancellable);
    }
    m_serial->wait_for_all_requests(cancellable);
}













}
}
