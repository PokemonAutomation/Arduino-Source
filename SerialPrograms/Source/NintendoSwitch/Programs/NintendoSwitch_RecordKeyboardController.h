/*  Record Keyboard Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RecordKeyboardController_H
#define PokemonAutomation_NintendoSwitch_RecordKeyboardController_H

#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class ProControllerState;

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
    JsonValue controller_history_to_json(Logger& logger, ControllerCategory controller_category);

    // convert the json, with the controller history, to a string, which represents C++ code.
    std::string json_to_cpp_code(Logger& logger, const JsonValue& json);

    std::string json_to_cpp_code_pro_controller(const JsonArray& history_json);


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
    StringOption JSON_FILE_NAME;

    std::vector<ControllerStateSnapshot> m_controller_history;
    

};




}
}
#endif



