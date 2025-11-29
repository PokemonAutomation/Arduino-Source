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
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class ProControllerState;



// given the json, with the controller history, output a text file which represents C++ code.
void json_to_cpp_code(Logger& logger, const JsonValue& json, const std::string& output_file_name);

// given the json, with the controller history, run the controller actions using the pbf functions.
void json_to_pbf_actions(
    SingleSwitchProgramEnvironment& env,
    CancellableScope& scope,
    const JsonValue& json,
    ControllerClass controller_class,
    uint32_t num_loops,
    uint32_t seconds_wait_between_loops
);

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
    struct ControllerStateSnapshot{
        WallClock time_stamp;
        JsonValue controller_state;
    };

private:
    enum class Mode{
        RECORD,
        REPLAY,
        CONVERT_JSON_TO_CODE,
    };
    EnumDropdownOption<Mode> MODE;
    StringOption FILE_NAME;
    SimpleIntegerOption<uint32_t> LOOP;
    SimpleIntegerOption<uint32_t> WAIT;
    BooleanCheckBoxOption GENERATE_CPP_CODE_AFTER_RECORDING;

    std::vector<ControllerStateSnapshot> m_controller_history;
    

};



}
}
#endif



