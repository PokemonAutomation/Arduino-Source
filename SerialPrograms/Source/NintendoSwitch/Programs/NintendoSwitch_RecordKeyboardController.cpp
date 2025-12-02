/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFile>
#include "Common/Cpp/Json/JsonArray.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProControllerState.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_JoyconState.h"
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
        "Programs/NintendoSwitch/RecordKeyboardController.html",
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
    , FILE_NAME(
        false,
        "<b>File name:</b><br>"
        "Name of the JSON file to read/write.", 
        LockMode::LOCK_WHILE_RUNNING, 
        "UserSettings/recording",
        "<name of JSON file>"
    )
    , LOOP(
        "<b>Number of times to loop:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0
    )
    , WAIT(
        "<b>Number of seconds between loops:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        5, 0
    )
    , GENERATE_CPP_CODE_AFTER_RECORDING(
        "<b>[For Developers]</b><br>"
        "<b>Automatically generate C++ code:</b><br>"
        "After recording, a text file with the C++ code will automatically be generated.<br>"
        "The name of the text file will be as per \"File name\" above.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(FILE_NAME);
    PA_ADD_OPTION(LOOP);
    PA_ADD_OPTION(WAIT);
    PA_ADD_OPTION(GENERATE_CPP_CODE_AFTER_RECORDING);
}



void RecordKeyboardController::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    AbstractControllerContext context(scope, env.console.controller());
    ControllerClass controller_class = env.console.controller().controller_class();

    if (MODE == Mode::RECORD){
        // check if given file name already exists. If it does, throw an error so we don't overwrite it.
        std::string output_json_filename = std::string(FILE_NAME) + ".json";
        QFile file(QString::fromStdString(output_json_filename));
        if (file.open(QFile::ReadOnly)){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Given file name already exists. Choose a different file name.", output_json_filename);
        }

        context.controller().add_keyboard_listener(*this);

        try{
            context.wait_until_cancel();
        }catch (ProgramCancelledException&){

            JsonValue json = controller_history_to_json(env.console.logger(), controller_class);
            json.dump(output_json_filename);
            m_controller_history.clear();

            if (GENERATE_CPP_CODE_AFTER_RECORDING){
                json_to_cpp_code(env.console.logger(), json, FILE_NAME);
            }

            context.controller().remove_keyboard_listener(*this);
            
            throw;
        }        
        
    }else if (MODE == Mode::REPLAY){
        JsonValue json = load_json_file(std::string(FILE_NAME) + ".json");
        json_to_pbf_actions(env, scope, json, controller_class, LOOP, WAIT);


    }else if (MODE == Mode::CONVERT_JSON_TO_CODE){
        JsonValue json = load_json_file(std::string(FILE_NAME) + ".json");
        json_to_cpp_code(env.console.logger(), json, FILE_NAME);


    }

}



template <typename ControllerState>
std::string json_to_cpp(const JsonArray& history){
    std::string ret;
    ControllerState state;
    for (const JsonValue& command : history){
        const JsonObject& snapshot = command.to_object_throw();
        Milliseconds duration(snapshot.get_integer_throw("duration_in_ms"));
        state.load_json(command);
        ret += state.to_cpp(duration, Milliseconds(0));
    }
    return ret;
}

template <typename ControllerState>
void execute_json_schedule(
    AbstractControllerContext& context,
    const JsonArray& history,
    uint32_t num_loops,
    uint32_t seconds_wait_between_loops
){
    ControllerState state;
    for (uint32_t i = 0; i < num_loops; i++){
        for (const JsonValue& command : history){
            const JsonObject& snapshot = command.to_object_throw();
            Milliseconds duration(snapshot.get_integer_throw("duration_in_ms"));
            state.load_json(command);
            state.execute(context, context.controller(), duration);
        }
        state.clear();
        state.execute(context, context.controller(), Seconds(seconds_wait_between_loops));
    }
}



