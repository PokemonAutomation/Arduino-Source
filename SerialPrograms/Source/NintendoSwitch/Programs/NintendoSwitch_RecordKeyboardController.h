/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RecordKeyboardController_H
#define PokemonAutomation_NintendoSwitch_RecordKeyboardController_H

#include <functional>
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class ProControllerState;

enum class NonNeutralControllerField {
    BUTTON,
    DPAD,
    LEFT_JOYSTICK,
    RIGHT_JOYSTICK,
    JOYSTICK,
    MULTIPLE,
    NONE,
};
NonNeutralControllerField get_non_neutral_pro_controller_field(Button button, DpadPosition dpad, uint8_t left_x, uint8_t left_y, uint8_t right_x, uint8_t right_y);
NonNeutralControllerField get_non_neutral_joycon_controller_field(Button button, uint8_t x, uint8_t y);

// helper function that exposes the Pro Controller fields from the given JSON
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
);

// helper function that exposes the Joycon fields from the given JSON
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
);

// given the json, with the controller history, output a text file which represents C++ code.
void json_to_cpp_code(Logger& logger, const JsonValue& json, const std::string& output_file_name);
std::string json_to_cpp_code_pro_controller(const JsonArray& history_json);
std::string json_to_cpp_code_joycon(const JsonArray& history);

// given the json, with the controller history, run the controller actions using the pbf functions.
void json_to_pbf_actions(SingleSwitchProgramEnvironment& env, CancellableScope& scope, const JsonValue& json, ControllerClass controller_class);
void json_to_pbf_actions_pro_controller(ProControllerContext& context, const JsonArray& history);
void json_to_pbf_actions_joycon(JoyconContext& context, const JsonArray& history);

class RecordKeyboardController_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RecordKeyboardController_Descriptor();
};



class RecordKeyboardController : public SingleSwitchProgramInstance, public KeyboardEventHandler::KeyboardListener{ 
public:
    ~RecordKeyboardController();
    RecordKeyboardController();
    

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    // whenever a keyboard command is sent/stopped: 
    // add to m_controller_history the time_stamp and the ControllerState serialized to JSON.
    virtual void on_keyboard_command_sent(WallClock time_stamp, const ControllerState& state) override;
    virtual void on_keyboard_command_stopped(WallClock time_stamp) override;

    // convert m_controller_history to json
    // remove adjacent duplicate controller states.
    JsonValue controller_history_to_json(Logger& logger, ControllerClass controller_class);


    // Examples for JsonObject controller_state:
    // ProControllerState:
        // {
        //     "is_neutral": false
        //     "buttons": "L R",
        //     "dpad": "none",
        //     "left_x": 100,
        //     "left_y": 200,
        //     "right_x": 128,
        //     "right_y": 128,
        // }
    // JoyconState
        // {
        //     "is_neutral": false
        //     "buttons": "L R",
        //     "joystick_x": 100,
        //     "joystick_x": 200,
        // }
    // Neutral controller: we know the controller is neutral, so no need for all the other button info
        // {
        //     "is_neutral": true
        // }
    struct ControllerStateSnapshot {
        WallClock time_stamp;
        JsonObject controller_state;
    };

private:
    enum class Mode{
        RECORD,
        REPLAY,
        CONVERT_JSON_TO_CODE,
    };
    EnumDropdownOption<Mode> MODE;
    StringOption FILE_NAME;
    BooleanCheckBoxOption GENERATE_CPP_CODE_AFTER_RECORDING;

    std::vector<ControllerStateSnapshot> m_controller_history;
    

};



}
}
#endif



