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
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "NintendoSwitch_CodeEntryTools.h"
#include "NintendoSwitch_KeyboardEntryMappings.h"
#include "NintendoSwitch_KeyboardCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{




void keyboard_enter_code(
    ConsoleHandle& console, AbstractControllerContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool include_plus
){
    auto* keyboard = context->cast<StandardHid::Keyboard>();
    if (keyboard){
        StandardHid::KeyboardContext subcontext(context);
        keyboard_enter_code(console, subcontext, keyboard_layout, code, include_plus);
        return;
    }

    auto* procon = context->cast<ProController>();
    if (procon){
        ProControllerContext subcontext(context);
        keyboard_enter_code(console, subcontext, keyboard_layout, code, include_plus);
        return;
    }

    throw UserSetupError(
        console, "Unsupported controller type."
    );
}





void keyboard_enter_code(
    ConsoleHandle& console, StandardHid::KeyboardContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool include_plus
){
    using namespace StandardHid;

    Milliseconds delay = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.TIME_UNIT;
    Milliseconds hold = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.HOLD;
    Milliseconds cool = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.COOLDOWN;

    Milliseconds parallel_delay = ConsoleSettings::instance().KEYBOARD_CONTROLLER_TIMINGS.PARLLELIZE
        ? 0ms
        : delay;

    const std::map<char, KeyboardKey>& MAP = KEYBOARD_MAPPINGS(keyboard_layout);

    std::vector<KeyboardKey> HID_ids;
    for (char ch : code){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            throw_and_log<OperationFailedException>(
                console, ErrorReport::NO_ERROR_REPORT,
                "Invalid code character."
            );
        }
        HID_ids.emplace_back(iter->second);
//        context->issue_key(&context, delay, hold, cool, iter->second);
    }

    for (size_t c = 1; c < HID_ids.size(); c++){
        KeyboardKey curr = HID_ids[c - 1];
        KeyboardKey next = HID_ids[c];
        context->issue_key(&context, curr < next ? parallel_delay : delay, hold, cool, curr);
    }
    if (!HID_ids.empty()){
        context->issue_key(&context, parallel_delay, hold, cool, HID_ids.back());
    }

    if (include_plus){
        context->issue_key(&context, delay, hold, cool, KeyboardKey::KEY_ENTER);
        context->issue_key(&context, delay, hold, cool, KeyboardKey::KEY_ENTER);
        context->issue_key(&context, delay, hold, cool, KeyboardKey::KEY_ENTER);
    }
}












//  Return the path from "source" to "destination".
std::vector<CodeEntryAction> keyboard_get_path(
    KeyboardEntryPosition source,
    KeyboardEntryPosition destination
){
    std::vector<CodeEntryAction> path;

    //  Vertical is easy since there's no wrapping and no hazards.
    if (source.row < destination.row){
        size_t diff = destination.row - source.row;
        for (size_t c = 0; c < diff; c++){
            path.emplace_back(CodeEntryAction::NORM_MOVE_DOWN);
        }
    }else{
        size_t diff = source.row - destination.row;
        for (size_t c = 0; c < diff; c++){
            path.emplace_back(CodeEntryAction::NORM_MOVE_UP);
        }
    }

    //  Horizontal is messy because we need extra delay on wrapping.
    uint8_t col = source.col;
    while (true){
        if (col == destination.col){
            break;
        }

        CodeEntryAction action;
        if (destination.col > col){
            if (destination.col - col <= 6){
                action = CodeEntryAction::NORM_MOVE_RIGHT;
                col++;
            }else{
                action = CodeEntryAction::NORM_MOVE_LEFT;
                col--;
            }
        }else{
            if (col - destination.col <= 6){
                action = CodeEntryAction::NORM_MOVE_LEFT;
                col--;
            }else{
                action = CodeEntryAction::NORM_MOVE_RIGHT;
                col++;
            }
        }

        //  Wrap around the sides.
        if (col == (uint8_t)-1){
            col = 11;
            action = (CodeEntryAction)((uint8_t)action | 0x80);
        }
        if (col == 12){
            col = 0;
            action = (CodeEntryAction)((uint8_t)action | 0x80);
        }

        path.emplace_back(action);
    }

    path.emplace_back(CodeEntryAction::ENTER_CHAR);

    return path;
}


//  Get all possible paths from "start" to cover everything [positions, positions + length).
std::vector<std::vector<CodeEntryAction>> keyboard_get_all_paths(
    KeyboardEntryPosition start,
    const KeyboardEntryPosition* positions, size_t length,
    bool reordering
){
    if (length == 1){
        return {keyboard_get_path(start, positions[0])};
    }

    std::vector<std::vector<CodeEntryAction>> paths;
    {
        KeyboardEntryPosition position = positions[0];
        std::vector<CodeEntryAction> current = keyboard_get_path(start, position);
        std::vector<std::vector<CodeEntryAction>> subpaths = keyboard_get_all_paths(
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
        KeyboardEntryPosition position = positions[length - 1];
        std::vector<CodeEntryAction> current = keyboard_get_path(start, position);
        current.emplace_back(CodeEntryAction::SCROLL_LEFT);
        std::vector<std::vector<CodeEntryAction>> subpaths = keyboard_get_all_paths(
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
std::vector<CodeEntryActionWithDelay> keyboard_get_best_path(
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




void keyboard_enter_code(
    ConsoleHandle& console, ProControllerContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool include_plus
){
    //  Calculate the coordinates.
    const std::map<char, KeyboardEntryPosition>& POSITION_MAP = KEYBOARD_POSITIONS(keyboard_layout);
    std::vector<KeyboardEntryPosition> positions;
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


    //  Compute the delays.

    Milliseconds unit;
    Milliseconds hold;
    Milliseconds cool;
    bool reordering;
    if (switch2){
        unit        = ConsoleSettings::instance().SWITCH2_KEYBOARD_ENTRY0.TIME_UNIT;
        hold        = ConsoleSettings::instance().SWITCH2_KEYBOARD_ENTRY0.HOLD;
        cool        = ConsoleSettings::instance().SWITCH2_KEYBOARD_ENTRY0.COOLDOWN;
        reordering  = ConsoleSettings::instance().SWITCH2_KEYBOARD_ENTRY0.REORDERING;
    }else{
        unit        = ConsoleSettings::instance().SWITCH1_KEYBOARD_ENTRY0.TIME_UNIT;
        hold        = ConsoleSettings::instance().SWITCH1_KEYBOARD_ENTRY0.HOLD;
        cool        = ConsoleSettings::instance().SWITCH1_KEYBOARD_ENTRY0.COOLDOWN;
        reordering  = ConsoleSettings::instance().SWITCH1_KEYBOARD_ENTRY0.REORDERING;
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
    std::vector<std::vector<CodeEntryAction>> all_paths = keyboard_get_all_paths(
        {0, 0},
        positions.data(), positions.size(),
        reordering
    );

    //  Pick the best path.
    std::vector<CodeEntryActionWithDelay> best_path = keyboard_get_best_path(
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
