/*  Pokken Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_ESP32.h"
#include "ClientSource/Libraries/MessageConverter.h"
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



class SerialPABotBase_WirelessProController::Message : public BotBaseRequest{
public:
    pabb_esp32_report30 params;
    Message(uint8_t ticks, ESP32Report0x30 report)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.ticks = ticks;
        params.report = report;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_ESP32_REPORT, params);
    }
};

int register_message_converters_ESP32(){
    register_message_converter(
        PABB_MSG_ESP32_REPORT,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ESP32_controller_state() - ";
            if (body.size() != sizeof(pabb_esp32_report30)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_esp32_report30*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << (int)params->ticks;
            return ss.str();
        }
    );
    return 0;
}
int init_Messages_ESP32 = register_message_converters_ESP32();



void SerialPABotBase_WirelessProController::push_state(const Cancellable* cancellable, WallDuration duration){
    ESP32Report0x30 report{
        .report_id = 0x30,
        .timer = 0,     //  Populate on controller.
        .byte2 = 0x91,  //  Full + charging : Pro controller + USB powered
        .button3 = 0,
        .button4 = 0,
        .button5 = 0,
        .leftstick_x_lo = 0x00,
        .leftstick_x_hi = 0x08,
        .leftstick_y = 128,
        .rightstick_x_lo = 0x00,
        .rightstick_x_hi = 0x08,
        .rightstick_y = 128,
        .vibrator = 0x80,
        .gyro = {},
    };

    //  Right
    report.button3 |= (m_buttons[0].is_busy() ? 1 : 0) << 0;    //  Y
    report.button3 |= (m_buttons[3].is_busy() ? 1 : 0) << 1;    //  X
    report.button3 |= (m_buttons[1].is_busy() ? 1 : 0) << 2;    //  A
    report.button3 |= (m_buttons[2].is_busy() ? 1 : 0) << 3;    //  B
    //  4 = Right Joycon: SR
    //  5 = Right Joycon: SL
    report.button3 |= (m_buttons[5].is_busy() ? 1 : 0) << 6;    //  R
    report.button3 |= (m_buttons[7].is_busy() ? 1 : 0) << 7;    //  ZR

    //  Shared
    report.button4 |= (m_buttons[8].is_busy() ? 1 : 0) << 0;    //  Minus
    report.button4 |= (m_buttons[9].is_busy() ? 1 : 0) << 1;    //  Plus
    report.button4 |= (m_buttons[11].is_busy() ? 1 : 0) << 2;   //  R Click
    report.button4 |= (m_buttons[10].is_busy() ? 1 : 0) << 3;   //  L Click
    report.button4 |= (m_buttons[12].is_busy() ? 1 : 0) << 4;   //  Home
    report.button4 |= (m_buttons[13].is_busy() ? 1 : 0) << 5;   //  Capture
    //  6 = Unused
    //  7 = Charging Grip

    //  Left
    SplitDpad dpad = convert_unified_to_split_dpad(m_dpad.position);
    report.button5 |= (dpad.down  ? 1 : 0) << 0;
    report.button5 |= (dpad.up    ? 1 : 0) << 1;
    report.button5 |= (dpad.right ? 1 : 0) << 2;
    report.button5 |= (dpad.left  ? 1 : 0) << 3;
    //  4 = Left Joycon: SR
    //  5 = Left Joycon: SL
    report.button5 |= (m_buttons[4].is_busy() ? 1 : 0) << 6;    //  L
    report.button5 |= (m_buttons[6].is_busy() ? 1 : 0) << 7;    //  ZL

    //  Left Joycon
    report.leftstick_x_lo = (m_left_joystick.x << 4) & 0xf0;
    report.leftstick_x_hi = (m_left_joystick.x & 0xf0) >> 4;
    report.leftstick_y = 255 - m_left_joystick.y;

    //  Right Joycon
    report.rightstick_x_lo = (m_right_joystick.x << 4) & 0xf0;
    report.rightstick_x_hi = (m_right_joystick.x & 0xf0) >> 4;
    report.rightstick_y = 255 - m_right_joystick.y;


    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  Divide the controller state into smaller chunks of 255 ticks.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);
    while (time_left > Milliseconds::zero()){
        Milliseconds current_ms = std::min(time_left, 255 * 8ms);
        uint8_t current_ticks = (uint8_t)milliseconds_to_ticks_8ms(current_ms.count());
        m_serial->issue_request(
            Message(current_ticks, report),
            cancellable
        );
        time_left -= current_ms;
    }
}





}
}
