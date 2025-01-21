/*  Nintendo Switch Controller (Serial PABotBase)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Messages_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_Superscalar.h"
#include "NintendoSwitch_SerialPABotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void SwitchControllerSerialPABotBase::wait_for_all(const Cancellable& cancellable){
    m_serial.wait_for_all_requests(&cancellable);
}
void SwitchControllerSerialPABotBase::cancel_all(const Cancellable& cancellable){
    m_serial.stop_all_commands();
}
void SwitchControllerSerialPABotBase::replace_on_next_command(const Cancellable& cancellable){
    m_serial.next_command_interrupt();
}
void SwitchControllerSerialPABotBase::send_wait_for_pending(const Cancellable& cancellable){
    m_serial.issue_request(
        DeviceRequest_ssf_flush_pipeline(),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_wait(const Cancellable& cancellable, uint16_t ticks){
    m_serial.issue_request(
        DeviceRequest_ssf_do_nothing(ticks),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_controller_state(
    const Cancellable& cancellable,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    uint16_t ticks
){
    // divide the controller state into smaller chunks of 255 ticks
    while (ticks > 0){
        uint16_t curr_ticks = std::min(ticks, (uint16_t)255);
        m_serial.issue_request(
            DeviceRequest_controller_state(button, position, left_x, left_y, right_x, right_y, (uint8_t)curr_ticks)
        );
        ticks -= curr_ticks;
    } // end while loop, and function
}
void SwitchControllerSerialPABotBase::send_botbase_request(
    const Cancellable& cancellable,
    const BotBaseRequest& request
){
    m_serial.issue_request(request, &cancellable);
}
BotBaseMessage SwitchControllerSerialPABotBase::send_botbase_request_and_wait(
    const Cancellable& cancellable,
    const BotBaseRequest& request
){
    return m_serial.issue_request_and_wait(request, &cancellable);
}
void SwitchControllerSerialPABotBase::send_buttons(
    const Cancellable& cancellable,
    Button button,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_button(button, delay, hold, cooldown),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_dpad(
    const Cancellable& cancellable,
    DpadPosition position,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_dpad(position, delay, hold, cooldown),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_left_joystick(
    const Cancellable& cancellable,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_joystick(true, x, y, delay, hold, cooldown),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_right_joystick(
    const Cancellable& cancellable,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_press_joystick(false, x, y, delay, hold, cooldown),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_mash_button(
    const Cancellable& cancellable,
    Button button, uint16_t ticks
){
    m_serial.issue_request(
        DeviceRequest_ssf_mash1_button(button, ticks),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_mash_button(
    const Cancellable& cancellable,
    Button button0, Button button1, uint16_t ticks
){
    m_serial.issue_request(
        DeviceRequest_ssf_mash2_button(button0, button1, ticks),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_mash_AZs(
    const Cancellable& cancellable,
    uint16_t ticks
){
    m_serial.issue_request(
        DeviceRequest_ssf_mash_AZs(ticks),
        &cancellable
    );
}
void SwitchControllerSerialPABotBase::send_system_scroll(
    const Cancellable& cancellable,
    DpadPosition direction, //  Diagonals not allowed.
    uint16_t delay, uint16_t hold, uint8_t cooldown
){
    m_serial.issue_request(
        DeviceRequest_ssf_issue_scroll(direction, delay, hold, cooldown),
        &cancellable
    );
}




}
}
