/*  Agent Server: Input Steps
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Parses the input vocabulary of AgentTools.json (button names, d-pad directions,
 *  stick positions, input steps) into Nintendo Switch controller values.
 *
 *  This is the C++ twin of pokemon_automation/buttons.py and `InputStep` in
 *  pokemon_automation/controller.py. Both are tested against the shared cases in
 *  AgentInputTestCases.json, so an agent's inputs mean the same thing whichever
 *  server it talks to. Keep them in sync.
 *
 *  Vocabulary:
 *  - Buttons: A B X Y L R ZL ZR PLUS ("+", START) MINUS ("-", SELECT) HOME
 *    CAPTURE LCLICK (L3, LS) RCLICK (R3, RS). Case-insensitive.
 *  - D-pad: UP DOWN LEFT RIGHT and diagonals (UP_RIGHT, "up-right", UPRIGHT,
 *    DPAD_UP...). "UP" and "RIGHT" together also mean up-right.
 *  - Combinations: "L+R", "b, x", or a JSON array ["ZL", "A"].
 *  - Sticks: a direction name ("up", "down_left", "neutral") or [x, y] in [-1, 1],
 *    +y = up. Diagonal names are normalized to length 1.
 */

#ifndef PokemonAutomation_AgentServer_InputSteps_H
#define PokemonAutomation_AgentServer_InputSteps_H

#include <stdint.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include "3rdParty-Core/nlohmann/json.hpp"
#include "Controllers/Joystick.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace AgentServer{


//  Thrown for input the agent got wrong. The message is meant for the agent.
class InputError : public std::runtime_error{
public:
    using std::runtime_error::runtime_error;
};


struct ParsedButtons{
    NintendoSwitch::Button buttons = NintendoSwitch::BUTTON_NONE;
    NintendoSwitch::DpadPosition dpad = NintendoSwitch::DPAD_NONE;

    bool has_buttons() const{ return buttons != NintendoSwitch::BUTTON_NONE; }
    bool has_dpad() const{ return dpad != NintendoSwitch::DPAD_NONE; }
};

//  Parse a button combination: a string ("A", "L+R", "zl, up") or a JSON array of
//  strings. null or "" means nothing pressed.
//  Throws InputError for unknown names or contradictory d-pad directions ("up+down").
ParsedButtons parse_buttons(const nlohmann::json& value);

//  Parse a joystick position: a direction name or an [x, y] array.
//  Throws InputError for unknown names, wrong array sizes or values outside [-1, 1].
JoystickPosition parse_stick(const nlohmann::json& value);


//  One step of an input sequence (a `run_inputs` step).
//
//  A step either waits (nothing pressed: only `wait_ms`), or holds the given
//  buttons, d-pad and sticks together for `hold_ms`, releases everything for
//  `release_ms`, and repeats that `repeat` times, then waits `wait_ms`.
struct InputStep{
    ParsedButtons pressed;
    std::optional<JoystickPosition> left_stick;
    std::optional<JoystickPosition> right_stick;
    uint64_t hold_ms = 80;
    uint64_t release_ms = 80;
    uint64_t repeat = 1;
    uint64_t wait_ms = 0;
    bool wait_only = false;         //  nothing to press: the step is just `wait_ms`

    //  Total time the step occupies on the controller.
    uint64_t duration_ms() const;
    //  Short description for logs, e.g. "A x3" or "left stick (0, 1) 2000ms".
    std::string describe() const;
};

//  Parse a step object, e.g. {"buttons": "A", "hold_ms": 100}.
//  Throws InputError for unknown fields, negative durations, repeat < 1, or
//  hold_ms = 0 on a step that presses something.
InputStep parse_step(const nlohmann::json& object);



}
}
#endif
