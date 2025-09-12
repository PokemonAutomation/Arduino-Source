/*  SerialPABotBase: Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "Controllers/ControllerTypeStrings.h"
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

    logger.log(
        "SerialPABotBase_Controller(): ControllerType = " +
        CONTROLLER_TYPE_STRINGS.get_string(connection.current_controller())
    );
}




void SerialPABotBase_Controller::cancel_all_commands(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->stop_all_commands();
    m_scheduler.clear_on_next();
}
void SerialPABotBase_Controller::replace_on_next_command(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->next_command_interrupt();
    m_scheduler.clear_on_next();
}


void SerialPABotBase_Controller::wait_for_all(const Cancellable* cancellable){
    SuperscalarScheduler::Schedule schedule;
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

        m_scheduler.issue_wait_for_all(schedule);
    }
    execute_schedule(cancellable, schedule);
    m_serial->wait_for_all_requests(cancellable);
}













}
}
