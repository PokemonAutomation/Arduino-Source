/*  PABotBase2: Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch_PABotBase2_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void PABotBase2_Controller::stop_with_error(std::string error_message) noexcept{
    try{
        WriteSpinLock lg(m_error_lock);
        m_error_string = error_message;
    }catch (...){}
    m_connection.cancel();
    m_connection.set_status_line1(std::move(error_message), COLOR_RED);
}


std::string PABotBase2_Controller::error_string() const{
    ReadSpinLock lg(m_error_lock);
    return m_error_string;
}


void PABotBase2_Controller::cancel_all_commands(){
    std::lock_guard<Mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_logger.log("cancel_all_commands()", COLOR_DARKGREEN);
    m_connection.device().command_queue().send_cancel();
    m_scheduler.clear_on_next();
}
void PABotBase2_Controller::replace_on_next_command(){
    std::lock_guard<Mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_logger.log("replace_on_next_command()", COLOR_DARKGREEN);
    m_connection.device().command_queue().send_replace_on_next();
    m_scheduler.clear_on_next();
}


void PABotBase2_Controller::wait_for_all(Cancellable* cancellable){
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
    m_connection.device().command_queue().wait_for_all(cancellable);
}




}
}
