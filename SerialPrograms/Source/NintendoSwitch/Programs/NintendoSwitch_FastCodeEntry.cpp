/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <map>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_FastCodeEntry.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{





FastCodeEntrySettingsOption::FastCodeEntrySettingsOption(LockMode lock_while_program_is_running)
    : GroupOption(
        "Fast Code Entry",
        lock_while_program_is_running,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , KEYBOARD_LAYOUT(
        "<b>Keyboard Layout:</b>",
        {
            {KeyboardLayout::QWERTY, "qwerty", "QWERTY"},
            {KeyboardLayout::AZERTY, "azerty", "AZERTY"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        KeyboardLayout::QWERTY
    )
    , SKIP_PLUS(
        "<b>Skip the Plus:</b><br>Don't press + to finalize the code. Useful for testing.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options: (developer only)</b></font>"
    )
    , DIGIT_REORDERING(
        "<b>Digit Reordering:</b><br>Allow digits to be entered out of order.",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE
    )
    , SCROLL_DELAY0(
        "<b>Scroll Delay:</b><br>Delay to scroll between adjacent keys.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        PreloadSettings::instance().DEVELOPER_MODE ? "40 ms" : "48 ms"
    )
    , WRAP_DELAY0(
        "<b>Wrap Delay:</b><br>Delay to wrap between left/right edges.<br>"
        "<font color=\"red\">This timing is only relevant if the controller is tick-precise.</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        "48 ms"
    )
{
    PA_ADD_OPTION(KEYBOARD_LAYOUT);
    PA_ADD_OPTION(SKIP_PLUS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(m_advanced_options);
        PA_ADD_OPTION(DIGIT_REORDERING);
        PA_ADD_OPTION(SCROLL_DELAY0);
        PA_ADD_OPTION(WRAP_DELAY0);
    }
}



const std::map<char, CodeboardPosition>& CODEBOARD_POSITIONS_QWERTY(){
    static const std::map<char, CodeboardPosition> map{
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
const std::map<char, CodeboardPosition>& CODEBOARD_POSITIONS_AZERTY(){
    static const std::map<char, CodeboardPosition> map{
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



std::vector<CodeboardAction> codeboard_get_mode_path(
    CodeboardPosition source,
    CodeboardPosition destination
){
    std::vector<CodeboardAction> path;

    //  Vertical is easy since there's no wrapping and no hazards.
    if (source.row < destination.row){
        size_t diff = destination.row - source.row;
        for (size_t c = 0; c < diff; c++){
            path.emplace_back(CodeboardAction::NORM_MOVE_DOWN);
        }
    }else{
        size_t diff = source.row - destination.row;
        for (size_t c = 0; c < diff; c++){
            path.emplace_back(CodeboardAction::NORM_MOVE_UP);
        }
    }

    //  Horizontal is messy because we need extra delay on wrapping.
    uint8_t col = source.col;
    while (true){
        if (col == destination.col){
            break;
        }

        CodeboardAction action;
        if (destination.col > col){
            if (destination.col - col <= 6){
                action = CodeboardAction::NORM_MOVE_RIGHT;
                col++;
            }else{
                action = CodeboardAction::NORM_MOVE_LEFT;
                col--;
            }
        }else{
            if (col - destination.col <= 6){
                action = CodeboardAction::NORM_MOVE_LEFT;
                col--;
            }else{
                action = CodeboardAction::NORM_MOVE_RIGHT;
                col++;
            }
        }

        //  Wrap around the sides.
        if (col == (uint8_t)-1){
            col = 11;
            action = (CodeboardAction)((uint8_t)action | 0x80);
        }
        if (col == 12){
            col = 0;
            action = (CodeboardAction)((uint8_t)action | 0x80);
        }

        path.emplace_back(action);
    }

    path.emplace_back(CodeboardAction::ENTER_CHAR);

    return path;
}

std::vector<std::vector<CodeboardAction>> codeboard_get_all_paths(
    KeyboardLayout keyboard_layout,
    CodeboardPosition start,
    const CodeboardPosition* positions, size_t length,
    bool reordering
){
    if (length == 1){
        return {codeboard_get_mode_path(start, positions[0])};
    }

    std::vector<std::vector<CodeboardAction>> paths;
    {
        CodeboardPosition position = positions[0];
        std::vector<CodeboardAction> current = codeboard_get_mode_path(start, position);
        std::vector<std::vector<CodeboardAction>> subpaths = codeboard_get_all_paths(
            keyboard_layout,
            position,
            positions + 1, length - 1,
            reordering
        );
        for (std::vector<CodeboardAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }
    if (reordering){
        CodeboardPosition position = positions[length - 1];
        std::vector<CodeboardAction> current = codeboard_get_mode_path(start, position);
        current.emplace_back(CodeboardAction::SCROLL_LEFT);
        std::vector<std::vector<CodeboardAction>> subpaths = codeboard_get_all_paths(
            keyboard_layout,
            position,
            positions, length - 1,
            reordering
        );
        for (std::vector<CodeboardAction>& path : subpaths){
            path.insert(path.begin(), current.begin(), current.end());
            paths.emplace_back(std::move(path));
        }
    }

    return paths;
}

Milliseconds codeboard_populate_delays(
    std::vector<CodeboardActionWithDelay>& path_with_delays,
    const std::vector<CodeboardAction>& path,
    const CodeboardDelays& delays,
    bool optimize
){
    //  Populate and assign default delays first.
    path_with_delays.resize(path.size());
    for (size_t c = 0; c < path_with_delays.size(); c++){
        CodeboardAction action = path[c];
        Milliseconds delay = (uint8_t)action >= (uint8_t)CodeboardAction::ENTER_CHAR
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
            CodeboardActionWithDelay& current = path_with_delays[c];
            if (current.action == CodeboardAction::SCROLL_LEFT){
                current.delay = 0ms;
                continue;
            }

            //  Zero the delay for any scroll immediately preceding an A press.
            CodeboardActionWithDelay& previous = path_with_delays[c - 1];
            if (current.action == CodeboardAction::ENTER_CHAR &&
                previous.action != CodeboardAction::ENTER_CHAR &&
                previous.action != CodeboardAction::SCROLL_LEFT
            ){
                previous.delay = 0ms;
            }
        }
    }

    Milliseconds total = 0ms;
    for (CodeboardActionWithDelay& action : path_with_delays){
        total += action.delay;
    }

    return total;
}

std::vector<CodeboardActionWithDelay> codeboard_get_best_path(
    const std::vector<std::vector<CodeboardAction>>& paths,
    const CodeboardDelays& delays,
    bool optimize
){
    std::vector<CodeboardActionWithDelay> best_path;
    Milliseconds best_time = Milliseconds::max();
    for (const std::vector<CodeboardAction>& path : paths){
        std::vector<CodeboardActionWithDelay> current_path;
        Milliseconds current_time = codeboard_populate_delays(current_path, path, delays, optimize);
        if (best_time > current_time){
            best_time = current_time;
            best_path = std::move(current_path);
        }
    }
    return best_path;
}

void codeboard_execute_path(
    ProControllerContext& context,
    const CodeboardDelays& delays,
    const std::vector<CodeboardActionWithDelay>& path
){
    for (const CodeboardActionWithDelay& action : path){
        switch (action.action){
        case CodeboardAction::ENTER_CHAR:
            ssf_press_button(context, BUTTON_A, action.delay, delays.hold, delays.cool);
            break;
        case CodeboardAction::SCROLL_LEFT:
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

void codeboard_enter_digits(
    Logger& logger, ProControllerContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool reordering, const CodeboardDelays& delays
){
    //  Calculate the coordinates.
    auto get_keyboard_layout = [](KeyboardLayout keyboard_layout){
        switch (keyboard_layout){
        case KeyboardLayout::QWERTY:
            return CODEBOARD_POSITIONS_QWERTY();
        case KeyboardLayout::AZERTY:
            return CODEBOARD_POSITIONS_AZERTY();
        default:
            return CODEBOARD_POSITIONS_QWERTY();
        }
    };
    const std::map<char, CodeboardPosition>& POSITION_MAP = get_keyboard_layout(keyboard_layout);
    std::vector<CodeboardPosition> positions;
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

    //  Get all the possible paths.
    std::vector<std::vector<CodeboardAction>> all_paths = codeboard_get_all_paths(
        keyboard_layout,
        {0, 0},
        positions.data(), positions.size(),
        reordering
    );

    //  Find best path.
    std::vector<CodeboardActionWithDelay> best_path = codeboard_get_best_path(
        all_paths,
        delays,
        context->timing_variation() == 0ms
    );

    codeboard_execute_path(context, delays, best_path);
}




FastCodeEntrySettings::FastCodeEntrySettings(const FastCodeEntrySettingsOption& option)
    : keyboard_layout(option.KEYBOARD_LAYOUT)
    , include_plus(!option.SKIP_PLUS)
    , scroll_delay(option.SCROLL_DELAY0)
    , wrap_delay(option.WRAP_DELAY0)
    , digit_reordering(option.DIGIT_REORDERING)
{}

void enter_alphanumeric_code(
    Logger& logger,
    ProControllerContext& context,
    const FastCodeEntrySettings& settings,
    const std::string& code
){
    codeboard_enter_digits(
        logger,
        context,
        settings.keyboard_layout,
        code,
        settings.digit_reordering,
        CodeboardDelays{
            .hold = 5 * 8ms + context->timing_variation(),
            .cool = 3 * 8ms + context->timing_variation(),
            .press_delay = 4 * 8ms + context->timing_variation(),
            .move_delay = settings.scroll_delay + context->timing_variation(),
            .wrap_delay = settings.wrap_delay + context->timing_variation(),
        }
    );
    if (settings.include_plus){
        ssf_press_button_ptv(context, BUTTON_PLUS);
        ssf_press_button_ptv(context, BUTTON_PLUS);
    }
    ssf_flush_pipeline(context);
}




















}
}
