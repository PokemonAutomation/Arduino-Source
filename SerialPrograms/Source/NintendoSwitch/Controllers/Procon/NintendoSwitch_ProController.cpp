/*  Nintendo Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Strings/StringTools.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "ControllerInput/ControllerInput.h"
#include "ControllerInput/Keyboard/KeyboardInput_State.h"
#include "Controllers/RumbleListener.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_ProControllerState.h"
#include "NintendoSwitch_ProController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;


const char ProController::NAME[] = "Nintendo Switch: Pro Controller";





struct ProController::Data{
    ListenerSet<RumbleListener> m_rumble_listeners;
    std::map<KeyboardKey, ProControllerDeltas> m_keyboard_mapping;
};



void ProController::add_listener(RumbleListener& listener){
    m_data->m_rumble_listeners.add(listener);
}
void ProController::remove_listener(RumbleListener& listener){
    m_data->m_rumble_listeners.remove(listener);
}




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

ControllerClass ProController::controller_class() const noexcept{
    return ControllerClass::NintendoSwitch_ProController;
}





bool ProController::run_string_command(Milliseconds duration, const std::string& command){
    //  stop
    //  replace
    //  wait
    //  A,B
    //  A,B|JSL:+0.5:-0.5

    std::vector<std::string> tokens = StringTools::split(command, "|");
    if (tokens.empty()){
        return false;
    }

    if (tokens[0] == "stop"){
        cancel_all_commands();
        return true;
    }
    if (tokens[0] == "replace"){
        replace_on_next_command();
        return true;
    }
    if (tokens[0] == "wait"){
        issue_nop(nullptr, duration);
        return true;
    }

    ProControllerState state;

    for (const std::string& token : tokens){
        if (token.starts_with("JSL")){
            std::vector<std::string> args = StringTools::split(token, ":");
            if (args.size() != 3){
                return false;
            }
            state.left_joystick.x = std::atof(args[1].data());
            state.left_joystick.y = std::atof(args[2].data());
            continue;
        }
        if (token.starts_with("JSR")){
            std::vector<std::string> args = StringTools::split(token, ":");
            if (args.size() != 3){
                return false;
            }
            state.right_joystick.x = std::atof(args[1].data());
            state.right_joystick.y = std::atof(args[2].data());
            continue;
        }
        state.buttons |= string_to_button(token, ",");
    }

    state.execute(nullptr, true, *this, duration);

    return true;
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

    ProControllerState controller_state;
    deltas.to_state(controller_state);

    WallClock timestamp;
    if (controller_state.is_neutral()){
        timestamp = current_time();
        cancel_all_commands();
    }else{
        replace_on_next_command();

        timestamp = current_time();
        controller_state.execute(nullptr, false, *this, 2000ms);
    }

    on_command_input(timestamp, controller_state);
}


void ProController::on_rumble(double magnitude){
    m_data->m_rumble_listeners.run_method(&RumbleListener::on_rumble, magnitude);
}





}
}
