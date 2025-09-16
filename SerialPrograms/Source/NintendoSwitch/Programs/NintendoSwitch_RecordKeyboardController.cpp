/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Json/JsonArray.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch_RecordKeyboardController.h"
#include "Controllers/ControllerTypeStrings.h"

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

RecordKeyboardController::RecordKeyboardController() 
    : MODE(
        "<b>Mode:</b>",
        {
            {Mode::RECORD,   "record", "Record "},
            {Mode::REPLAY,  "replay", "Replay"},
            {Mode::CONVERT_JSON_TO_CODE,       "convert-to-code", "[For Developers] Convert json to code."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Mode::RECORD
    )
    , JSON_FILE_NAME(
        false,
        "Name of the JSON file to read/write.", 
        LockMode::LOCK_WHILE_RUNNING, 
        "recording",
        "<name of JSON file>"
    )
{
    PA_ADD_OPTION(MODE);
}



void RecordKeyboardController::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    AbstractControllerContext context(scope, env.console.controller());
    ControllerCategory controller_category = env.console.controller().controller_category();

    if (MODE == Mode::RECORD){
        context.controller().add_keyboard_listener(*this);
        
    }else if (MODE == Mode::REPLAY){

    }else if (MODE == Mode::CONVERT_JSON_TO_CODE){

    }


    try{
        context.wait_until_cancel();
    }catch (ProgramCancelledException&){

        if (MODE == Mode::RECORD){
            JsonValue json = controller_history_to_json(env.console.logger(), controller_category);
            json.dump(std::string(JSON_FILE_NAME) + ".json");
            m_controller_history.clear();

            json_to_cpp_code(json);

            context.controller().remove_keyboard_listener(*this);
        }
        throw;
    }
}


std::string RecordKeyboardController::json_to_cpp_code(const JsonValue& json){
    // std::string controller_category = json[]

    return "";
}

JsonValue RecordKeyboardController::controller_history_to_json(Logger& logger, ControllerCategory controller_category){
    if (m_controller_history.size() < 2){
        // throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "RecordKeyboardController:: m_controller_history should have at least two entries, start and stop.");
        logger.log("RecordKeyboardController:: We expected m_controller_history to have at least two entries, start and stop. Aborting.", COLOR_RED);
        return JsonValue();
    }
    
    JsonArray json_array;
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
        json_array.push_back(std::move(recording));
        prev_snapshot = &snapshot; // update the previous non-duplicate snapshot
    }
    
    
    JsonObject json_result;
    json_result["controller_category"] = CONTROLLER_CATEGORY_STRINGS.get_string(controller_category);
    json_result["history"] = JsonValue(std::move(json_array));

    return json_result;

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

