/*  SerialPABotBase: Wireless Pro Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_ESP32.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch_SerialPABotBase_WirelessProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SerialPABotBase_WirelessProController::SerialPABotBase_WirelessProController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection
)
    : SerialPABotBase_WirelessController(
        logger,
        connection,
        ControllerType::NintendoSwitch_WirelessProController
    )
{}
SerialPABotBase_WirelessProController::~SerialPABotBase_WirelessProController(){
    ProController::stop();
    SerialPABotBase_WirelessController::stop();
}


void SerialPABotBase_WirelessProController::push_state(const Cancellable* cancellable, WallDuration duration){
    //  https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md

    ESP32Report0x30 report{
        .report_id = 0x30,
        .timer = 0,     //  Populate on controller.
        .byte2 = 0x99,  //  Full + charging : Pro controller + USB powered
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .leftstick_x_lo = 0x00,
        .leftstick_x_hi = 0x08,
        .leftstick_y = 0x80,
        .rightstick_x_lo = 0x00,
        .rightstick_x_hi = 0x08,
        .rightstick_y = 0x80,
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
//        case BUTTON_RIGHT_SR:   report.button3 |= 1 << 4; break;
//        case BUTTON_RIGHT_SL:   report.button3 |= 1 << 5; break;
        case BUTTON_R:          report.button3 |= 1 << 6; break;
        case BUTTON_ZR:         report.button3 |= 1 << 7; break;

        //  Shared
        case BUTTON_MINUS:      report.button4 |= 1 << 0; break;
        case BUTTON_PLUS:       report.button4 |= 1 << 1; break;
        case BUTTON_RCLICK:     report.button4 |= 1 << 2; break;
        case BUTTON_LCLICK:     report.button4 |= 1 << 3; break;
        case BUTTON_HOME:       report.button4 |= 1 << 4; break;
        case BUTTON_CAPTURE:    report.button4 |= 1 << 5; break;

        //  Left
        case BUTTON_DOWN:       report.button5 |= 1 << 0; break;
        case BUTTON_UP:         report.button5 |= 1 << 1; break;
        case BUTTON_RIGHT:      report.button5 |= 1 << 2; break;
        case BUTTON_LEFT:       report.button5 |= 1 << 3; break;
//        case BUTTON_LEFT_SR:    report.button5 |= 1 << 4; break;
//        case BUTTON_LEFT_SL:    report.button5 |= 1 << 5; break;
        case BUTTON_L:          report.button5 |= 1 << 6; break;
        case BUTTON_ZL:         report.button5 |= 1 << 7; break;

        default:;
        }
    }

    if (m_dpad.is_busy()){
        SplitDpad dpad = convert_unified_to_split_dpad(m_dpad.position);
        report.button5 |= (dpad.down  ? 1 : 0) << 0;
        report.button5 |= (dpad.up    ? 1 : 0) << 1;
        report.button5 |= (dpad.right ? 1 : 0) << 2;
        report.button5 |= (dpad.left  ? 1 : 0) << 3;
    }

    //  Left Stick
    if (m_left_joystick.is_busy()){
        report.leftstick_x_lo = (m_left_joystick.x << 4) & 0xf0;
        report.leftstick_x_hi = (m_left_joystick.x & 0xf0) >> 4;
        report.leftstick_y = 255 - m_left_joystick.y;
    }

    //  Right Stick
    if (m_right_joystick.is_busy()){
        report.rightstick_x_lo = (m_right_joystick.x << 4) & 0xf0;
        report.rightstick_x_hi = (m_right_joystick.x & 0xf0) >> 4;
        report.rightstick_y = 255 - m_right_joystick.y;
    }

    issue_report(cancellable, report, duration);
}





}
}
