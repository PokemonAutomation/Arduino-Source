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
    PA_ADD_OPTION(JSON_FILE_NAME);
}



void RecordKeyboardController::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    AbstractControllerContext context(scope, env.console.controller());
    ControllerCategory controller_category = env.console.controller().controller_category();

    if (MODE == Mode::RECORD){
        context.controller().add_keyboard_listener(*this);

        try{
            context.wait_until_cancel();
        }catch (ProgramCancelledException&){

            if (MODE == Mode::RECORD){
                JsonValue json = controller_history_to_json(env.console.logger(), controller_category);
                json.dump(std::string(JSON_FILE_NAME) + ".json");
                m_controller_history.clear();

                json_to_cpp_code(env.console.logger(),json);

                context.controller().remove_keyboard_listener(*this);
            }
            throw;
        }        
        
    }else if (MODE == Mode::REPLAY){

    }else if (MODE == Mode::CONVERT_JSON_TO_CODE){
        JsonValue json = load_json_file(std::string(JSON_FILE_NAME) + ".json");
        json_to_cpp_code(env.console.logger(), json);


    }

}


std::string RecordKeyboardController::json_to_cpp_code(Logger& logger, const JsonValue& json){
    try{
        const JsonObject& obj = json.to_object_throw();

        std::string controller_category_string = obj.get_string_throw("controller_category");
        cout << controller_category_string << endl;
        ControllerCategory controller_category = CONTROLLER_CATEGORY_STRINGS().get_enum(controller_category_string);

        const JsonArray& history_json = obj.get_array_throw("history");

        switch (controller_category){
        case ControllerCategory::PRO_CONTROLLER:
            return json_to_cpp_code_pro_controller(history_json);
        }

        return "";
    }catch (ParseException&){
        
        throw ParseException("JSON parsing error. Given JSON file doesn't match the expected format.");
    }
}

std::string RecordKeyboardController::json_to_cpp_code_pro_controller(const JsonArray& history){
    std::string result;
    for(size_t i = 0; i < history.size(); i++){
        const JsonObject& snapshot = history[i].to_object_throw();
        int64_t duration_in_ms = snapshot.get_integer_throw("duration_in_ms");
        bool is_neutral = snapshot.get_boolean_throw("is_neutral");
        cout << duration_in_ms << endl;
        if (is_neutral){
            result += "pbf_wait(context, " + std::to_string(duration_in_ms) + "ms);\n";
        }else{
            std::string buttons_string = snapshot.get_string_throw("buttons");
            std::string dpad_string = snapshot.get_string_throw("dpad");

            Button button = string_to_button(buttons_string);
            DpadPosition dpad = string_to_dpad(dpad_string);

            int64_t left_x = snapshot.get_integer_throw("left_x");
            int64_t left_y = snapshot.get_integer_throw("left_y");
            int64_t right_x = snapshot.get_integer_throw("right_x");
            int64_t right_y = snapshot.get_integer_throw("right_y");
            
            // ensure all x, y are within STICK_MIN/MAX
            if (left_x > STICK_MAX || left_x < STICK_MIN || 
                left_y > STICK_MAX || left_y < STICK_MIN || 
                right_x > STICK_MAX || right_x < STICK_MIN || 
                right_y > STICK_MAX || right_y < STICK_MIN)
            {
                throw ParseException();
            }

            result += "pbf_controller_state(context, " 
                + button_to_code_string(button) + ", " 
                + dpad_to_code_string(dpad) + ", " 
                + std::to_string(left_x) + ", " + std::to_string(left_y) + ", " 
                + std::to_string(right_x) + ", " + std::to_string(right_y) + ", " 
                + std::to_string(duration_in_ms) +"ms);\n";
        }

    }

    cout << result << endl;
    return result;

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
    json_result["controller_category"] = CONTROLLER_CATEGORY_STRINGS().get_string(controller_category);
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

