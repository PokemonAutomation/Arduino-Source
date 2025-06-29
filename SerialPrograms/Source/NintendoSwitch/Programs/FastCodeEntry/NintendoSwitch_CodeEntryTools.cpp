/*  Code Entry Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_CodeEntryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{




void codeboard_populate_delays(
    bool switch2,
    std::vector<CodeEntryActionWithDelay>& path_with_delays,
    const std::vector<CodeEntryAction>& path,
    const CodeEntryDelays& delays,
    bool optimize
){
    //  Populate and assign default delays first.
    path_with_delays.resize(path.size());
    for (size_t c = 0; c < path_with_delays.size(); c++){
        CodeEntryAction action = path[c];
        Milliseconds delay;
        if (action == CodeEntryAction::ENTER_CHAR){
            delay = delays.press_delay;
        }else if (action == CodeEntryAction::SCROLL_LEFT){
            delay = delays.scroll_delay;
        }else{
            delay = delays.move_delay;
        }

        //  If we are wrapping and the previous action is a move, then we can't
        //  overlap.
        if (is_wrap(action) && c != 0){
            CodeEntryActionWithDelay& previous = path_with_delays[c - 1];
            if (is_move(previous.action)){
                previous.delay = delays.hold;
            }
        }

        path_with_delays[c].action = action;
        path_with_delays[c].delay = delay;
    }


    //  Pad out the stalls.
    Milliseconds A_to_A_delay = std::max(delays.press_delay, delays.hold + delays.cool);
    Milliseconds last_A_time = -1000ms;
    Milliseconds current_time = 0ms;
    for (size_t c = 1; c < path_with_delays.size(); c++){
        CodeEntryActionWithDelay& previous = path_with_delays[c - 1];
        CodeEntryActionWithDelay& current = path_with_delays[c];

        //  The Switch doesn't like overlapping the scroll with the A press.
        //  So we treat the left-scroll like an A press for timing purposes.

        if (current.action == CodeEntryAction::ENTER_CHAR ||
            (switch2 && current.action == CodeEntryAction::SCROLL_LEFT)
        ){
            Milliseconds time_since_last_A = current_time - last_A_time;
            if (time_since_last_A < A_to_A_delay){
                Milliseconds stall = A_to_A_delay - time_since_last_A;
                previous.delay += stall;
                current_time += stall;
            }
            last_A_time = current_time;
        }

        current_time += current.delay;
    }


    //  Optimize

    if (!optimize || path_with_delays.empty() || switch2){
        return;
    }

    //  These only work on the Switch 1.
    for (size_t c = 1; c < path_with_delays.size(); c++){
        //  Zero the delay for any L press.
        CodeEntryActionWithDelay& current = path_with_delays[c];
        if (current.action == CodeEntryAction::SCROLL_LEFT){
            current.delay = 0ms;
            continue;
        }

        //  Zero the delay for any scroll immediately preceding an A press.
        CodeEntryActionWithDelay& previous = path_with_delays[c - 1];
        if (current.action == CodeEntryAction::ENTER_CHAR &&
            previous.action != CodeEntryAction::ENTER_CHAR &&
            previous.action != CodeEntryAction::SCROLL_LEFT
        ){
            previous.delay = 0ms;
        }
    }
}




void codeboard_execute_path(
    ProControllerContext& context,
    const CodeEntryDelays& delays,
    const std::vector<CodeEntryActionWithDelay>& path
){
    for (const CodeEntryActionWithDelay& action : path){
        switch (action.action){
        case CodeEntryAction::ENTER_CHAR:
            ssf_press_button(context, BUTTON_A, action.delay, delays.hold, delays.cool);
            break;
        case CodeEntryAction::SCROLL_LEFT:
            ssf_press_button(context, BUTTON_L, action.delay, delays.hold, delays.cool);
            break;
        default:
            ssf_issue_scroll(
                context,
                (DpadPosition)((uint8_t)action.action & 0x7f),
                action.delay, delays.hold, delays.cool
            );
            break;
        }
    }
}






}
}
}
