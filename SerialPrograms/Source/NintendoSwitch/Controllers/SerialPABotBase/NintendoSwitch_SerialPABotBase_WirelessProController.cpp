/*  SerialPABotBase: Wireless Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/SerialPABotBase/SerialPABotBase_Messages_NS1_WirelessControllers.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch_SerialPABotBase_WirelessProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SerialPABotBase_WirelessProController::SerialPABotBase_WirelessProController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    ControllerResetMode reset_mode
)
    : ProController(logger)
    , SerialPABotBase_WirelessController(
        logger,
        connection,
        ControllerType::NintendoSwitch_WirelessProController,
        reset_mode
    )
{}
SerialPABotBase_WirelessProController::~SerialPABotBase_WirelessProController(){
    ProController::stop();
    SerialPABotBase_WirelessController::stop();
}


void SerialPABotBase_WirelessProController::execute_state(
    const Cancellable* cancellable,
    const SuperscalarScheduler::ScheduleEntry& entry
){
    SwitchControllerState controller_state;
    for (auto& item : entry.state){
        static_cast<const SwitchCommand&>(*item).apply(controller_state);
    }

    //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md
    pabb_NintendoSwitch_WirelessController_State0x30_Buttons buttons{
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .left_joystick = {0x00, 0x08, 0x80},
        .right_joystick = {0x00, 0x08, 0x80},
        .vibrator = 0x00,
    };

//    Button all_buttons =
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

    //  Right Stick
    encode_joystick<JOYSTICK_MIN_THRESHOLD, JOYSTICK_MAX_THRESHOLD>(
        buttons.right_joystick,
        controller_state.right_stick_x, controller_state.right_stick_y
    );

    pabb_NintendoSwitch_WirelessController_State0x30_Gyro gyro{
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
        0x0000,
    };
    bool gyro_active = populate_report_gyro(gyro, controller_state);

//    gyro_active = true;
//    gyro.rotation_y = 0x00ff;
//    gyro.rotation_z = 0x000f;

    if (!gyro_active){
        issue_report(cancellable, entry.duration, buttons);
    }else{
        issue_report(cancellable, entry.duration, buttons, gyro);
    }

#if 0
    m_logger.log(
        "push_state(): (" + button_to_string(all_buttons) +
        "), dpad(" + dpad_to_string(m_dpad.position) +
        "), LJ(" + std::to_string(m_left_joystick.x) + "," + std::to_string(m_left_joystick.y) +
        "), RJ(" + std::to_string(m_right_joystick.x) + "," + std::to_string(m_right_joystick.y) +
        "), hold = " + std::to_string(std::chrono::duration_cast<Milliseconds>(duration).count()) + "ms",
        COLOR_DARKGREEN
    );
#endif
}





}
}
