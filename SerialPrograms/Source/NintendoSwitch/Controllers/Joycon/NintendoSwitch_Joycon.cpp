/*  Nintendo Switch Joycon
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
#include "Controllers/ControllerTypes.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_JoyconState.h"
#include "NintendoSwitch_Joycon.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<NintendoSwitch::JoyconController>;
template class SuperControlSession<NintendoSwitch::JoyconController>;

namespace NintendoSwitch{

using namespace std::chrono_literals;


const char JoyconController::NAME[] = "Nintendo Switch: Joycon";
const char LeftJoycon::NAME[] = "Nintendo Switch: Left Joycon";
const char RightJoycon::NAME[] = "Nintendo Switch: Right Joycon";



struct JoyconController::Data{
    std::map<KeyboardKey, JoyconDeltas> m_keyboard_mapping;
};




JoyconController::JoyconController(Logger& logger, ControllerClass controller_class)
    : m_data(CONSTRUCT_TOKEN)
{
    std::vector<std::shared_ptr<EditableTableRow>> mapping =
        controller_class == ControllerClass::NintendoSwitch_LeftJoycon
            ? ConsoleSettings::instance().KEYBOARD_MAPPINGS.LEFT_JOYCON2.current_refs()
            : ConsoleSettings::instance().KEYBOARD_MAPPINGS.RIGHT_JOYCON2.current_refs();

    for (const auto& deltas : mapping){
        const JoyconFromKeyboardTableRow& row = static_cast<const JoyconFromKeyboardTableRow&>(*deltas);
        m_data->m_keyboard_mapping[row.key] += row.snapshot();
    }
}
JoyconController::~JoyconController(){
}



void JoyconController::issue_joystick(
    Cancellable* cancellable,
    Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
    uint8_t x, uint8_t y
){
    issue_joystick(
        cancellable,
        delay, hold, cooldown,
        JoystickPosition(
            JoystickTools::linear_u8_to_float(x),
            -JoystickTools::linear_u8_to_float(y)
        )
    );
}
void JoyconController::issue_full_controller_state(
    Cancellable* cancellable,
    bool enable_logging,
    Milliseconds duration,
    Button button,
    uint8_t joystick_x, uint8_t joystick_y
){
    issue_full_controller_state(
        cancellable,
        enable_logging,
        duration,
        button,
        JoystickPosition(
            JoystickTools::linear_u8_to_float(joystick_x),
            -JoystickTools::linear_u8_to_float(joystick_y)
        )
    );
}


void JoyconController::run_controller_input(const ControllerInputState& state){

    if (state.type() != ControllerInputType::HID_Keyboard){
        return;
    }

    JoyconDeltas deltas;

    const KeyboardInputState& lstate = static_cast<const KeyboardInputState&>(state);
    const std::map<KeyboardKey, JoyconDeltas>& map = m_data->m_keyboard_mapping;

//    cout << "keys() = " << lstate.keys().size() << endl;

    for (KeyboardKey key : lstate.keys()){
        auto iter = map.find(key);
        if (iter != map.end()){
            deltas += iter->second;
        }
    }

    replace_on_next_command();

    JoyconState controller_state;
    deltas.to_state(controller_state);

    WallClock timestamp = current_time();
    controller_state.execute(nullptr, false, *this, 2000ms);

    on_command_input(timestamp, controller_state);
}






ControllerClass LeftJoycon::controller_class() const{
    return ControllerClass::NintendoSwitch_LeftJoycon;
}
ControllerClass RightJoycon::controller_class() const{
    return ControllerClass::NintendoSwitch_RightJoycon;
}





}
}
