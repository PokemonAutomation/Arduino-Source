/*  Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSV_CodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const char* normalize_code(std::string& normalized_code, const std::string& code){
    static const std::map<char, char> MAP{
        {'1', '1'}, {'I', '1'}, {'i', '1'},
        {'2', '2'},
        {'3', '3'},
        {'4', '4'},
        {'5', '5'},
        {'6', '6'},
        {'7', '7'},
        {'8', '8'},
        {'9', '9'},
        {'0', '0'}, {'O', '0'}, {'o', '0'},

        {'Q', 'Q'}, {'q', 'Q'},
        {'W', 'W'}, {'w', 'W'},
        {'E', 'E'}, {'e', 'E'},
        {'R', 'R'}, {'r', 'R'},
        {'T', 'T'}, {'t', 'T'},
        {'Y', 'Y'}, {'y', 'Y'},
        {'U', 'U'}, {'u', 'U'},
        {'P', 'P'}, {'p', 'P'},

        {'A', 'A'}, {'a', 'A'},
        {'S', 'S'}, {'s', 'S'}, {'Z', 'S'}, {'z', 'S'},
        {'D', 'D'}, {'d', 'D'},
        {'F', 'F'}, {'f', 'F'},
        {'G', 'G'}, {'g', 'G'},
        {'H', 'H'}, {'h', 'H'},
        {'J', 'J'}, {'j', 'J'},
        {'K', 'K'}, {'k', 'K'},
        {'L', 'L'}, {'l', 'L'},

        {'X', 'X'}, {'x', 'X'},
        {'C', 'C'}, {'c', 'C'},
        {'V', 'V'}, {'v', 'V'},
        {'B', 'B'}, {'b', 'B'},
        {'N', 'N'}, {'n', 'N'},
        {'M', 'M'}, {'m', 'M'},
    };

    normalized_code.clear();

    //  Prune invalid characters.
    bool digits_only = true;
    for (char ch : code){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            continue;
        }
        ch = iter->second;
        digits_only &= '0' <= ch && ch <= '9';
        normalized_code += ch;
    }

    switch (normalized_code.size()){
    case 4:
        if (!digits_only){
            return "4-digit codes must be only digits.";
        }
        break;
    case 6:
        break;
    case 8:
        if (!digits_only){
            return "8-digit codes must be only digits.";
        }
        break;
    default:
        return "Invalid code length. Must be 4, 6, or 8 characters long.";
    }

    return nullptr;
}

void enter_code(
    ConsoleHandle& console, BotBaseContext& context,
    const FastCodeEntrySettings& settings, const std::string& normalized_code,
    bool connect_controller_press
){
    if (connect_controller_press){
        //  Connect the controller.
        pbf_press_button(context, BUTTON_R | BUTTON_L, 5, 3);
    }

    switch (normalized_code.size()){
    case 4:
        enter_digits_str(context, 4, normalized_code.c_str());
        break;
    case 6:
        enter_alphanumeric_code(
            console.logger(), context,
            settings,
            normalized_code
        );
        break;
    case 8:
        enter_digits_str(context, 8, normalized_code.c_str());
        break;
    }
}
const char* enter_code(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const FastCodeEntrySettings& settings, const std::string& code,
    bool connect_controller_press
){
    std::string normalized_code;
    const char* error = normalize_code(normalized_code, code);
    if (error){
        return error;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        if (connect_controller_press){
            //  Connect the controller.
            pbf_press_button(context, BUTTON_R | BUTTON_L, 5, 3);
        }

        switch (normalized_code.size()){
        case 4:
            enter_digits_str(context, 4, normalized_code.c_str());
            break;
        case 6:
            enter_alphanumeric_code(
                console.logger(), context,
                settings,
                normalized_code
            );
            break;
        case 8:
            enter_digits_str(context, 8, normalized_code.c_str());
            break;
        }

    });

    return nullptr;
}



}
}
}
