/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Json/JsonArray.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch_RecordKeyboardController.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


RecordKeyboardController_Descriptor::RecordKeyboardController_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:RecordKeyboardController",
        "Nintendo Switch", "Record Keyboard Controller",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/RecordKeyboardController.md",
        "Record actions from the keyboard controller, then play it back.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}



RecordKeyboardController::~RecordKeyboardController(){
    
    // m_keyboard_manager->remove_listener(*this);
}

RecordKeyboardController::RecordKeyboardController(){
}



void RecordKeyboardController::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    AbstractControllerContext context(scope, env.console.controller());
    // ProControllerContext context(scope, env.console.controller<ProController>());
    context.controller().add_keyboard_listener(*this);
    // CONTROLLER_TYPE_STRINGS.get_string(connection.current_controller());

    try{
        context.wait_until_cancel();
    }catch (ProgramCancelledException&){

        JsonValue json = controller_history_to_json(env.console.logger());
        json.dump("recording.json");
        m_controller_history.clear();
        context.controller().remove_keyboard_listener(*this);
        throw;
    }

    
    
}

JsonValue RecordKeyboardController::controller_history_to_json(Logger& logger){
    if (m_controller_history.size() < 2){
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "RecordKeyboardController:: m_controller_history should have at least two entries, start and stop.");
    }
    
    JsonArray json;
    ControllerStateSnapshot* prev_snapshot = &m_controller_history[0]; // the previous non-duplicate snapshot

    for (size_t i = 1; i < m_controller_history.size(); i++){ // start at index i = 1, since prev_snapshot starts at i=0 and we continue when current == previous.
        ControllerStateSnapshot& snapshot = m_controller_history[i];
        WallClock time_stamp = snapshot.time_stamp;
        JsonObject& controller_state = snapshot.controller_state;
        
        WallClock prev_time_stamp = prev_snapshot->time_stamp;
        JsonObject& prev_controller_state = prev_snapshot->controller_state;

        if (controller_state == prev_controller_state){
            continue;
        }
        
        // cout << time_stamp << endl;
        // cout << prev_time_stamp << endl;
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(time_stamp - prev_time_stamp); // might need to figure out how rounding works.
        int64_t duration = elapsed_time.count();
        // cout << std::to_string(duration) << endl;
        // cout << prev_controller_state.dump() << endl;
        JsonObject recording = prev_controller_state.clone();
        recording["duration_in_ms"] = duration;
        json.push_back(std::move(recording));
        prev_snapshot = &snapshot; // update the previous non-duplicate snapshot
    }
    

    return json;

}


void RecordKeyboardController::on_keyboard_command_sent(WallClock time_stamp, const ControllerState& state){
    cout << "keyboard_command_sent" << endl;
    JsonObject serialized_state = state.serialize_state();
    cout << serialized_state.dump(0) << endl;
    
    ControllerStateSnapshot state_snapshot = {
        time_stamp,
        std::move(serialized_state)
    };
    m_controller_history.emplace_back(std::move(state_snapshot));
}
void RecordKeyboardController::on_keyboard_command_stopped(WallClock time_stamp){
    cout << "keyboard_command_stopped" << endl;
    JsonObject obj;
    obj["is_neutral"] = true;

    ControllerStateSnapshot state_snapshot = {
        time_stamp, 
        std::move(obj)
    };
    m_controller_history.emplace_back(std::move(state_snapshot));
}



}
}

