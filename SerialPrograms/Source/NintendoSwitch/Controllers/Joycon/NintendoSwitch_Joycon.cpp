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
#include "Controllers/ControllerTypes.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"
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
    KeyboardEventHandler m_input_sniffer;
};




JoyconController::JoyconController(Logger& logger, ControllerClass controller_class)
    : m_data(CONSTRUCT_TOKEN)
{
    std::vector<std::shared_ptr<EditableTableRow>> mapping =
        controller_class == ControllerClass::NintendoSwitch_LeftJoycon
            ? ConsoleSettings::instance().KEYBOARD_MAPPINGS.LEFT_JOYCON.current_refs()
            : ConsoleSettings::instance().KEYBOARD_MAPPINGS.RIGHT_JOYCON.current_refs();

    for (const auto& deltas : mapping){
        const JoyconFromKeyboardTableRow& row = static_cast<const JoyconFromKeyboardTableRow&>(*deltas);
        m_data->m_keyboard_mapping[row.key] += row.snapshot();
    }
}
JoyconController::~JoyconController(){
}

void JoyconController::controller_input_state(const ControllerInputState& state){

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

    WallClock timestamp = current_time();

    JoyconState controller_state;
    deltas.to_state(controller_state);
    issue_full_controller_state(
        nullptr,
        false,
        2000ms,
        controller_state.buttons,
        controller_state.joystick_x,
        controller_state.joystick_y
    );

    m_data->m_input_sniffer.report_keyboard_command_sent(timestamp, controller_state);
}

void JoyconController::add_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_data->m_input_sniffer.add_listener(keyboard_listener);
}
void JoyconController::remove_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_data->m_input_sniffer.remove_listener(keyboard_listener);
}






ControllerClass LeftJoycon::controller_class() const{
    return ControllerClass::NintendoSwitch_LeftJoycon;
}
ControllerClass RightJoycon::controller_class() const{
    return ControllerClass::NintendoSwitch_RightJoycon;
}





}
}
