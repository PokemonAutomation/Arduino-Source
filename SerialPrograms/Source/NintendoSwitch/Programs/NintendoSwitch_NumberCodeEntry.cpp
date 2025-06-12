/*  Number Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.h"
#include "NintendoSwitch_NumberCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


struct NumberEntryPosition{
    uint8_t row;
    uint8_t col;
};

static const std::map<char, NumberEntryPosition>& NUMBER_POSITIONS(){
    static const std::map<char, NumberEntryPosition> map{
        {1, {0, 0}},
        {2, {0, 1}},
        {3, {0, 2}},
        {4, {1, 0}},
        {5, {1, 1}},
        {6, {1, 2}},
        {7, {2, 0}},
        {8, {2, 1}},
        {9, {2, 2}},
        {0, {3, 1}},

        {'1', {0, 0}},
        {'2', {0, 1}},
        {'3', {0, 2}},
        {'4', {1, 0}},
        {'5', {1, 1}},
        {'6', {1, 2}},
        {'7', {2, 0}},
        {'8', {2, 1}},
        {'9', {2, 2}},
        {'0', {3, 1}},
    };
    return map;
}


enum class NumberEntryAction : uint8_t{
    ENTER_CHAR  = 0xf0,
    SCROLL_LEFT = 0xf1,
    MOVE_UP    =    (uint8_t)DpadPosition::DPAD_UP,
    MOVE_RIGHT =    (uint8_t)DpadPosition::DPAD_RIGHT,
    MOVE_DOWN  =    (uint8_t)DpadPosition::DPAD_DOWN,
    MOVE_LEFT  =    (uint8_t)DpadPosition::DPAD_LEFT,
};
bool is_button_press(NumberEntryAction action){
    switch (action){
    case NumberEntryAction::ENTER_CHAR:
    case NumberEntryAction::SCROLL_LEFT:
        return true;
    default:
        return false;
    }
}

struct NumberEntryActionWithDelay{
    NumberEntryAction action;
    Milliseconds delay;
};

struct NumberEntryDelays{
    Milliseconds hold;
    Milliseconds cool;
    Milliseconds press_delay;
    Milliseconds move_delay;
};



//  Return the path from "source" to "destination".
std::vector<NumberEntryAction> numberpad_get_path(
    NumberEntryPosition source,
    NumberEntryPosition destination
){
    std::vector<NumberEntryAction> path;

    uint8_t row = source.row;
    uint8_t col = source.col;

    //  Do vertical first since it may auto-center the horizontal position.
    while (row < destination.row){
        path.emplace_back(NumberEntryAction::MOVE_DOWN);
        row++;
    }
    while (row > destination.row){
        path.emplace_back(NumberEntryAction::MOVE_UP);
        row--;
    }

    //  Moving to the zero automatically changes the column.
    if (row == 3){
        col = 1;
    }

    while (col < destination.col){
        path.emplace_back(NumberEntryAction::MOVE_RIGHT);
        col++;
    }
    while (col > destination.col){
        path.emplace_back(NumberEntryAction::MOVE_LEFT);
        col--;
    }

    path.emplace_back(NumberEntryAction::ENTER_CHAR);

    return path;
}


//  Get all possible paths from "start" to cover everything [positions, positions + length).
std::vector<std::vector<NumberEntryAction>> numberpad_get_all_paths(
    NumberEntryPosition start,
    const NumberEntryPosition* positions, size_t length,
    bool reordering
){
    if (length == 1){
        return {numberpad_get_path(start, positions[0])};
    }

    std::vector<std::vector<NumberEntryAction>> paths;
    {
        NumberEntryPosition position = positions[0];
        std::vector<NumberEntryAction> current = numberpad_get_path(start, position);
        std::vector<std::vector<NumberEntryAction>> subpaths = numberpad_get_all_paths(
            position,
            positions + 1, length - 1,
            reordering
        );
        for (std::vector<NumberEntryAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }
    if (reordering){
        NumberEntryPosition position = positions[length - 1];
        std::vector<NumberEntryAction> current = numberpad_get_path(start, position);
        current.emplace_back(NumberEntryAction::SCROLL_LEFT);
        std::vector<std::vector<NumberEntryAction>> subpaths = numberpad_get_all_paths(
            position,
            positions, length - 1,
            reordering
        );
        for (std::vector<NumberEntryAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }

    return paths;
}


//  Given a path, optimize it and fully populate the delays.
void numberpad_populate_delays(
    std::vector<NumberEntryActionWithDelay>& path_with_delays,
    const std::vector<NumberEntryAction>& path,
    const NumberEntryDelays& delays,
    bool optimize
){
    //  Populate and assign default delays first.
    path_with_delays.resize(path.size());
    for (size_t c = 0; c < path_with_delays.size(); c++){
        NumberEntryAction action = path[c];
        Milliseconds delay = is_button_press(action)
            ? delays.press_delay
            : delays.move_delay;

        path_with_delays[c].action = action;
        path_with_delays[c].delay = delay;
    }

    //  Optimize

    if (!optimize || path_with_delays.empty()){
        return;
    }

    //  These only work on the Switch 1.
    for (size_t c = 1; c < path_with_delays.size(); c++){
        //  Zero the delay for any L press.
        NumberEntryActionWithDelay& current = path_with_delays[c];
        if (current.action == NumberEntryAction::SCROLL_LEFT){
            current.delay = 0ms;
            continue;
        }

        //  Zero the delay for any scroll immediately preceding an A press.
        NumberEntryActionWithDelay& previous = path_with_delays[c - 1];
        if (current.action == NumberEntryAction::ENTER_CHAR &&
            previous.action != NumberEntryAction::ENTER_CHAR &&
            previous.action != NumberEntryAction::SCROLL_LEFT
        ){
            previous.delay = 0ms;
        }
    }
}


//  Given a set of paths, find the best one and return it with fully populated
//  delays.
std::vector<NumberEntryActionWithDelay> keyboard_get_best_path(
    const std::vector<std::vector<NumberEntryAction>>& paths,
    const NumberEntryDelays& delays,
    bool optimize
){
    std::vector<NumberEntryActionWithDelay> best_path;
    Milliseconds best_time = Milliseconds::max();
    for (const std::vector<NumberEntryAction>& path : paths){
        std::vector<NumberEntryActionWithDelay> current_path;
        numberpad_populate_delays(current_path, path, delays, optimize);
        Milliseconds current_time = 0ms;
        for (NumberEntryActionWithDelay& action : current_path){
            current_time += action.delay;
        }
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
    const NumberEntryDelays& delays,
    const std::vector<NumberEntryActionWithDelay>& path
){
    for (const NumberEntryActionWithDelay& action : path){
        switch (action.action){
        case NumberEntryAction::ENTER_CHAR:
            ssf_press_button(context, BUTTON_A, action.delay, delays.hold, delays.cool);
            break;
        case NumberEntryAction::SCROLL_LEFT:
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



void numberpad_enter_code(
    ConsoleHandle& console, ProControllerContext& context,
    const std::string& code,
    bool include_plus
){
    //  Calculate the coordinates.
    const std::map<char, NumberEntryPosition>& POSITION_MAP = NUMBER_POSITIONS();
    std::vector<NumberEntryPosition> positions;
    for (char ch : code){
        auto iter = POSITION_MAP.find(ch);
        if (iter == POSITION_MAP.end()){
            throw_and_log<OperationFailedException>(
                console, ErrorReport::NO_ERROR_REPORT,
                "Invalid code character."
            );
        }
        positions.emplace_back(iter->second);
    }


    ConsoleType console_type = console.state().console_type();
    bool switch2;
    if (console_type == ConsoleType::Switch1){
        switch2 = false;
    }else if (is_switch2(console_type)){
        switch2 = true;
    }else{
        throw UserSetupError(
            console,
            "Please select a valid Switch console type."
        );
    }


    //  Fetch the delays.

    Milliseconds unit;
    Milliseconds hold;
    Milliseconds cool;
    bool reordering;
    if (switch2){
        unit        = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY.TIME_UNIT;
        hold        = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY.HOLD;
        cool        = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY.COOLDOWN;
        reordering  = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY.REORDERING;
    }else{
        unit        = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY.TIME_UNIT;
        hold        = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY.HOLD;
        cool        = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY.COOLDOWN;
        reordering  = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY.REORDERING;
    }

    Milliseconds tv = context->timing_variation();
    unit += tv;

    NumberEntryDelays delays{
        .hold = hold,
        .cool = cool,
        .press_delay = unit,
        .move_delay = unit,
    };


    //  Get all the possible paths.
    std::vector<std::vector<NumberEntryAction>> all_paths = numberpad_get_all_paths(
        {0, 0},
        positions.data(), positions.size(),
        reordering
    );

    //  Pick the best path.
    std::vector<NumberEntryActionWithDelay> best_path = keyboard_get_best_path(
        all_paths,
        delays,
        !switch2 && context->atomic_multibutton()
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
