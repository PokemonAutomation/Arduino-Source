/*  SerialPABotBase: HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
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

    m_status_thread.reset(new SerialPABotBase::ControllerStatusThread(
        connection, *this
    ));
}
SerialPABotBase_Keyboard::~SerialPABotBase_Keyboard(){
    stop();
}
void SerialPABotBase_Keyboard::stop(){
    Keyboard::stop();
    m_status_thread.reset();
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
    m_logger.log("SerialPABotBase_Keyboard::execute_state() - Not implemented yet.", COLOR_RED);
}



void SerialPABotBase_Keyboard::update_status(Cancellable& cancellable){
    pabb_MsgAckRequestI32 response;
    m_serial->issue_request_and_wait(
        SerialPABotBase::MessageControllerStatus(),
        &cancellable
    ).convert<PABB_MSG_ACK_REQUEST_I32>(m_logger, response);

    uint32_t status = response.data;
    bool status_connected = status & 1;
    bool status_ready     = status & 2;

    std::string str;
    str += "Connected: " + (status_connected
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );
    str += " - Ready: " + (status_ready
        ? html_color_text("Yes", theme_friendly_darkblue())
        : html_color_text("No", COLOR_RED)
    );

    m_handle.set_status_line1(str);
}
void SerialPABotBase_Keyboard::stop_with_error(std::string error_message){
    {
        WriteSpinLock lg(m_error_lock);
        m_error_string = error_message;
    }
    m_serial->stop(std::move(error_message));
}






}
}
