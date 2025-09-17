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
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "NintendoSwitch_CodeEntryTools.h"
#include "NintendoSwitch_NumberCodeEntry.h"
#include "NintendoSwitch_KeyboardCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{




void numberpad_enter_code(
    ConsoleHandle& console, AbstractControllerContext& context,
    const std::string& code,
    bool include_plus
){
    auto* keyboard = dynamic_cast<StandardHid::Keyboard*>(&context.controller());
    if (keyboard){
        StandardHid::KeyboardContext subcontext(context);
        keyboard_enter_code(console, subcontext, KeyboardLayout::QWERTY, code, include_plus);
        return;
    }

    auto* procon = dynamic_cast<ProController*>(&context.controller());
    if (procon){
        ProControllerContext subcontext(context);
        numberpad_enter_code(console, subcontext, code, include_plus);
        return;
    }

    throw UserSetupError(
        console, "Unsupported controller type."
    );
}




#if 0
void numberpad_enter_code(
    ConsoleHandle& console, StandardHid::KeyboardContext& context,
    const std::string& code,
    bool include_plus
){
    using namespace StandardHid;

    Milliseconds delay = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.TIME_UNIT;
    Milliseconds hold = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.HOLD;
    Milliseconds cool = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.COOLDOWN;

    static const std::map<char, KeyboardKey> MAP{
        {0, KeyboardKey::KEY_KP_0},
        {1, KeyboardKey::KEY_KP_1},
        {2, KeyboardKey::KEY_KP_2},
        {3, KeyboardKey::KEY_KP_3},
        {4, KeyboardKey::KEY_KP_4},
        {5, KeyboardKey::KEY_KP_5},
        {6, KeyboardKey::KEY_KP_6},
        {7, KeyboardKey::KEY_KP_7},
        {8, KeyboardKey::KEY_KP_8},
        {9, KeyboardKey::KEY_KP_9},
        {'0', KeyboardKey::KEY_KP_0},
        {'1', KeyboardKey::KEY_KP_1},
        {'2', KeyboardKey::KEY_KP_2},
        {'3', KeyboardKey::KEY_KP_3},
        {'4', KeyboardKey::KEY_KP_4},
        {'5', KeyboardKey::KEY_KP_5},
        {'6', KeyboardKey::KEY_KP_6},
        {'7', KeyboardKey::KEY_KP_7},
        {'8', KeyboardKey::KEY_KP_8},
        {'9', KeyboardKey::KEY_KP_9},
    };

    for (char ch : code){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            throw_and_log<OperationFailedException>(
                console, ErrorReport::NO_ERROR_REPORT,
                "Invalid code character."
            );
        }
        context->issue_key(&context, delay, hold, cool, iter->second);
    }

    if (include_plus){
        context->issue_key(&context, delay, hold, cool, KeyboardKey::KEY_ENTER);
        context->issue_key(&context, delay, hold, cool, KeyboardKey::KEY_ENTER);
        context->issue_key(&context, delay, hold, cool, KeyboardKey::KEY_ENTER);
    }
}
#endif



















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





//  Return the path from "source" to "destination".
std::vector<CodeEntryAction> numberpad_get_path(
    NumberEntryPosition source,
    NumberEntryPosition destination
){
    std::vector<CodeEntryAction> path;

    uint8_t row = source.row;
    uint8_t col = source.col;

    //  Do vertical first since it may auto-center the horizontal position.
    while (row < destination.row){
        path.emplace_back(CodeEntryAction::NORM_MOVE_DOWN);
        row++;
    }
    while (row > destination.row){
        path.emplace_back(CodeEntryAction::NORM_MOVE_UP);
        row--;
    }

    //  Moving to the zero automatically changes the column.
    if (row == 3){
        col = 1;
    }

    while (col < destination.col){
        path.emplace_back(CodeEntryAction::NORM_MOVE_RIGHT);
        col++;
    }
    while (col > destination.col){
        path.emplace_back(CodeEntryAction::NORM_MOVE_LEFT);
        col--;
    }

    path.emplace_back(CodeEntryAction::ENTER_CHAR);

    return path;
}


