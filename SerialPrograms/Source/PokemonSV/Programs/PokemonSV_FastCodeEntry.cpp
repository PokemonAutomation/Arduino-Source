/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_FastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


FastCodeEntry_Descriptor::FastCodeEntry_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:FastCodeEntry",
        STRING_POKEMON + " SV", "Fast Code Entry (FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/FastCodeEntry.md",
        "Force your way into raids by entering the 4 or 6 digit link codes in under a second.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

FastCodeEntry::FastCodeEntry()
    : CODE(
        false,
        "<b>Link Code:</b><br>Must be 4-digit numeric or 6-digit alphanumeric. (not case sensitive)",
        LockWhileRunning::LOCKED,
        "0123", "0123"
    ),
    KEYBOARD_LAYOUT(
        "<b>Keyboard Layout:</b>",
        {
            {KeyboardLayout::QWERTY, "qwerty", "QWERTY"},
            {KeyboardLayout::AZERTY, "azerty", "AZERTY"},
        },
        LockWhileRunning::LOCKED,
        KeyboardLayout::QWERTY
    )
{
    PA_ADD_OPTION(CODE);
    PA_ADD_OPTION(KEYBOARD_LAYOUT);
}

void FastCodeEntry::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    pbf_press_button(context, BUTTON_PLUS, 3, 0);


    static const std::map<char, char> MAP{
        {'1', '1'},
        {'2', '2'},
        {'3', '3'},
        {'4', '4'},
        {'5', '5'},
        {'6', '6'},
        {'7', '7'},
        {'8', '8'},
        {'9', '9'},
        {'0', '0'},

        {'Q', 'Q'}, {'q', 'Q'},
        {'W', 'W'}, {'w', 'W'},
        {'E', 'E'}, {'e', 'E'},
        {'R', 'R'}, {'r', 'R'},
        {'T', 'T'}, {'t', 'T'},
        {'Y', 'Y'}, {'y', 'Y'},
        {'U', 'U'}, {'u', 'U'},
        {'P', 'P'}, {'p', 'P'},

        {'A', 'A'}, {'a', 'A'},
        {'S', 'S'}, {'s', 'S'},
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

    std::string code;

    //  Prune invalid characters.
    bool digits_only = true;
    for (char ch : (std::string)CODE){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            continue;
        }
        ch = iter->second;
        digits_only &= '0' <= ch && ch <= '9';
        code += ch;
    }

    if (code.size() == 4){
        if (!digits_only){
            throw UserSetupError(env.logger(), "4-digit codes must be only digits.");
        }
        enter_digits_str(context, 4, code.c_str());
    }else if (code.size() == 6){
        enter_alphanumeric_code(env.logger(), context, code, KEYBOARD_LAYOUT);
    }else{
        throw UserSetupError(env.logger(), "Invalid code length. Must be 4 or 6 characters long.");
    }

}



}
}
}
