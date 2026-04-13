/*  SerialPABotBase: Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
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


void SerialPABotBase_Controller::stop_with_error(std::string error_message) noexcept{
    try{
        WriteSpinLock lg(m_error_lock);
        m_error_string = error_message;
    }catch (...){}
    m_serial->stop(std::move(error_message));
}

bool SerialPABotBase_Controller::is_ready() const{
    return m_serial
        && m_serial->state() == BotBaseController::State::RUNNING
        && m_handle.is_ready();
}
std::string SerialPABotBase_Controller::error_string() const{
    ReadSpinLock lg(m_error_lock);
    return m_error_string;
}

void SerialPABotBase_Controller::cancel_all_commands(){
    std::lock_guard<Mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->stop_all_commands();
    m_scheduler.clear_on_next();
    m_logger.log("cancel_all_commands()", COLOR_DARKGREEN);
}
void SerialPABotBase_Controller::replace_on_next_command(){
    std::lock_guard<Mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->next_command_interrupt();
    m_scheduler.clear_on_next();
    m_logger.log("replace_on_next_command()", COLOR_DARKGREEN);
}


void SerialPABotBase_Controller::wait_for_all(Cancellable* cancellable){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<Mutex> lg0(m_issue_lock);
    {
        std::lock_guard<Mutex> lg1(m_state_lock);

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
