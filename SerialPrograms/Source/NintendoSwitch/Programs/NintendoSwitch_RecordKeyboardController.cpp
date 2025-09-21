/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFile>
#include "Common/Cpp/Json/JsonArray.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
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
    , FILE_NAME(
        false,
        "<b>File name:</b><br>"
        "Name of the JSON file to read/write.", 
        LockMode::LOCK_WHILE_RUNNING, 
        "UserSettings/recording",
        "<name of JSON file>"
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
        json_to_pbf_actions(env, scope, json, controller_class);


    }else if (MODE == Mode::CONVERT_JSON_TO_CODE){
        JsonValue json = load_json_file(std::string(FILE_NAME) + ".json");
        json_to_cpp_code(env.console.logger(), json, FILE_NAME);


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
        case ControllerClass::PRO_CONTROLLER:
            output_text = json_to_cpp_code_pro_controller(history_json);
            break;
        case ControllerClass::LEFT_JOYCON:
        case ControllerClass::RIGHT_JOYCON:
            output_text = json_to_cpp_code_joycon(history_json);
            break;
        default:
            // generate empty text if ControllerClass is not one of the above
            break;
        }

        QFile file(QString::fromStdString(output_file_name + ".txt"));
        file.open(QIODevice::WriteOnly);
        file.write(output_text.c_str(), output_text.size());

    }catch (ParseException& e){
        logger.log(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.", COLOR_RED);
        throw ParseException(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.");
    }
}

std::string json_to_cpp_code_pro_controller(const JsonArray& history){
    std::string result;

    json_to_pro_controller_state(history, 
        [&](int64_t duration_in_ms){
            result += "pbf_wait(context, " + std::to_string(duration_in_ms) + "ms);\n";
        },
        [&](NonNeutralControllerField non_neutral_field,
            Button button, 
            DpadPosition dpad, 
            uint8_t left_x, 
            uint8_t left_y, 
            uint8_t right_x, 
            uint8_t right_y, 
            int64_t duration_in_ms
        ){

            switch (non_neutral_field){
            case NonNeutralControllerField::BUTTON:
                result += "pbf_press_button(context, " 
                    + button_to_code_string(button) + ", " 
                    + std::to_string(duration_in_ms) + "ms, 0ms);\n";
                break;
            case NonNeutralControllerField::DPAD:
                result += "pbf_press_dpad(context, " 
                    + dpad_to_code_string(dpad) + ", " 
                    + std::to_string(duration_in_ms) + "ms, 0ms);\n";
                break;
            case NonNeutralControllerField::LEFT_JOYSTICK:
                result += "pbf_move_left_joystick(context, " 
                    + std::to_string(left_x) + ", " + std::to_string(left_y) + ", " 
                    + std::to_string(duration_in_ms) + "ms, 0ms);\n";
                break;
            case NonNeutralControllerField::RIGHT_JOYSTICK:
                result += "pbf_move_right_joystick(context, " 
                    + std::to_string(right_x) + ", " + std::to_string(right_y) + ", "
                    + std::to_string(duration_in_ms) + "ms, 0ms);\n";
                break;
            case NonNeutralControllerField::MULTIPLE:
                result += "pbf_controller_state(context, " 
                    + button_to_code_string(button) + ", " 
                    + dpad_to_code_string(dpad) + ", " 
                    + std::to_string(left_x) + ", " + std::to_string(left_y) + ", " 
                    + std::to_string(right_x) + ", " + std::to_string(right_y) + ", " 
                    + std::to_string(duration_in_ms) +"ms);\n";
                break;
            case NonNeutralControllerField::NONE:
                result += "pbf_wait(context, " + std::to_string(duration_in_ms) + "ms);\n";
                break;
            default:
                throw ParseException("Unexpected NonNeutralControllerField enum.");
            }            
        }
    );
    
    cout << result << endl;
    return result;

}

std::string json_to_cpp_code_joycon(const JsonArray& history){
    std::string result;

    json_to_joycon_state(history, 
        [&](int64_t duration_in_ms){
            result += "pbf_wait(context, " + std::to_string(duration_in_ms) + "ms);\n";
        },
        [&](NonNeutralControllerField non_neutral_field,
            Button button, 
            uint8_t x, 
            uint8_t y, 
            int64_t duration_in_ms
        ){

            switch (non_neutral_field){
            case NonNeutralControllerField::BUTTON:
                result += "pbf_press_button(context, " 
                    + button_to_code_string(button) + ", " 
                    + std::to_string(duration_in_ms) + "ms, 0ms);\n";
                break;
            case NonNeutralControllerField::JOYSTICK:
                result += "pbf_move_left_joystick(context, " 
                    + std::to_string(x) + ", " + std::to_string(y) + ", " 
                    + std::to_string(duration_in_ms) + "ms, 0ms);\n";
                break;
            case NonNeutralControllerField::MULTIPLE:
                result += "pbf_controller_state(context, " 
                    + button_to_code_string(button) + ", " 
                    + std::to_string(x) + ", " + std::to_string(y) + ", " 
                    + std::to_string(duration_in_ms) +"ms);\n";
                break;
            case NonNeutralControllerField::NONE:
                result += "pbf_wait(context, " + std::to_string(duration_in_ms) + "ms);\n";
                break;
            default:
                throw ParseException("Unexpected NonNeutralControllerField enum.");
            }            
        }
    );
    
    cout << result << endl;
    return result;

}


void json_to_pbf_actions(SingleSwitchProgramEnvironment& env, CancellableScope& scope, const JsonValue& json, ControllerClass controller_class){
    try{
        const JsonObject& obj = json.to_object_throw();

        std::string controller_class_string = obj.get_string_throw("controller_class");
        ControllerClass controller_class_json = CONTROLLER_CLASS_STRINGS().get_enum(controller_class_string);
        if (controller_class_json != controller_class){
            throw UserSetupError(env.logger(), "Controller class in the JSON file does not match your current selected controller.");
        }

        const JsonArray& history_json = obj.get_array_throw("history");

        switch (controller_class){
        case ControllerClass::PRO_CONTROLLER:
        {
            ProControllerContext context(scope, env.console.controller<ProController>());
            json_to_pbf_actions_pro_controller(context, history_json);
            break;
        }
        case ControllerClass::LEFT_JOYCON:
        case ControllerClass::RIGHT_JOYCON:
        {
            JoyconContext context(scope, env.console.controller<JoyconController>());
            json_to_pbf_actions_joycon(context, history_json);
            break;
        }
        default:
            // do nothing if the ControllerClass is not one of the above.
            break;
        }

    }catch (ParseException& e){
        env.log(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.", COLOR_RED);
        throw ParseException(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.");
    }    

}

void json_to_pbf_actions_pro_controller(ProControllerContext& context, const JsonArray& history){

    json_to_pro_controller_state(history, 
        [&](int64_t duration_in_ms){
            pbf_wait(context, Milliseconds(duration_in_ms));
        },
        [&](NonNeutralControllerField non_neutral_field,
            Button button, 
            DpadPosition dpad, 
            uint8_t left_x, 
            uint8_t left_y, 
            uint8_t right_x, 
            uint8_t right_y, 
            int64_t duration_in_ms
        ){
            switch (non_neutral_field){
            case NonNeutralControllerField::BUTTON:
                pbf_press_button(context, button, Milliseconds(duration_in_ms), Milliseconds(0));
                break;
            case NonNeutralControllerField::DPAD:
                pbf_press_dpad(context, dpad, Milliseconds(duration_in_ms), Milliseconds(0));
                break;
            case NonNeutralControllerField::LEFT_JOYSTICK:
                pbf_move_left_joystick(context, left_x, left_y, Milliseconds(duration_in_ms), Milliseconds(0));
                break;
            case NonNeutralControllerField::RIGHT_JOYSTICK:
                pbf_move_right_joystick(context, right_x, right_y, Milliseconds(duration_in_ms), Milliseconds(0));
                break;
            case NonNeutralControllerField::MULTIPLE:
                pbf_controller_state(context, button, dpad, left_x, left_y, right_x, right_y, Milliseconds(duration_in_ms));
                break;
            case NonNeutralControllerField::NONE:
                pbf_wait(context, Milliseconds(duration_in_ms));
                break;
            default:
                throw ParseException("Unexpected NonNeutralControllerField enum.");
            }            
        }
    );
}

void json_to_pbf_actions_joycon(JoyconContext& context, const JsonArray& history){

    json_to_joycon_state(history, 
        [&](int64_t duration_in_ms){
            pbf_wait(context, Milliseconds(duration_in_ms));
        },
        [&](NonNeutralControllerField non_neutral_field,
            Button button, 
            uint8_t x, 
            uint8_t y, 
            int64_t duration_in_ms
        ){
            switch (non_neutral_field){
            case NonNeutralControllerField::BUTTON:
                pbf_press_button(context, button, Milliseconds(duration_in_ms), Milliseconds(0));
                break;
            case NonNeutralControllerField::JOYSTICK:
                pbf_move_joystick(context, x, y, Milliseconds(duration_in_ms), Milliseconds(0));
                break;
            case NonNeutralControllerField::MULTIPLE:
                pbf_controller_state(context, button, x, y, Milliseconds(duration_in_ms));
                break;
            case NonNeutralControllerField::NONE:
                pbf_wait(context, Milliseconds(duration_in_ms));
                break;
            default:
                throw ParseException("Unexpected NonNeutralControllerField enum.");
            }            
        }
    );
}


NonNeutralControllerField get_non_neutral_pro_controller_field(Button button, DpadPosition dpad, uint8_t left_x, uint8_t left_y, uint8_t right_x, uint8_t right_y){
    NonNeutralControllerField non_neutral_field = NonNeutralControllerField::NONE;
    int8_t num_non_neutral_fields = 0;
    if (button != BUTTON_NONE) { 
        num_non_neutral_fields++;
        // if (num_non_neutral_fields > 1){
        //     return NonNeutralControllerField::MULTIPLE;
        // }
        non_neutral_field = NonNeutralControllerField::BUTTON; 
    }

    if (dpad != DPAD_NONE){
        num_non_neutral_fields++;
        if (num_non_neutral_fields > 1){
            return NonNeutralControllerField::MULTIPLE;
        }
        non_neutral_field = NonNeutralControllerField::DPAD; 
    }

    if (left_x != STICK_CENTER || left_y != STICK_CENTER){
        num_non_neutral_fields++;
        if (num_non_neutral_fields > 1){
            return NonNeutralControllerField::MULTIPLE;
        }
        non_neutral_field = NonNeutralControllerField::LEFT_JOYSTICK; 
    }

    if (right_x != STICK_CENTER || right_y != STICK_CENTER){
        num_non_neutral_fields++;
        if (num_non_neutral_fields > 1){
            return NonNeutralControllerField::MULTIPLE;
        }
        non_neutral_field = NonNeutralControllerField::RIGHT_JOYSTICK; 
    }

    return non_neutral_field;
}

NonNeutralControllerField get_non_neutral_joycon_controller_field(Button button, uint8_t x, uint8_t y){
    NonNeutralControllerField non_neutral_field = NonNeutralControllerField::NONE;
    int8_t num_non_neutral_fields = 0;
    if (button != BUTTON_NONE) { 
        num_non_neutral_fields++;
        // if (num_non_neutral_fields > 1){
        //     return NonNeutralControllerField::MULTIPLE;
        // }
        non_neutral_field = NonNeutralControllerField::BUTTON; 
    }

    if (x != STICK_CENTER || y != STICK_CENTER){
        num_non_neutral_fields++;
        if (num_non_neutral_fields > 1){
            return NonNeutralControllerField::MULTIPLE;
        }
        non_neutral_field = NonNeutralControllerField::JOYSTICK; 
    }

    return non_neutral_field;
}



void json_to_pro_controller_state(
    const JsonArray& history, 
    std::function<void(int64_t duration_in_ms)>&& neutral_action,
    std::function<void(
        NonNeutralControllerField non_neutral_field,
        Button button, 
        DpadPosition dpad, 
        uint8_t left_x, 
        uint8_t left_y, 
        uint8_t right_x, 
        uint8_t right_y, 
        int64_t duration_in_ms
    )>&& non_neutral_action
){
    for(size_t i = 0; i < history.size(); i++){
        const JsonObject& snapshot = history[i].to_object_throw();
        int64_t duration_in_ms = snapshot.get_integer_throw("duration_in_ms");
        bool is_neutral = snapshot.get_boolean_throw("is_neutral");
        if (is_neutral){
            neutral_action(duration_in_ms);
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
                throw ParseException("x or y values are outside of 0-255.");
            }

            NonNeutralControllerField non_neutral_field = get_non_neutral_pro_controller_field(button, dpad, left_x, left_y, right_x, right_y);

            non_neutral_action(non_neutral_field, button, dpad, left_x, left_y, right_x, right_y, duration_in_ms);
            
        }

    }

}

void json_to_joycon_state(
    const JsonArray& history, 
    std::function<void(int64_t duration_in_ms)>&& neutral_action,
    std::function<void(
        NonNeutralControllerField non_neutral_field,
        Button button, 
        uint8_t x, 
        uint8_t y, 
        int64_t duration_in_ms
    )>&& non_neutral_action
){
    for(size_t i = 0; i < history.size(); i++){
        const JsonObject& snapshot = history[i].to_object_throw();
        int64_t duration_in_ms = snapshot.get_integer_throw("duration_in_ms");
        bool is_neutral = snapshot.get_boolean_throw("is_neutral");
        if (is_neutral){
            neutral_action(duration_in_ms);
        }else{
            std::string buttons_string = snapshot.get_string_throw("buttons");

            Button button = string_to_button(buttons_string);

            int64_t x = snapshot.get_integer_throw("joystick_x");
            int64_t y = snapshot.get_integer_throw("joystick_y");
            
            // ensure x, y are within STICK_MIN/MAX
            if (x > STICK_MAX || x < STICK_MIN || 
                y > STICK_MAX || y < STICK_MIN)
            {
                throw ParseException("x or y values are outside of 0-255.");
            }

            NonNeutralControllerField non_neutral_field = get_non_neutral_joycon_controller_field(button, x, y);

            non_neutral_action(non_neutral_field, button, x, y, duration_in_ms);
            
        }

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
        JsonObject& controller_state = snapshot.controller_state;
        
        WallClock prev_time_stamp = prev_snapshot->time_stamp;
        JsonObject& prev_controller_state = prev_snapshot->controller_state;

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

        // Normalized timestamps		Diff using normalized timestamps    Total time since start
        // 0ms                          1ms                                 0ms
        // 1.4ms -> 1ms                 2ms	                                1ms
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

