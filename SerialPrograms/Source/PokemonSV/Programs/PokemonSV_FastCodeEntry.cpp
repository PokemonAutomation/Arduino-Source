/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_FastCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


FastCodeEntry_Descriptor::FastCodeEntry_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:FastCodeEntry",
        STRING_POKEMON + " SV", "Fast Code Entry (FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/FastCodeEntry.md",
        "Quickly enter a 4 or 6 digit link code.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}

FastCodeEntry::FastCodeEntry()
    : CODE(
        "<b>Link Code:</b><br>Must be 4-digit numeric or 6-digit alphanumeric. (not case sensitive)<br>"
        "(Box is big so it's easy to land your mouse on.)",
        LockWhileRunning::LOCKED,
        "0123", "0123"
    )
    , KEYBOARD_LAYOUT(
        "<b>Keyboard Layout:</b>",
        {
            {KeyboardLayout::QWERTY, "qwerty", "QWERTY"},
            {KeyboardLayout::AZERTY, "azerty", "AZERTY"},
        },
        LockWhileRunning::LOCKED,
        KeyboardLayout::QWERTY
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options: (developer only)</b></font>"
    )
    , SKIP_PLUS(
        "<b>Skip the Plus:</b>",
        LockWhileRunning::LOCKED,
        false
    )
    , DIGIT_REORDERING(
        "<b>Digit Reordering:</b><br>Allow digits to be entered out of order.",
        LockWhileRunning::LOCKED,
        PreloadSettings::instance().DEVELOPER_MODE,
        PreloadSettings::instance().DEVELOPER_MODE
    )
    , SCROLL_DELAY(
        "<b>Scroll Delay:</b><br>Delay to scroll between adjacent keys.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        3, 15,
        PreloadSettings::instance().DEVELOPER_MODE ? "4" : "10",
        PreloadSettings::instance().DEVELOPER_MODE ? "4" : "10"
    )
    , WRAP_DELAY(
        "<b>Wrap Delay:</b><br>Delay to wrap between left/right edges.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        3, 15,
        PreloadSettings::instance().DEVELOPER_MODE ? "5" : "10",
        PreloadSettings::instance().DEVELOPER_MODE ? "5" : "10"
    )
{
    PA_ADD_OPTION(CODE);
    PA_ADD_OPTION(KEYBOARD_LAYOUT);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(m_advanced_options);
        PA_ADD_OPTION(SKIP_PLUS);
        PA_ADD_OPTION(DIGIT_REORDERING);
        PA_ADD_OPTION(SCROLL_DELAY);
        PA_ADD_OPTION(WRAP_DELAY);
    }
}

void FastCodeEntry::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
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
        {'0', '0'}, {'O', '1'}, {'o', '1'},

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

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Connect the controller.
        pbf_press_button(context, BUTTON_PLUS, 5, 3);

        if (code.size() == 4){
            if (!digits_only){
                throw UserSetupError(console.logger(), "4-digit codes must be only digits.");
            }
            enter_digits_str(context, 4, code.c_str());
        }else if (code.size() == 6){
            enter_alphanumeric_code(
                console.logger(), context,
                KEYBOARD_LAYOUT, code,
                !SKIP_PLUS,
                SCROLL_DELAY, WRAP_DELAY, DIGIT_REORDERING
            );
        }else{
            throw UserSetupError(console.logger(), "Invalid code length. Must be 4 or 6 characters long.");
        }

    });

}



}
}
}
