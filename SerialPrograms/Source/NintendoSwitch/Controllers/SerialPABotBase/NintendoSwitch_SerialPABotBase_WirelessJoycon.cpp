/*  SerialPABotBase: Wireless Joycon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch_SerialPABotBase_WirelessJoycon.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SerialPABotBase_WirelessJoycon::SerialPABotBase_WirelessJoycon(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerType controller_type
)
    : JoyconController(controller_type)
    , SerialPABotBase_WirelessController(
        logger,
        connection,
        controller_type
    )
    , m_controller_type(controller_type)
{
    switch (controller_type){
    case ControllerType::NintendoSwitch_LeftJoycon:
        m_valid_buttons = VALID_LEFT_JOYCON_BUTTONS;
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        m_valid_buttons = VALID_RIGHT_JOYCON_BUTTONS;
        break;
    default:
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
    }
}
SerialPABotBase_WirelessJoycon::~SerialPABotBase_WirelessJoycon(){
    JoyconController::stop();
    SerialPABotBase_WirelessController::stop();
}



void SerialPABotBase_WirelessJoycon::issue_buttons(
    const Cancellable* cancellable,
    Button button,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    button &= m_valid_buttons;
    switch (m_controller_type){
    case ControllerType::NintendoSwitch_LeftJoycon:
        ControllerWithScheduler::issue_buttons(cancellable, button, delay, hold, cooldown);
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_buttons(cancellable, button, delay, hold, cooldown);
        break;
    default:
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
    }
}
void SerialPABotBase_WirelessJoycon::issue_joystick(
    const Cancellable* cancellable,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    switch (m_controller_type){
    case ControllerType::NintendoSwitch_LeftJoycon:
        ControllerWithScheduler::issue_left_joystick(cancellable, x, y, delay, hold, cooldown);
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_right_joystick(cancellable, x, y, delay, hold, cooldown);
        break;
    default:
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
    }
}
void SerialPABotBase_WirelessJoycon::issue_full_controller_state(
    const Cancellable* cancellable,
    Button button,
    uint8_t joystick_x, uint8_t joystick_y,
    Milliseconds hold
){
    button &= m_valid_buttons;
    switch (m_controller_type){
    case ControllerType::NintendoSwitch_LeftJoycon:
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            button,
            DPAD_NONE,
            joystick_x, joystick_y,
            0x80, 0x80,
            hold
        );
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            button,
            DPAD_NONE,
            0x80, 0x80,
            joystick_x, joystick_y,
            hold
        );
        break;
    default:;
    }
}



void SerialPABotBase_WirelessJoycon::issue_mash_button(
    const Cancellable* cancellable,
    Button button, Milliseconds duration
){
    button &= m_valid_buttons;
    ControllerWithScheduler::issue_mash_button(cancellable, button, duration);
}




void SerialPABotBase_WirelessJoycon::push_state_left_joycon(const Cancellable* cancellable, WallDuration duration){
    //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md

    SerialPABotBase::NintendoSwitch_ESP32Report0x30 report{
        .report_id = 0x30,
        .timer = 0,     //  Populate on controller.
        .byte2 = 0x9d,  //  Full + Charging : Joycon
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
        .gyro = {},
    };

    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        if (!m_buttons[c].is_busy()){
            continue;
        }

//        cout << "button: " << c << endl;

        Button button = (Button)((ButtonFlagType)1 << c);
        switch (button){
        //  Right
//        case BUTTON_Y:          report.button3 |= 1 << 0; break;
//        case BUTTON_X:          report.button3 |= 1 << 1; break;
//        case BUTTON_B:          report.button3 |= 1 << 2; break;
//        case BUTTON_A:          report.button3 |= 1 << 3; break;
//        case BUTTON_RIGHT_SR:   report.button3 |= 1 << 4; break;
//        case BUTTON_RIGHT_SL:   report.button3 |= 1 << 5; break;
//        case BUTTON_R:          report.button3 |= 1 << 6; break;
//        case BUTTON_ZR:         report.button3 |= 1 << 7; break;

        //  Shared
        case BUTTON_MINUS:      report.button4 |= 1 << 0; break;
//        case BUTTON_PLUS:       report.button4 |= 1 << 1; break;
//        case BUTTON_RCLICK:     report.button4 |= 1 << 2; break;
        case BUTTON_LCLICK:     report.button4 |= 1 << 3; break;
//        case BUTTON_HOME:       report.button4 |= 1 << 4; break;
        case BUTTON_CAPTURE:    report.button4 |= 1 << 5; break;

        //  Left
        case BUTTON_DOWN:       report.button5 |= 1 << 0; break;
        case BUTTON_UP:         report.button5 |= 1 << 1; break;
        case BUTTON_RIGHT:      report.button5 |= 1 << 2; break;
        case BUTTON_LEFT:       report.button5 |= 1 << 3; break;
        case BUTTON_LEFT_SR:    report.button5 |= 1 << 4; break;
        case BUTTON_LEFT_SL:    report.button5 |= 1 << 5; break;
        case BUTTON_L:          report.button5 |= 1 << 6; break;
        case BUTTON_ZL:         report.button5 |= 1 << 7; break;

        default:;
        }
    }

    //  Left Stick
    if (m_left_joystick.is_busy()){
        encode_joystick(report.left_joystick, m_left_joystick.x, m_left_joystick.y);
    }

    issue_report(cancellable, report, duration);
}
void SerialPABotBase_WirelessJoycon::push_state_right_joycon(const Cancellable* cancellable, WallDuration duration){
    //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md

    SerialPABotBase::NintendoSwitch_ESP32Report0x30 report{
        .report_id = 0x30,
        .timer = 0,     //  Populate on controller.
        .byte2 = 0x9d,  //  Full + Charging : Joycon
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
        .gyro = {},
    };

    for (size_t c = 0; c < TOTAL_BUTTONS; c++){
        if (!m_buttons[c].is_busy()){
            continue;
        }
        Button button = (Button)((ButtonFlagType)1 << c);
        switch (button){
        //  Right
        case BUTTON_Y:          report.button3 |= 1 << 0; break;
        case BUTTON_X:          report.button3 |= 1 << 1; break;
        case BUTTON_B:          report.button3 |= 1 << 2; break;
        case BUTTON_A:          report.button3 |= 1 << 3; break;
        case BUTTON_RIGHT_SR:   report.button3 |= 1 << 4; break;
        case BUTTON_RIGHT_SL:   report.button3 |= 1 << 5; break;
        case BUTTON_R:          report.button3 |= 1 << 6; break;
        case BUTTON_ZR:         report.button3 |= 1 << 7; break;

        //  Shared
//        case BUTTON_MINUS:      report.button4 |= 1 << 0; break;
        case BUTTON_PLUS:       report.button4 |= 1 << 1; break;
        case BUTTON_RCLICK:     report.button4 |= 1 << 2; break;
//        case BUTTON_LCLICK:     report.button4 |= 1 << 3; break;
        case BUTTON_HOME:       report.button4 |= 1 << 4; break;
//        case BUTTON_CAPTURE:    report.button4 |= 1 << 5; break;

        //  Left
//        case BUTTON_DOWN:       report.button5 |= 1 << 0; break;
//        case BUTTON_UP:         report.button5 |= 1 << 1; break;
//        case BUTTON_RIGHT:      report.button5 |= 1 << 2; break;
//        case BUTTON_LEFT:       report.button5 |= 1 << 3; break;
//        case BUTTON_LEFT_SR:    report.button5 |= 1 << 4; break;
//        case BUTTON_LEFT_SL:    report.button5 |= 1 << 5; break;
//        case BUTTON_L:          report.button5 |= 1 << 6; break;
//        case BUTTON_ZL:         report.button5 |= 1 << 7; break;

        default:;
        }
    }

    //  Right Stick
    if (m_right_joystick.is_busy()){
        encode_joystick(report.right_joystick, m_right_joystick.x, m_right_joystick.y);
    }

    issue_report(cancellable, report, duration);

}
void SerialPABotBase_WirelessJoycon::push_state(const Cancellable* cancellable, WallDuration duration){
    switch (m_controller_type){
    case ControllerType::NintendoSwitch_LeftJoycon:
        push_state_left_joycon(cancellable, duration);
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        push_state_right_joycon(cancellable, duration);
        break;
    default:
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
    }
}





}
}
