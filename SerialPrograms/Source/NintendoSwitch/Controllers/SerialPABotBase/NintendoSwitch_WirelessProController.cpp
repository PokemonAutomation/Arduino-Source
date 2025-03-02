/*  Pokken Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch_WirelessProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;



SerialPABotBase_WirelessProController::SerialPABotBase_WirelessProController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    const ControllerRequirements& requirements
)
    : SerialPABotBase_ProController(
        logger,
        ControllerType::NintendoSwitch_WirelessProController,
        connection,
        requirements
    )
{}
SerialPABotBase_WirelessProController::~SerialPABotBase_WirelessProController(){
    stop();
}




struct SerialPABotBase_WirelessProController::Report_0x30{
    //  Not part of the report.
    uint8_t duration_ticks;

    uint8_t report_id = 0x30;
    uint8_t timer = 0;          //  Populate on controller.
    uint8_t byte2 = 0x91;       //  Full + charging : Pro controller + USB powered
    uint8_t button3 = 0;
    uint8_t button4 = 0;
    uint8_t button5 = 0;
    uint8_t leftstick_x_lo = 0x00;
    uint8_t leftstick_x_hi = 0x08;
    uint8_t leftstick_y = 128;
    uint8_t rightstick_x_lo = 0x00;
    uint8_t rightstick_x_hi = 0x08;
    uint8_t rightstick_y = 128;
    uint8_t vibrator = 0x80;
    uint8_t gyro[49 - 13] = {};
};


class SerialPABotBase_WirelessProController::Message : public BotBaseRequest{
public:
    Report_0x30 report;
    Message(Report_0x30 p_report, uint8_t duration_ticks)
        : BotBaseRequest(true)
        , report(p_report)
    {
        report.duration_ticks = duration_ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(0x9e, report);
    }
};



void SerialPABotBase_WirelessProController::push_state(const Cancellable* cancellable, WallDuration duration){
    Report_0x30 report;

    //  Right
    report.button3 |= (m_buttons[BUTTON_Y].is_busy() ? 1 : 0) << 0;
    report.button3 |= (m_buttons[BUTTON_X].is_busy() ? 1 : 0) << 1;
    report.button3 |= (m_buttons[BUTTON_B].is_busy() ? 1 : 0) << 2;
    report.button3 |= (m_buttons[BUTTON_A].is_busy() ? 1 : 0) << 3;
    //  4 = Right Joycon: SR
    //  5 = Right Joycon: SL
    report.button3 |= (m_buttons[BUTTON_R].is_busy() ? 1 : 0) << 6;
    report.button3 |= (m_buttons[BUTTON_ZR].is_busy() ? 1 : 0) << 7;

    //  Shared
    report.button4 |= (m_buttons[BUTTON_MINUS].is_busy() ? 1 : 0) << 0;
    report.button4 |= (m_buttons[BUTTON_PLUS].is_busy() ? 1 : 0) << 1;
    report.button4 |= (m_buttons[BUTTON_RCLICK].is_busy() ? 1 : 0) << 2;
    report.button4 |= (m_buttons[BUTTON_LCLICK].is_busy() ? 1 : 0) << 3;
    report.button4 |= (m_buttons[BUTTON_HOME].is_busy() ? 1 : 0) << 4;
    report.button4 |= (m_buttons[BUTTON_CAPTURE].is_busy() ? 1 : 0) << 5;
    //  6 = Unused
    //  7 = Charging Grip

    //  Left
    SplitDpad dpad = convert_unified_to_split_dpad(m_dpad.position);
    report.button5 |= (m_buttons[dpad.down].is_busy() ? 1 : 0) << 0;
    report.button5 |= (m_buttons[dpad.up].is_busy() ? 1 : 0) << 1;
    report.button5 |= (m_buttons[dpad.right].is_busy() ? 1 : 0) << 2;
    report.button5 |= (m_buttons[dpad.left].is_busy() ? 1 : 0) << 3;
    //  4 = Left Joycon: SR
    //  5 = Left Joycon: SL
    report.button5 |= (m_buttons[BUTTON_L].is_busy() ? 1 : 0) << 6;
    report.button5 |= (m_buttons[BUTTON_ZL].is_busy() ? 1 : 0) << 7;

    //  Left Joycon
    report.leftstick_x_lo = (m_left_joystick.x << 4) & 0xf0;
    report.leftstick_x_hi = (m_left_joystick.x & 0xf0) >> 4;
    report.leftstick_y = m_left_joystick.y;

    //  Right Joycon
    report.rightstick_x_lo = (m_right_joystick.x << 4) & 0xf0;
    report.rightstick_x_hi = (m_right_joystick.x & 0xf0) >> 4;
    report.rightstick_y = m_right_joystick.y;


    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  Divide the controller state into smaller chunks of 255 ticks.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);
    while (time_left > Milliseconds::zero()){
        Milliseconds current_ms = std::min(time_left, 255 * 8ms);
        uint8_t current_ticks = (uint8_t)milliseconds_to_ticks_8ms(current_ms.count());
        m_serial->issue_request(
            Message(report, current_ticks),
            cancellable
        );
        time_left -= current_ms;
    }
}






}
}
