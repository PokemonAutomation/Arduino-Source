/*  Pokken Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "Controllers/ControllerCapability.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Messages_PushButtons.h"
#include "NintendoSwitch_PokkenController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;




SerialPABotBase_PokkenController::SerialPABotBase_PokkenController(
    Logger& logger,
    SerialPABotBase::SerialPABotBase_Connection& connection,
    const ControllerRequirements& requirements
)
    : SerialPABotBase_ProController(
        logger,
        ControllerType::NintendoSwitch_WiredProController,
        connection,
        requirements
    )
{}
SerialPABotBase_PokkenController::~SerialPABotBase_PokkenController(){
    stop();
}

void SerialPABotBase_PokkenController::push_state(const Cancellable* cancellable, WallDuration duration){
    //  Must be called inside "m_state_lock".

    if (!is_ready()){
        throw InvalidConnectionStateException();
    }

    Button buttons = BUTTON_NONE;
    for (size_t c = 0; c < 14; c++){
        buttons |= m_buttons[c].is_busy()
            ? (Button)((uint16_t)1 << c)
            : BUTTON_NONE;
    }

    DpadPosition dpad = m_dpad.is_busy() ? m_dpad.position : DPAD_NONE;

    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
    if (m_left_joystick.is_busy()){
        left_x = m_left_joystick.x;
        left_y = m_left_joystick.y;
    }
    if (m_right_joystick.is_busy()){
        right_x = m_right_joystick.x;
        right_y = m_right_joystick.y;
    }


    //  Release the state lock since we are no longer touching state.
    //  This loop can block indefinitely if the command queue is full.
    ReverseLockGuard<std::mutex> lg(m_state_lock);

    //  Divide the controller state into smaller chunks of 255 ticks.
    Milliseconds time_left = std::chrono::duration_cast<Milliseconds>(duration);
    while (time_left > Milliseconds::zero()){
        Milliseconds current_ms = std::min(time_left, 255 * 8ms);
        uint8_t current_ticks = (uint8_t)milliseconds_to_ticks_8ms(current_ms.count());
        m_serial->issue_request(
            DeviceRequest_controller_state(buttons, dpad, left_x, left_y, right_x, right_y, current_ticks),
            cancellable
        );
        time_left -= current_ms;
    }
}








}
}