void json_to_cpp_code(Logger& logger, const JsonValue& json, const std::string& output_file_name){
    try{
        const JsonObject& obj = json.to_object_throw();

        std::string controller_class_string = obj.get_string_throw("controller_class");
        cout << controller_class_string << endl;
        ControllerClass controller_class = CONTROLLER_CLASS_STRINGS().get_enum(controller_class_string);

        const JsonArray& history_json = obj.get_array_throw("history");

        std::string output_text;
        switch (controller_class){
        case ControllerClass::NintendoSwitch_ProController:
            output_text = json_to_cpp<ProControllerState>(history_json);
            break;
        case ControllerClass::NintendoSwitch_LeftJoycon:
        case ControllerClass::NintendoSwitch_RightJoycon:
            output_text = json_to_cpp<JoyconState>(history_json);
            break;
        default:
            // generate empty text if ControllerClass is not one of the above
            break;
        }

        QFile file(QString::fromStdString(output_file_name + ".txt"));
        if (file.open(QIODevice::WriteOnly)){
            file.write(output_text.c_str(), output_text.size());
        }

    }catch (ParseException& e){
        logger.log(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.", COLOR_RED);
        throw ParseException(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.");
    }
}
void json_to_pbf_actions(
    SingleSwitchProgramEnvironment& env,
    CancellableScope& scope,
    const JsonValue& json,
    ControllerClass controller_class,
    uint32_t num_loops,
    uint32_t seconds_wait_between_loops
){
    try{
        const JsonObject& obj = json.to_object_throw();

        std::string controller_class_string = obj.get_string_throw("controller_class");
        ControllerClass controller_class_json = CONTROLLER_CLASS_STRINGS().get_enum(controller_class_string);
        if (controller_class_json != controller_class){
            throw UserSetupError(env.logger(), "Controller class in the JSON file does not match your current selected controller.");
        }

        const JsonArray& history_json = obj.get_array_throw("history");

        AbstractControllerContext acontext(scope, env.console.controller());

        switch (controller_class){
        case ControllerClass::NintendoSwitch_ProController:
            execute_json_schedule<ProControllerState>(acontext, history_json, num_loops, seconds_wait_between_loops);
            break;
        case ControllerClass::NintendoSwitch_LeftJoycon:
        case ControllerClass::NintendoSwitch_RightJoycon:
            execute_json_schedule<JoyconState>(acontext, history_json, num_loops, seconds_wait_between_loops);
            break;
        default:
            // do nothing if the ControllerClass is not one of the above.
            break;
        }

    }catch (ParseException& e){
        env.log(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.", COLOR_RED);
        throw ParseException(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.");
    }

}




JsonValue RecordKeyboardController::controller_history_to_json(Logger& logger, ControllerClass controller_class){
    if (m_controller_history.size() < 2){
        // throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "RecordKeyboardController:: m_controller_history should have at least two entries, start and stop.");
        logger.log("RecordKeyboardController:: We expected m_controller_history to have at least two entries, start and stop. Aborting.", COLOR_RED);
        return JsonValue();
    }
    
    JsonArray json_array;
    ControllerStateSnapshot* prev_snapshot = &m_controller_history[0]; // the previous non-duplicate snapshot
    WallClock initial_time_stamp = m_controller_history[0].time_stamp;

    for (size_t i = 1; i < m_controller_history.size(); i++){ // start at index i = 1, since prev_snapshot starts at i=0 and we continue when current == previous.
        ControllerStateSnapshot& snapshot = m_controller_history[i];
        WallClock time_stamp = snapshot.time_stamp;
        JsonObject& controller_state = snapshot.controller_state.to_object_throw();
        
        WallClock prev_time_stamp = prev_snapshot->time_stamp;
        JsonObject& prev_controller_state = prev_snapshot->controller_state.to_object_throw();

        if (controller_state == prev_controller_state){
            continue;
        }
        
        // Normalize each time stamp relative to the initial time stamp, and round to milliseconds.
        // When only considering the distance between adjacent timestamps, you end up with drift due to rounding from nanoseconds to milliseconds, 

        // example:
        // Timestamps           Diff only comparing adjacent                Total time since start
        // 12:00 1us            1.4us -> 1ms                                0ms
        // 12:00 1401us         1.4us -> 1ms                                1ms
        // 12:00 2801us         1.4us -> 1ms                                2ms
        // 12:00 4201us         1.4us -> 1ms                                3ms
        // 12:00 5601us         1.4us -> 1ms                                4ms
        // 12:00 7001us         1.4us -> 1ms                                5ms
        // 12:00 8401us         1.4us -> 1ms                                6ms
        // 12:00 9801us         1.4us -> 1ms                                7ms
        // 12:00 11201us                                                    8ms
        // total time elapsed: 11.2ms vs 8ms

        // Normalized timestamps        Diff using normalized timestamps    Total time since start
        // 0ms                          1ms                                 0ms
        // 1.4ms -> 1ms                 2ms                                 1ms
        // 2.8ms -> 3ms                 1ms                                 3ms
        // 4.2ms -> 4ms                 2ms                                 4ms
        // 5.6ms -> 6ms                 1ms                                 6ms
        // 7.0ms -> 7ms                 1ms                                 7ms
        // 8.4ms -> 8ms                 2ms                                 8ms
        // 9.8ms -> 10ms                1ms                                 10ms
        // 11.2ms -> 11ms                                                   11ms
        // total time elapsed: 11.2ms vs 11ms


        Milliseconds current_timestamp_time_since_start = 
            std::chrono::round<Milliseconds>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_stamp - initial_time_stamp)); // find the time difference as nanoseconds, then round to milliseconds
        Milliseconds prev_timestamp_time_since_start = 
            std::chrono::round<Milliseconds>(std::chrono::duration_cast<std::chrono::nanoseconds>(prev_time_stamp - initial_time_stamp)); 
        Milliseconds elapsed_time = current_timestamp_time_since_start - prev_timestamp_time_since_start;
        int64_t duration = elapsed_time.count();

        // cout << std::to_string(duration) << endl;
        // cout << prev_controller_state.dump() << endl;
        JsonObject recording = prev_controller_state.clone();
        recording["duration_in_ms"] = duration;
        json_array.push_back(std::move(recording));
        prev_snapshot = &snapshot; // update the previous non-duplicate snapshot
    }
    
    
    JsonObject json_result;
    json_result["controller_class"] = CONTROLLER_CLASS_STRINGS().get_string(controller_class);
    json_result["history"] = JsonValue(std::move(json_array));

    return json_result;

}


void RecordKeyboardController::on_keyboard_command_sent(WallClock time_stamp, const ControllerState& state){
    cout << "keyboard_command_sent" << endl;
    JsonValue serialized_state = state.to_json();
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
//    obj["is_neutral"] = true;

    ControllerStateSnapshot state_snapshot = {
        time_stamp, 
        std::move(obj)
    };
    m_controller_history.emplace_back(std::move(state_snapshot));
}



}
}

