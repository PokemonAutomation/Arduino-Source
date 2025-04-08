/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include <map>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.h"
#include "NintendoSwitch_KeyboardCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


struct KeyboardEntryPosition{
    uint8_t row;
    uint8_t col;
};

static const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS_QWERTY(){
    static const std::map<char, KeyboardEntryPosition> map{
        {'1', {0, 0}},
        {'2', {0, 1}},
        {'3', {0, 2}},
        {'4', {0, 3}},
        {'5', {0, 4}},
        {'6', {0, 5}},
        {'7', {0, 6}},
        {'8', {0, 7}},
        {'9', {0, 8}},
        {'0', {0, 9}},

        {'Q', {1, 0}},
        {'W', {1, 1}},
        {'E', {1, 2}},
        {'R', {1, 3}},
        {'T', {1, 4}},
        {'Y', {1, 5}},
        {'U', {1, 6}},
        {'P', {1, 9}},

        {'A', {2, 0}},
        {'S', {2, 1}},
        {'D', {2, 2}},
        {'F', {2, 3}},
        {'G', {2, 4}},
        {'H', {2, 5}},
        {'J', {2, 6}},
        {'K', {2, 7}},
        {'L', {2, 8}},

        {'X', {3, 1}},
        {'C', {3, 2}},
        {'V', {3, 3}},
        {'B', {3, 4}},
        {'N', {3, 5}},
        {'M', {3, 6}},
    };
    return map;
}
static const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS_AZERTY(){
    static const std::map<char, KeyboardEntryPosition> map{
        {'1', {0, 0}},
        {'2', {0, 1}},
        {'3', {0, 2}},
        {'4', {0, 3}},
        {'5', {0, 4}},
        {'6', {0, 5}},
        {'7', {0, 6}},
        {'8', {0, 7}},
        {'9', {0, 8}},
        {'0', {0, 9}},

        {'A', {1, 0}},
        {'E', {1, 2}},
        {'R', {1, 3}},
        {'T', {1, 4}},
        {'Y', {1, 5}},
        {'U', {1, 6}},
        {'P', {1, 9}},

        {'Q', {2, 0}},
        {'S', {2, 1}},
        {'D', {2, 2}},
        {'F', {2, 3}},
        {'G', {2, 4}},
        {'H', {2, 5}},
        {'J', {2, 6}},
        {'K', {2, 7}},
        {'L', {2, 8}},
        {'M', {2, 9}},

        {'W', {3, 0}},
        {'X', {3, 1}},
        {'C', {3, 2}},
        {'V', {3, 3}},
        {'B', {3, 4}},
        {'N', {3, 5}},
    };
    return map;
}




