/*  Code Entry Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_CodeEntryTools_H
#define PokemonAutomation_NintendoSwitch_CodeEntryTools_H

#include <stdint.h>
#include <vector>
#include "Common/Cpp/Time.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{




enum class CodeEntryAction : uint8_t{
    ENTER_CHAR  = 0xf0,
    SCROLL_LEFT = 0xf1,
    NORM_MOVE_UP    =        (uint8_t)DpadPosition::DPAD_UP,
    NORM_MOVE_RIGHT =        (uint8_t)DpadPosition::DPAD_RIGHT,
    NORM_MOVE_DOWN  =        (uint8_t)DpadPosition::DPAD_DOWN,
    NORM_MOVE_LEFT  =        (uint8_t)DpadPosition::DPAD_LEFT,
    WRAP_MOVE_UP    = 0x80 | (uint8_t)DpadPosition::DPAD_UP,
    WRAP_MOVE_RIGHT = 0x80 | (uint8_t)DpadPosition::DPAD_RIGHT,
    WRAP_MOVE_DOWN  = 0x80 | (uint8_t)DpadPosition::DPAD_DOWN,
    WRAP_MOVE_LEFT  = 0x80 | (uint8_t)DpadPosition::DPAD_LEFT,
};
inline bool is_button_press(CodeEntryAction action){
    switch (action){
    case CodeEntryAction::ENTER_CHAR:
    case CodeEntryAction::SCROLL_LEFT:
        return true;
    default:
        return false;
    }
}
inline bool is_move(CodeEntryAction action){
    switch (action){
    case CodeEntryAction::NORM_MOVE_UP:
    case CodeEntryAction::NORM_MOVE_RIGHT:
    case CodeEntryAction::NORM_MOVE_DOWN:
    case CodeEntryAction::NORM_MOVE_LEFT:
    case CodeEntryAction::WRAP_MOVE_UP:
    case CodeEntryAction::WRAP_MOVE_RIGHT:
    case CodeEntryAction::WRAP_MOVE_DOWN:
    case CodeEntryAction::WRAP_MOVE_LEFT:
        return true;
    default:
        return false;
    }
}
inline bool is_wrap(CodeEntryAction action){
    switch (action){
    case CodeEntryAction::WRAP_MOVE_UP:
    case CodeEntryAction::WRAP_MOVE_RIGHT:
    case CodeEntryAction::WRAP_MOVE_DOWN:
    case CodeEntryAction::WRAP_MOVE_LEFT:
        return true;
    default:
        return false;
    }
}


struct CodeEntryDelays{
    Milliseconds hold;
    Milliseconds cool;
    Milliseconds press_delay;
    Milliseconds move_delay;
    Milliseconds scroll_delay;
    Milliseconds wrap_delay;
};


struct CodeEntryActionWithDelay{
    CodeEntryAction action;
    Milliseconds delay;
};




//  Given a path, optimize it and fully populate the delays.
void codeboard_populate_delays(
    bool switch2,
    std::vector<CodeEntryActionWithDelay>& path_with_delays,
    const std::vector<CodeEntryAction>& path,
    const CodeEntryDelays& delays,
    bool optimize
);





//inline Milliseconds calculate_path_delay(const std::vector<CodeEntryActionWithDelay>& path){

//}



//  Actually execute a path and enter it into the Switch.
void codeboard_execute_path(
    ProControllerContext& context,
    const CodeEntryDelays& delays,
    const std::vector<CodeEntryActionWithDelay>& path
);
















}
}
}
#endif
