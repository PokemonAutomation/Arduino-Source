/*  SerialPABotBase: HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h"
#include "HID_Keyboard_SerialPABotBase.h"

namespace PokemonAutomation{
namespace HidControllers{



SerialPABotBase_Keyboard::SerialPABotBase_Keyboard(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerResetMode reset_mode
)
    : Keyboard(logger)
    , KeyboardControllerWithScheduler(logger)
    , m_handle(connection)
    , m_serial(connection.botbase())
{
    using namespace SerialPABotBase;

    switch (reset_mode){
    case PokemonAutomation::ControllerResetMode::DO_NOT_RESET:
        break;
    case PokemonAutomation::ControllerResetMode::SIMPLE_RESET:
        connection.botbase()->issue_request_and_wait(
            DeviceRequest_change_controller_mode(PABB_CID_StandardHid_Keyboard),
            nullptr
        );
        break;
    case PokemonAutomation::ControllerResetMode::RESET_AND_CLEAR_STATE:
        connection.botbase()->issue_request_and_wait(
            DeviceRequest_reset_to_controller(PABB_CID_StandardHid_Keyboard),
            nullptr
        );
        break;
    }

    //  Re-read the controller.
    ControllerType current_controller = connection.refresh_controller_type();
    if (current_controller != ControllerType::HID_Keyboard){
        throw SerialProtocolException(logger, PA_CURRENT_FUNCTION, "Failed to set controller type.");
    }
}
SerialPABotBase_Keyboard::~SerialPABotBase_Keyboard(){
    stop();
//    m_status_thread.join();
}
void SerialPABotBase_Keyboard::stop(){
    if (m_stopping.exchange(true)){
        return;
    }
    Keyboard::stop();
    m_scope.cancel(nullptr);
    {
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_serial){
            m_serial->notify_all();
        }
        m_cv.notify_all();
    }
}




void SerialPABotBase_Keyboard::cancel_all_commands(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->stop_all_commands();
    m_scheduler.clear_on_next();
}
void SerialPABotBase_Keyboard::replace_on_next_command(){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (!is_ready()){
        throw InvalidConnectionStateException(error_string());
    }
    m_serial->next_command_interrupt();
    m_scheduler.clear_on_next();
}


void SerialPABotBase_Keyboard::wait_for_all(const Cancellable* cancellable){
    SuperscalarScheduler::Schedule schedule;
    std::lock_guard<std::mutex> lg0(m_issue_lock);
    {
        std::lock_guard<std::mutex> lg1(m_state_lock);
        m_logger.log("wait_for_all()", COLOR_DARKGREEN);

        if (!is_ready()){
            throw InvalidConnectionStateException(error_string());
        }

        m_scheduler.issue_wait_for_all(schedule);
    }
    execute_schedule(cancellable, schedule);
    m_serial->wait_for_all_requests(cancellable);
}

void SerialPABotBase_Keyboard::execute_state(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    //  TODO
}





}
}