enum class KeyboardEntryAction : uint8_t{
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

struct KeyboardEntryActionWithDelay{
    KeyboardEntryAction action;
    Milliseconds delay;
};

struct KeyboardEntryDelays{
    Milliseconds hold;
    Milliseconds cool;
    Milliseconds press_delay;
    Milliseconds move_delay;
    Milliseconds wrap_delay;
};





//  Return the path from "source" to "destination".
std::vector<KeyboardEntryAction> keyboard_get_path(
    KeyboardEntryPosition source,
    KeyboardEntryPosition destination
){
    std::vector<KeyboardEntryAction> path;

    //  Vertical is easy since there's no wrapping and no hazards.
    if (source.row < destination.row){
        size_t diff = destination.row - source.row;
        for (size_t c = 0; c < diff; c++){
            path.emplace_back(KeyboardEntryAction::NORM_MOVE_DOWN);
        }
    }else{
        size_t diff = source.row - destination.row;
        for (size_t c = 0; c < diff; c++){
            path.emplace_back(KeyboardEntryAction::NORM_MOVE_UP);
        }
    }

    //  Horizontal is messy because we need extra delay on wrapping.
    uint8_t col = source.col;
    while (true){
        if (col == destination.col){
            break;
        }

        KeyboardEntryAction action;
        if (destination.col > col){
            if (destination.col - col <= 6){
                action = KeyboardEntryAction::NORM_MOVE_RIGHT;
                col++;
            }else{
                action = KeyboardEntryAction::NORM_MOVE_LEFT;
                col--;
            }
        }else{
            if (col - destination.col <= 6){
                action = KeyboardEntryAction::NORM_MOVE_LEFT;
                col--;
            }else{
                action = KeyboardEntryAction::NORM_MOVE_RIGHT;
                col++;
            }
        }

        //  Wrap around the sides.
        if (col == (uint8_t)-1){
            col = 11;
            action = (KeyboardEntryAction)((uint8_t)action | 0x80);
        }
        if (col == 12){
            col = 0;
            action = (KeyboardEntryAction)((uint8_t)action | 0x80);
        }

        path.emplace_back(action);
    }

    path.emplace_back(KeyboardEntryAction::ENTER_CHAR);

    return path;
}


//  Get all possible paths from "start" to cover everything [positions, positions + length).
std::vector<std::vector<KeyboardEntryAction>> keyboard_get_all_paths(
    KeyboardEntryPosition start,
    const KeyboardEntryPosition* positions, size_t length,
    bool reordering
){
    if (length == 1){
        return {keyboard_get_path(start, positions[0])};
    }

    std::vector<std::vector<KeyboardEntryAction>> paths;
    {
        KeyboardEntryPosition position = positions[0];
        std::vector<KeyboardEntryAction> current = keyboard_get_path(start, position);
        std::vector<std::vector<KeyboardEntryAction>> subpaths = keyboard_get_all_paths(
            position,
            positions + 1, length - 1,
            reordering
        );
        for (std::vector<KeyboardEntryAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }
    if (reordering){
        KeyboardEntryPosition position = positions[length - 1];
        std::vector<KeyboardEntryAction> current = keyboard_get_path(start, position);
        current.emplace_back(KeyboardEntryAction::SCROLL_LEFT);
        std::vector<std::vector<KeyboardEntryAction>> subpaths = keyboard_get_all_paths(
            position,
            positions, length - 1,
            reordering
        );
        for (std::vector<KeyboardEntryAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }

    return paths;
}


//  Given a path, optimize it and fully populate the delays.
Milliseconds keyboard_populate_delays(
    std::vector<KeyboardEntryActionWithDelay>& path_with_delays,
    const std::vector<KeyboardEntryAction>& path,
    const KeyboardEntryDelays& delays,
    bool optimize
){
    //  Populate and assign default delays first.
    path_with_delays.resize(path.size());
    for (size_t c = 0; c < path_with_delays.size(); c++){
        KeyboardEntryAction action = path[c];
        Milliseconds delay = (uint8_t)action >= (uint8_t)KeyboardEntryAction::ENTER_CHAR
            ? delays.press_delay
            : (uint8_t)action & 0x80
                ? delays.wrap_delay
                : delays.move_delay;

        path_with_delays[c].action = action;
        path_with_delays[c].delay = delay;
    }

    //  Optimize
    if (optimize){
        for (size_t c = 1; c < path_with_delays.size(); c++){
            //  Zero the delay for any L press.
            KeyboardEntryActionWithDelay& current = path_with_delays[c];
            if (current.action == KeyboardEntryAction::SCROLL_LEFT){
                current.delay = 0ms;
                continue;
            }

            //  Zero the delay for any scroll immediately preceding an A press.
            KeyboardEntryActionWithDelay& previous = path_with_delays[c - 1];
            if (current.action == KeyboardEntryAction::ENTER_CHAR &&
                previous.action != KeyboardEntryAction::ENTER_CHAR &&
                previous.action != KeyboardEntryAction::SCROLL_LEFT
            ){
                previous.delay = 0ms;
            }
        }
    }

    Milliseconds total = 0ms;
    for (KeyboardEntryActionWithDelay& action : path_with_delays){
        total += action.delay;
    }

    return total;
}


//  Given a set of paths, find the best one and return it with fully populated
//  delays.
std::vector<KeyboardEntryActionWithDelay> keyboard_get_best_path(
    const std::vector<std::vector<KeyboardEntryAction>>& paths,
    const KeyboardEntryDelays& delays,
    bool optimize
){
    std::vector<KeyboardEntryActionWithDelay> best_path;
    Milliseconds best_time = Milliseconds::max();
    for (const std::vector<KeyboardEntryAction>& path : paths){
        std::vector<KeyboardEntryActionWithDelay> current_path;
        Milliseconds current_time = keyboard_populate_delays(current_path, path, delays, optimize);
        if (best_time > current_time){
            best_time = current_time;
            best_path = std::move(current_path);
        }
    }
    return best_path;
}


//  Actually execute a path and enter it into the Switch.
void keyboard_execute_path(
    ProControllerContext& context,
    const KeyboardEntryDelays& delays,
    const std::vector<KeyboardEntryActionWithDelay>& path
){
    for (const KeyboardEntryActionWithDelay& action : path){
        switch (action.action){
        case KeyboardEntryAction::ENTER_CHAR:
            ssf_press_button(context, BUTTON_A, action.delay, delays.hold, delays.cool);
            break;
        case KeyboardEntryAction::SCROLL_LEFT:
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



void keyboard_enter_code(
    Logger& logger, ProControllerContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool include_plus
){
    //  Calculate the coordinates.
    auto get_keyboard_layout = [](KeyboardLayout keyboard_layout){
        switch (keyboard_layout){
        case KeyboardLayout::QWERTY:
            return KEYBOARD_POSITIONS_QWERTY();
        case KeyboardLayout::AZERTY:
            return KEYBOARD_POSITIONS_AZERTY();
        default:
            return KEYBOARD_POSITIONS_QWERTY();
        }
    };

    const std::map<char, KeyboardEntryPosition>& POSITION_MAP = get_keyboard_layout(keyboard_layout);
    std::vector<KeyboardEntryPosition> positions;
    for (char ch : code){
        auto iter = POSITION_MAP.find(ch);
        if (iter == POSITION_MAP.end()){
            throw_and_log<OperationFailedException>(
                logger, ErrorReport::NO_ERROR_REPORT,
                "Invalid code character."
            );
        }
        positions.emplace_back(iter->second);
    }


    //  Compute the delays.

    Milliseconds unit = ConsoleSettings::instance().KEYBOARD_ENTRY.TIME_UNIT;
    Milliseconds tv = context->timing_variation();
    unit += tv;

    KeyboardEntryDelays delays{
        .hold = 2*unit,
        .cool = unit,
        .press_delay = unit,
        .move_delay = unit,
        .wrap_delay = 2*unit,
    };

    //  Get all the possible paths.
    std::vector<std::vector<KeyboardEntryAction>> all_paths = keyboard_get_all_paths(
        {0, 0},
        positions.data(), positions.size(),
        ConsoleSettings::instance().KEYBOARD_ENTRY.DIGIT_REORDERING
    );

    //  Pick the best path.
    std::vector<KeyboardEntryActionWithDelay> best_path = keyboard_get_best_path(
        all_paths,
        delays,
        context->atomic_multibutton()
    );

    keyboard_execute_path(context, delays, best_path);

    if (include_plus){
        ssf_press_button_ptv(context, BUTTON_PLUS);
        ssf_press_button_ptv(context, BUTTON_PLUS);
    }

    ssf_flush_pipeline(context);
}













}
}
