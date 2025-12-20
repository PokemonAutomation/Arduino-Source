/*  SerialPABotBase: Joycon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch_SerialPABotBase_Joycon.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SerialPABotBase_LeftJoycon::SerialPABotBase_LeftJoycon(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
)
    : SerialPABotBase_Joycon<LeftJoycon>(
        logger, connection,
        ControllerClass::NintendoSwitch_LeftJoycon,
        controller_type,
        reset_mode
    )
{
    m_valid_buttons = VALID_LEFT_JOYCON_BUTTONS;
}
SerialPABotBase_LeftJoycon::~SerialPABotBase_LeftJoycon(){
    SerialPABotBase_OemController::stop();
}

SerialPABotBase_RightJoycon::SerialPABotBase_RightJoycon(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
)
    : SerialPABotBase_Joycon<RightJoycon>(
        logger, connection,
        ControllerClass::NintendoSwitch_RightJoycon,
        controller_type,
        reset_mode
    )
{
    m_valid_buttons = VALID_RIGHT_JOYCON_BUTTONS;
}
SerialPABotBase_RightJoycon::~SerialPABotBase_RightJoycon(){
    SerialPABotBase_OemController::stop();
}







template <typename JoyconType>
SerialPABotBase_Joycon<JoyconType>::SerialPABotBase_Joycon(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerClass controller_class,
    ControllerType controller_type,
    ControllerResetMode reset_mode
)
    : JoyconType(logger, controller_class)
    , SerialPABotBase_OemController(
        logger,
        connection,
        controller_type,
        reset_mode
    )
    , m_controller_type(controller_type)
{}



template <typename JoyconType>
void SerialPABotBase_Joycon<JoyconType>::issue_buttons(
    Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    Button button
){
    button &= m_valid_buttons;
    ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
}
template <typename JoyconType>
void SerialPABotBase_Joycon<JoyconType>::issue_joystick(
    Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    const JoystickPosition& position
){
    switch (this->controller_class()){
    case ControllerClass::NintendoSwitch_LeftJoycon:
        ControllerWithScheduler::issue_left_joystick(cancellable, delay, hold, cooldown, position);
        break;
    case ControllerClass::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_right_joystick(cancellable, delay, hold, cooldown, position);
        break;
    default:
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
    }
}
template <typename JoyconType>
void SerialPABotBase_Joycon<JoyconType>::issue_full_controller_state(
    Cancellable* cancellable,
    bool enable_logging,
    Milliseconds duration,
    Button button,
    const JoystickPosition& joystick
){
    button &= m_valid_buttons;
    switch (this->controller_class()){
    case ControllerClass::NintendoSwitch_LeftJoycon:
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            enable_logging,
            duration,
            button,
            DPAD_NONE,
            joystick,
            {0, 0}
        );
        break;
    case ControllerClass::NintendoSwitch_RightJoycon:
        ControllerWithScheduler::issue_full_controller_state(
            cancellable,
            enable_logging,
            duration,
            button,
            DPAD_NONE,
            {0, 0},
            joystick
        );
        break;
    default:;
    }
}



template <typename JoyconType>
void SerialPABotBase_Joycon<JoyconType>::issue_mash_button(
    Cancellable* cancellable,
    Button button, Milliseconds duration
){
    button &= m_valid_buttons;
    ControllerWithScheduler::issue_mash_button(cancellable, duration, button);
}




template <typename JoyconType>
void SerialPABotBase_Joycon<JoyconType>::execute_state_left_joycon(
    Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }

    pabb_NintendoSwitch_OemController_State0x30_Buttons buttons{
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
    };

    populate_report_buttons(buttons, controller_state);

    {
        SplitDpad dpad = convert_unified_to_split_dpad(controller_state.dpad);
        buttons.button5 |= (dpad.down  ? 1 : 0) << 0;
        buttons.button5 |= (dpad.up    ? 1 : 0) << 1;
        buttons.button5 |= (dpad.right ? 1 : 0) << 2;
        buttons.button5 |= (dpad.left  ? 1 : 0) << 3;
    }

    //  Left Stick
    encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
        buttons.left_joystick,
        controller_state.left_joystick
    );

    pabb_NintendoSwitch_OemController_State0x30_Gyro gyro{};
    bool gyro_active = populate_report_gyro(gyro, controller_state);

    if (!gyro_active){
        issue_report(cancellable, entry.duration, buttons);
    }else{
        issue_report(cancellable, entry.duration, buttons, gyro);
    }
}
template <typename JoyconType>
void SerialPABotBase_Joycon<JoyconType>::execute_state_right_joycon(
    Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }

    pabb_NintendoSwitch_OemController_State0x30_Buttons buttons{
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
    };

    populate_report_buttons(buttons, controller_state);

    //  Right Stick
    encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
        buttons.right_joystick,
        controller_state.right_joystick
    );

#if 0
    cout << (int)controller_state.right_stick_x << " - "
         << (int)controller_state.right_stick_y << ": "
         << std::chrono::duration_cast<Milliseconds>(entry.duration).count() << endl;
#endif

    pabb_NintendoSwitch_OemController_State0x30_Gyro gyro{};
    bool gyro_active = populate_report_gyro(gyro, controller_state);

    if (!gyro_active){
        issue_report(cancellable, entry.duration, buttons);
    }else{
        issue_report(cancellable, entry.duration, buttons, gyro);
    }
}
template <typename JoyconType>
void SerialPABotBase_Joycon<JoyconType>::execute_state(
    Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    switch (this->controller_class()){
    case ControllerClass::NintendoSwitch_LeftJoycon:
        execute_state_left_joycon(cancellable, entry);
        break;
    case ControllerClass::NintendoSwitch_RightJoycon:
        execute_state_right_joycon(cancellable, entry);
        break;
    default:
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
    }
}








}
}
