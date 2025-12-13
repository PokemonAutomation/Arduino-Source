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
    KeyboardEventHandler m_input_sniffer;
};




ProController::ProController(Logger& logger)
    : m_data(CONSTRUCT_TOKEN)
{
    std::vector<std::shared_ptr<EditableTableRow>> mapping =
        ConsoleSettings::instance().KEYBOARD_MAPPINGS.PRO_CONTROLLER.current_refs();

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

void ProController::controller_input_state(const ControllerInputState& state){
//    cout << "controller_input_state()" << endl;

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

    WallClock timestamp = current_time();

    ProControllerState controller_state;
    deltas.to_state(controller_state);
    issue_full_controller_state(
        nullptr,
        false,
        2000ms,
        controller_state.buttons,
        controller_state.dpad,
        controller_state.left_x,
        controller_state.left_y,
        controller_state.right_x,
        controller_state.right_y
    );

    m_data->m_input_sniffer.report_keyboard_command_sent(timestamp, controller_state);
}

void ProController::add_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_data->m_input_sniffer.add_listener(keyboard_listener);
}
void ProController::remove_keyboard_listener(KeyboardEventHandler::KeyboardListener& keyboard_listener){
    m_data->m_input_sniffer.remove_listener(keyboard_listener);
}




}
}
