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
    : JoyconController(logger, controller_type)
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
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
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
        ControllerWithScheduler::issue_left_joystick(cancellable, delay, hold, cooldown, x, y);
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_right_joystick(cancellable, delay, hold, cooldown, x, y);
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
            hold,
            button,
            DPAD_NONE,
            joystick_x, joystick_y,
            0x80, 0x80
        );
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            hold,
            button,
            DPAD_NONE,
            0x80, 0x80,
            joystick_x, joystick_y
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
    ControllerWithScheduler::issue_mash_button(cancellable, duration, button);
}




void SerialPABotBase_WirelessJoycon::push_state_left_joycon(const Cancellable* cancellable, WallDuration duration){
    pabb_NintendoSwitch_WirelessController_State0x30_Buttons buttons{
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
    };

    populate_report_buttons(buttons);

    //  Left Stick
    if (m_left_joystick.is_busy()){
        encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
            buttons.left_joystick,
            m_left_joystick.x, m_left_joystick.y
        );
    }

    pabb_NintendoSwitch_WirelessController_State0x30_Gyro gyro{};
    bool gyro_active = populate_report_gyro(gyro);

    if (!gyro_active){
        issue_report(cancellable, duration, buttons);
    }else{
        issue_report(cancellable, duration, buttons, gyro);
    }
}
void SerialPABotBase_WirelessJoycon::push_state_right_joycon(const Cancellable* cancellable, WallDuration duration){
    pabb_NintendoSwitch_WirelessController_State0x30_Buttons buttons{
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
    };

    populate_report_buttons(buttons);

    //  Right Stick
    if (m_right_joystick.is_busy()){
        encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
            buttons.right_joystick,
            m_right_joystick.x, m_right_joystick.y
        );
    }

//    cout << (int)m_right_joystick.x << " - " << (int)m_right_joystick.y << ": " << std::chrono::duration_cast<Milliseconds>(duration).count() << endl;

    pabb_NintendoSwitch_WirelessController_State0x30_Gyro gyro{};
    bool gyro_active = populate_report_gyro(gyro);

    if (!gyro_active){
        issue_report(cancellable, duration, buttons);
    }else{
        issue_report(cancellable, duration, buttons, gyro);
    }
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
