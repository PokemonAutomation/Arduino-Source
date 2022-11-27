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

namespace PokemonAutomation{
namespace NintendoSwitch{


struct CodeboardPosition{
    uint8_t row;
    uint8_t col;
};

const std::map<char, CodeboardPosition>& CODEBOARD_POSITIONS(){
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

struct DigitPath{
    uint8_t length = 0;
    bool left_cursor = false;
    uint8_t path[14];
};
DigitPath get_codeboard_digit_path(
    CodeboardPosition source,
    CodeboardPosition destination
){
    DigitPath path;

    if (source.row < destination.row){
        size_t diff = destination.row - source.row;
        for (size_t c = 0; c < diff; c++){
            path.path[path.length++] = DPAD_DOWN;
        }
    }else{
        size_t diff = source.row - destination.row;
        for (size_t c = 0; c < diff; c++){
            path.path[path.length++] = DPAD_UP;
        }
    }

    uint8_t diff = (12 + destination.col - source.col) % 12;
    if (diff <= 6){
        for (size_t c = 0; c < diff; c++){
            path.path[path.length++] = DPAD_RIGHT;
        }
    }else{
        diff = 12 - diff;
        for (size_t c = 0; c < diff; c++){
            path.path[path.length++] = DPAD_LEFT;
        }
    }

    return path;
}
size_t get_codeboard_path_cost(const std::vector<DigitPath>& path){
    size_t total_cost = 0;
    for (const DigitPath& digit : path){
        size_t cost = digit.length * 3;
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
    CodeboardPosition start
){
    if (e - s == 1){
        return {get_codeboard_digit_path(start, positions[s])};
    }

    std::vector<DigitPath> forward;
    {
        CodeboardPosition position = positions[s];
        forward.emplace_back(get_codeboard_digit_path(start, position));
        std::vector<DigitPath> remaining = get_codeboard_path(positions, s + 1, e, position);
        forward.insert(forward.end(), remaining.begin(), remaining.end());
    }

    std::vector<DigitPath> reverse;
    {
        CodeboardPosition position = positions[e - 1];
        reverse.emplace_back(get_codeboard_digit_path(start, position));
        reverse.back().left_cursor = true;
        std::vector<DigitPath> remaining = get_codeboard_path(positions, s, e - 1, position);
        reverse.insert(reverse.end(), remaining.begin(), remaining.end());
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
    CodeboardPosition start = {0, 0}
){
    const std::map<char, CodeboardPosition>& POSITION_MAP = CODEBOARD_POSITIONS();
    std::vector<CodeboardPosition> positions;
    for (char ch : code){
        auto iter = POSITION_MAP.find(ch);
        if (iter == POSITION_MAP.end()){
            throw OperationFailedException(logger, "Invalid code character.");
        }
        positions.emplace_back(iter->second);
    }
    return get_codeboard_path(positions, 0, positions.size(), start);
}



void run_codeboard_path(
    BotBaseContext& context,
    const std::vector<DigitPath>& path
){
    for (const DigitPath& digit : path){
        if (digit.length > 0){
            for (size_t c = 0; c < digit.length - 1; c++){
                ssf_issue_scroll(context, digit.path[c], 3);
            }
            ssf_issue_scroll(context, digit.path[digit.length - 1], 0);
        }
        ssf_press_button(context, BUTTON_A, 3);
        if (digit.left_cursor){
            ssf_press_button(context, BUTTON_L, 1);
        }
    }
}


void enter_alphanumeric_code(
    Logger& logger,
    BotBaseContext& context,
    const std::string& code
){
    run_codeboard_path(context, get_codeboard_path(logger, code));
    pbf_press_button(context, BUTTON_PLUS, 5, 3);
}



}
}
