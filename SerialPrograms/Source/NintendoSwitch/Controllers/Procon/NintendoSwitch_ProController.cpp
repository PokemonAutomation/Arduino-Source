/*  Nintendo Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "ControllerInput/ControllerInput.h"
#include "ControllerInput/Keyboard/KeyboardInput_State.h"
#include "Controllers/JoystickTools.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_ProControllerState.h"
#include "NintendoSwitch_ProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<NintendoSwitch::ProController>;
template class SuperControlSession<NintendoSwitch::ProController>;

namespace NintendoSwitch{

using namespace std::chrono_literals;


const char ProController::NAME[] = "Nintendo Switch: Pro Controller";





struct ProController::Data{
    std::map<KeyboardKey, ProControllerDeltas> m_keyboard_mapping;
};




ProController::ProController(Logger& logger)
    : m_data(CONSTRUCT_TOKEN)
{
    std::vector<std::shared_ptr<EditableTableRow>> mapping =
        ConsoleSettings::instance().KEYBOARD_MAPPINGS.PRO_CONTROLLER2.current_refs();

    for (const auto& deltas : mapping){
        const ProControllerFromKeyboardTableRow& row = static_cast<const ProControllerFromKeyboardTableRow&>(*deltas);
        m_data->m_keyboard_mapping[row.key] += row.snapshot();
    }
}
ProController::~ProController(){
}

ControllerClass ProController::controller_class() const{
    return ControllerClass::NintendoSwitch_ProController;
}



void ProController::issue_left_joystick(
    Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    uint8_t x, uint8_t y
){
    issue_left_joystick(
        cancellable,
        delay, hold, cooldown,
        JoystickPosition(
            JoystickTools::linear_u8_to_float(x),
            -JoystickTools::linear_u8_to_float(y)
        )
    );
}
void ProController::issue_right_joystick(
    Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    uint8_t x, uint8_t y
){
    issue_right_joystick(
        cancellable,
        delay, hold, cooldown,
        JoystickPosition(
            JoystickTools::linear_u8_to_float(x),
            -JoystickTools::linear_u8_to_float(y)
        )
    );
}
void ProController::issue_full_controller_state(
    Cancellable* cancellable,
    bool enable_logging,
    Milliseconds duration,
    Button button,
    DpadPosition dpad,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y
){
    issue_full_controller_state(
        cancellable,
        enable_logging,
        duration,
        button,
        dpad,
        JoystickPosition(
            JoystickTools::linear_u8_to_float(left_x),
            -JoystickTools::linear_u8_to_float(left_y)
        ),
        JoystickPosition(
            JoystickTools::linear_u8_to_float(right_x),
            -JoystickTools::linear_u8_to_float(right_y)
        )
    );
}



void ProController::run_controller_input(const ControllerInputState& state){
//    cout << "run_controller_input()" << endl;

    if (state.type() != ControllerInputType::HID_Keyboard){
        return;
    }

    ProControllerDeltas deltas;

    const KeyboardInputState& lstate = static_cast<const KeyboardInputState&>(state);
    const std::map<KeyboardKey, ProControllerDeltas>& map = m_data->m_keyboard_mapping;

//    cout << "keys() = " << lstate.keys().size() << endl;

    for (KeyboardKey key : lstate.keys()){
        auto iter = map.find(key);
        if (iter != map.end()){
            deltas += iter->second;
        }
    }

    replace_on_next_command();

    ProControllerState controller_state;
    deltas.to_state(controller_state);

    WallClock timestamp = current_time();
    controller_state.execute(nullptr, false, *this, 2000ms);

    on_command_input(timestamp, controller_state);
}




}
}