//  Get all possible paths from "start" to cover everything [positions, positions + length).
std::vector<std::vector<CodeEntryAction>> numberpad_get_all_paths(
    NumberEntryPosition start,
    const NumberEntryPosition* positions, size_t length,
    bool reordering
){
    if (length == 1){
        return {numberpad_get_path(start, positions[0])};
    }

    std::vector<std::vector<CodeEntryAction>> paths;
    {
        NumberEntryPosition position = positions[0];
        std::vector<CodeEntryAction> current = numberpad_get_path(start, position);
        std::vector<std::vector<CodeEntryAction>> subpaths = numberpad_get_all_paths(
            position,
            positions + 1, length - 1,
            reordering
        );
        for (std::vector<CodeEntryAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }
    if (reordering){
        NumberEntryPosition position = positions[length - 1];
        std::vector<CodeEntryAction> current = numberpad_get_path(start, position);
        current.emplace_back(CodeEntryAction::SCROLL_LEFT);
        std::vector<std::vector<CodeEntryAction>> subpaths = numberpad_get_all_paths(
            position,
            positions, length - 1,
            reordering
        );
        for (std::vector<CodeEntryAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }

    return paths;
}



//  Given a set of paths, find the best one and return it with fully populated
//  delays.
std::vector<CodeEntryActionWithDelay> numberpad_get_best_path(
    bool switch2,
    const std::vector<std::vector<CodeEntryAction>>& paths,
    const CodeEntryDelays& delays,
    bool optimize
){
    std::vector<CodeEntryActionWithDelay> best_path;
    Milliseconds best_time = Milliseconds::max();
    for (const std::vector<CodeEntryAction>& path : paths){
        std::vector<CodeEntryActionWithDelay> current_path;
        codeboard_populate_delays(switch2, current_path, path, delays, optimize);
        Milliseconds current_time = 0ms;
        for (CodeEntryActionWithDelay& action : current_path){
            current_time += action.delay;
            if (action.action == CodeEntryAction::SCROLL_LEFT){
//                action.delay =
            }
        }
        if (best_time > current_time ||
            (best_time == current_time && best_path.size() > current_path.size())
        ){
            best_time = current_time;
            best_path = std::move(current_path);
        }
    }
    return best_path;
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


    ConsoleType console_type = detect_console_type_from_in_game(console, context);
    bool switch2;
    if (is_switch1(console_type)){
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
        unit        = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY0.TIME_UNIT;
        hold        = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY0.HOLD;
        cool        = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY0.COOLDOWN;
        reordering  = ConsoleSettings::instance().SWITCH2_DIGIT_ENTRY0.REORDERING;
    }else{
        unit        = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY0.TIME_UNIT;
        hold        = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY0.HOLD;
        cool        = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY0.COOLDOWN;
        reordering  = ConsoleSettings::instance().SWITCH1_DIGIT_ENTRY0.REORDERING;
    }

    Milliseconds tv = context->timing_variation();
    unit += tv;

    CodeEntryDelays delays{
        .hold = hold + tv,
        .cool = cool,
        .press_delay = unit,
        .move_delay = unit,
        .scroll_delay = unit,
        .wrap_delay = 2*unit,
    };


    //  Get all the possible paths.
    std::vector<std::vector<CodeEntryAction>> all_paths = numberpad_get_all_paths(
        {0, 0},
        positions.data(), positions.size(),
        reordering
    );

    //  Pick the best path.
    std::vector<CodeEntryActionWithDelay> best_path = numberpad_get_best_path(
        switch2,
        all_paths,
        delays,
        !switch2 && context->atomic_multibutton()
    );

    codeboard_execute_path(context, delays, best_path);

    if (include_plus){
        ssf_press_button_ptv(context, BUTTON_PLUS);
        ssf_press_button_ptv(context, BUTTON_PLUS);
    }

    ssf_flush_pipeline(context);
}







}
}
}
