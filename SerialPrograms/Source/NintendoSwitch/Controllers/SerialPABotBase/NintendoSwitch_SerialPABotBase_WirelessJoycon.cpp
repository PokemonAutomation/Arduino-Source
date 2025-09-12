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



SerialPABotBase_WirelessLeftJoycon::SerialPABotBase_WirelessLeftJoycon(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerResetMode reset_mode
)
    : SerialPABotBase_WirelessJoycon<LeftJoycon>(
        logger, connection,
        ControllerType::NintendoSwitch_LeftJoycon,
        reset_mode
    )
{
    m_valid_buttons = VALID_LEFT_JOYCON_BUTTONS;
}
SerialPABotBase_WirelessLeftJoycon::~SerialPABotBase_WirelessLeftJoycon(){
    JoyconController::stop();
    SerialPABotBase_WirelessController::stop();
}

SerialPABotBase_WirelessRightJoycon::SerialPABotBase_WirelessRightJoycon(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerResetMode reset_mode
)
    : SerialPABotBase_WirelessJoycon<RightJoycon>(
        logger, connection,
        ControllerType::NintendoSwitch_RightJoycon,
        reset_mode
    )
{
    m_valid_buttons = VALID_RIGHT_JOYCON_BUTTONS;
}
SerialPABotBase_WirelessRightJoycon::~SerialPABotBase_WirelessRightJoycon(){
    JoyconController::stop();
    SerialPABotBase_WirelessController::stop();
}







template <typename JoyconType>
SerialPABotBase_WirelessJoycon<JoyconType>::SerialPABotBase_WirelessJoycon(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerType controller_type,
    ControllerResetMode reset_mode
)
    : JoyconType(logger, controller_type)
    , SerialPABotBase_WirelessController(
        logger,
        connection,
        controller_type,
        reset_mode
    )
    , m_controller_type(controller_type)
{}



template <typename JoyconType>
void SerialPABotBase_WirelessJoycon<JoyconType>::issue_buttons(
    const Cancellable* cancellable,
    Button button,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown
){
    button &= m_valid_buttons;
    ControllerWithScheduler::issue_buttons(cancellable, delay, hold, cooldown, button);
}
template <typename JoyconType>
void SerialPABotBase_WirelessJoycon<JoyconType>::issue_joystick(
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
template <typename JoyconType>
void SerialPABotBase_WirelessJoycon<JoyconType>::issue_full_controller_state(
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



template <typename JoyconType>
void SerialPABotBase_WirelessJoycon<JoyconType>::issue_mash_button(
    const Cancellable* cancellable,
    Button button, Milliseconds duration
){
    button &= m_valid_buttons;
    ControllerWithScheduler::issue_mash_button(cancellable, duration, button);
}




template <typename JoyconType>
void SerialPABotBase_WirelessJoycon<JoyconType>::execute_state_left_joycon(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }

    pabb_NintendoSwitch_WirelessController_State0x30_Buttons buttons{
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
        controller_state.left_stick_x, controller_state.left_stick_y
    );

    pabb_NintendoSwitch_WirelessController_State0x30_Gyro gyro{};
    bool gyro_active = populate_report_gyro(gyro, controller_state);

    if (!gyro_active){
        issue_report(cancellable, entry.duration, buttons);
    }else{
        issue_report(cancellable, entry.duration, buttons, gyro);
    }
}
template <typename JoyconType>
void SerialPABotBase_WirelessJoycon<JoyconType>::execute_state_right_joycon(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }

    pabb_NintendoSwitch_WirelessController_State0x30_Buttons buttons{
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
        controller_state.right_stick_x, controller_state.right_stick_y
    );

//    cout << (int)m_right_joystick.x << " - " << (int)m_right_joystick.y << ": " << std::chrono::duration_cast<Milliseconds>(duration).count() << endl;

    pabb_NintendoSwitch_WirelessController_State0x30_Gyro gyro{};
    bool gyro_active = populate_report_gyro(gyro, controller_state);

    if (!gyro_active){
        issue_report(cancellable, entry.duration, buttons);
    }else{
        issue_report(cancellable, entry.duration, buttons, gyro);
    }
}
template <typename JoyconType>
void SerialPABotBase_WirelessJoycon<JoyconType>::execute_state(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    switch (m_controller_type){
    case ControllerType::NintendoSwitch_LeftJoycon:
        execute_state_left_joycon(cancellable, entry);
        break;
    case ControllerType::NintendoSwitch_RightJoycon:
        execute_state_right_joycon(cancellable, entry);
        break;
    default:
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Invalid joycon type.");
    }
}








}
}
