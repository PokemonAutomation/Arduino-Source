/*  Code Entry Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_CodeEntryTools.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{



Milliseconds calculate_path_time(
    const CodeEntryDelays& delays,
    const std::vector<CodeEntryActionWithDelay>& path
){
    Milliseconds full_cooldown = delays.hold + delays.cool;

    Milliseconds ready_A = 0ms;
    Milliseconds ready_move = 0ms;
    Milliseconds ready_scroll = 0ms;

    Milliseconds current = 0ms;

    for (const CodeEntryActionWithDelay& action : path){
        switch (action.action){
        case CodeEntryAction::ENTER_CHAR:
            current = std::max(current, ready_A);
            ready_A = current + full_cooldown;
            current += action.delay;
            break;
        case CodeEntryAction::SCROLL_LEFT:
            current = std::max(current, ready_move);
            ready_move = current + full_cooldown;
            current += action.delay;
            break;
        default:
            current = std::max(current, ready_scroll);
            ready_scroll = current + full_cooldown;
            current += action.delay;
            break;
        }
    }

    return current;
}


Milliseconds codeboard_populate_delays(
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

//        cout << "    " << (int)action << " : " << delay.count() << endl;
    }


    //  Optimize

    if (!optimize || path_with_delays.empty() || switch2){
        return calculate_path_time(delays, path_with_delays);
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

    return calculate_path_time(delays, path_with_delays);
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
