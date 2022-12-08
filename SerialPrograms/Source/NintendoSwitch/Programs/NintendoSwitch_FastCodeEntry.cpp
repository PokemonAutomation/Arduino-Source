/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "NintendoSwitch_FastCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



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

DigitPath get_codeboard_digit_path(CodeboardPosition source, CodeboardPosition destination){
    uint8_t scroll_delay = 3;
    uint8_t wrap_delay = 4;

    DigitPath path;

    //  Vertical is easy since there's no wrapping and no hazards.
    if (source.row < destination.row){
        size_t diff = destination.row - source.row;
        for (size_t c = 0; c < diff; c++){
            path.path[path.length++] = {DPAD_DOWN, scroll_delay};
        }
    }else{
        size_t diff = source.row - destination.row;
        for (size_t c = 0; c < diff; c++){
            path.path[path.length++] = {DPAD_UP, scroll_delay};
        }
    }

    //  Horizontal is messy because we need extra delay on wrapping.
    uint8_t col = source.col;
    while (true){
        if (col == destination.col){
            if (path.length > 0){
                path.path[path.length - 1].delay = 0;
            }
            break;
        }

        uint8_t direction;
        if (destination.col > col){
            if (destination.col - col <= 6){
                direction = DPAD_RIGHT;
                col++;
            }else{
                direction = DPAD_LEFT;
                col--;
            }
        }else{
            if (col - destination.col <= 6){
                direction = DPAD_LEFT;
                col--;
            }else{
                direction = DPAD_RIGHT;
                col++;
            }
        }

        if (col == (uint8_t)-1){
            col = 11;
        }
        if (col == 12){
            col = 0;
        }

        uint8_t delay = scroll_delay;
        if (direction == DPAD_RIGHT && col == 11){
            delay = wrap_delay;
        }

//        cout << "col = " << (int)col << ", length = " << (int)path.length << endl;

        path.path[path.length++] = {direction, delay};
    }

    return path;
}
size_t get_codeboard_path_cost(const DigitPath& path){
    size_t total_cost = 0;
    for (uint8_t c = 0; c < path.length; c++){
        total_cost += path.path[c].delay;
    }
    return total_cost;
}
size_t get_codeboard_path_cost(const std::vector<DigitPath>& path){
    size_t total_cost = 0;
    for (const DigitPath& digit : path){
        size_t cost = get_codeboard_path_cost(digit);
        if (digit.left_cursor){
            cost++;
        }
        cost = std::max<size_t>(cost, 8);
        total_cost += cost;
    }
    return total_cost;
}

std::vector<DigitPath> get_codeboard_path(
    const std::vector<CodeboardPosition>& positions, size_t s, size_t e,
    CodeboardPosition start,
    KeyboardLayout keyboard_layout,
    bool fast
){
    if (e - s == 1){
        return {get_codeboard_digit_path(start, positions[s])};
    }

    std::vector<DigitPath> forward;
    {
        CodeboardPosition position = positions[s];
        forward.emplace_back(get_codeboard_digit_path(start, position));
        std::vector<DigitPath> remaining = get_codeboard_path(positions, s + 1, e, position, keyboard_layout, fast);
        forward.insert(forward.end(), remaining.begin(), remaining.end());
    }

    std::vector<DigitPath> reverse;
    {
        CodeboardPosition position = positions[e - 1];
        reverse.emplace_back(get_codeboard_digit_path(start, position));
        reverse.back().left_cursor = true;
        std::vector<DigitPath> remaining = get_codeboard_path(positions, s, e - 1, position, keyboard_layout, fast);
        reverse.insert(reverse.end(), remaining.begin(), remaining.end());
    }

    if (!fast){
        return forward;
    }

    if (get_codeboard_path_cost(forward) <= get_codeboard_path_cost(reverse)){
        return forward;
    }else{
        return reverse;
    }
}
std::vector<DigitPath> get_codeboard_path(
    Logger& logger,
    const std::string& code,
    KeyboardLayout keyboard_layout,
    bool fast,
    CodeboardPosition start = {0, 0}
){
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
            throw OperationFailedException(logger, "Invalid code character.");
        }
        positions.emplace_back(iter->second);
    }
    return get_codeboard_path(positions, 0, positions.size(), start, keyboard_layout, fast);
}


void move_codeboard(BotBaseContext& context, const DigitPath& path, bool fast){
    uint8_t min_delay = fast ? 0 : 10;
    if (path.length > 0){
        for (size_t c = 0; c < (size_t)path.length - 0; c++){
            ssf_issue_scroll(
                context,
                path.path[c].direction,
                std::max(path.path[c].delay, min_delay)
            );
        }
    }
    ssf_press_button(context, BUTTON_A, 3);
}

void run_codeboard_path(
    BotBaseContext& context,
    const std::vector<DigitPath>& path,
    bool fast
){
    for (const DigitPath& digit : path){
        move_codeboard(context, digit, fast);
        if (digit.left_cursor){
            ssf_press_button(context, BUTTON_L, 1);
        }
    }
}


void enter_alphanumeric_code(
    Logger& logger,
    BotBaseContext& context,
    const std::string& code,
    KeyboardLayout keyboard_layout,
    bool include_plus, bool fast
){
    run_codeboard_path(context, get_codeboard_path(logger, code, keyboard_layout, fast), fast);
    if (include_plus){
        pbf_press_button(context, BUTTON_PLUS, 5, 3);
    }
}



}
}
