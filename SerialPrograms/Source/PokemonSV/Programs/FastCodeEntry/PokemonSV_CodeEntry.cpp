/*  Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardCodeEntry.h"
#include "PokemonSV_CodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



FastCodeEntryKeyboardLayout::FastCodeEntryKeyboardLayout()
    : GroupOption(
        "Keyboard Layouts",
        LockMode::UNLOCK_WHILE_RUNNING,
        EnableMode::ALWAYS_ENABLED
    )
    , CONSOLE(4)
{
    CONSOLE.emplace_back("<b>Switch 0 (Top Left):</b>");
    CONSOLE.emplace_back("<b>Switch 1 (Top Right):</b>");
    CONSOLE.emplace_back("<b>Switch 2 (Bottom Left):</b>");
    CONSOLE.emplace_back("<b>Switch 3 (Bottom Right):</b>");
    PA_ADD_OPTION(CONSOLE[0]);
    PA_ADD_OPTION(CONSOLE[1]);
    PA_ADD_OPTION(CONSOLE[2]);
    PA_ADD_OPTION(CONSOLE[3]);
}
FastCodeEntryKeyboardLayout::~FastCodeEntryKeyboardLayout() = default;


FastCodeEntrySettingsOption::FastCodeEntrySettingsOption()
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
{
    PA_ADD_OPTION(SKIP_PLUS);
    PA_ADD_OPTION(KEYBOARD_LAYOUTS);
}
void FastCodeEntrySettingsOption::set_active_consoles(size_t active_consoles){
    for (size_t c = 0; c < 4; c++){
        KEYBOARD_LAYOUTS.CONSOLE[c].set_visibility(
            c < active_consoles ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
        );
    }
}



const char* normalize_code(std::string& normalized_code, const std::string& code, bool override_mode){
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

    if (override_mode){
        return nullptr;
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
    ConsoleHandle& console, AbstractControllerContext& context,
    KeyboardLayout keyboard_layout,
    const std::string& normalized_code, bool force_keyboard_mode,
    bool include_plus,
    bool connect_controller_press
){
    if (connect_controller_press){
        //  Connect the controller.
        auto* procon = context->cast<ProController>();
        if (procon){
            ProControllerContext subcontext(context, *procon);
            pbf_press_button(subcontext, BUTTON_R | BUTTON_L, 5, 3);
        }
    }

    if (force_keyboard_mode){
        FastCodeEntry::keyboard_enter_code(
            console, context, keyboard_layout,
            normalized_code, include_plus
        );
        return;
    }

    switch (normalized_code.size()){
    case 4:
//        enter_digits_str(context, 4, normalized_code.c_str());
        FastCodeEntry::numberpad_enter_code(console, context, normalized_code, include_plus);
        break;
    case 6:
        FastCodeEntry::keyboard_enter_code(
            console, context, keyboard_layout,
            normalized_code, include_plus
        );
        break;
    case 8:
//        enter_digits_str(context, 8, normalized_code.c_str());
        FastCodeEntry::numberpad_enter_code(console, context, normalized_code, include_plus);
        break;
    }
}
const char* enter_code(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const FastCodeEntrySettings& settings,
    const std::string& code, bool force_keyboard_mode,
    bool connect_controller_press
){
    std::string normalized_code;
    const char* error = normalize_code(normalized_code, code, force_keyboard_mode);
    if (error){
        return error;
    }

    env.run_in_parallel(scope, [&](CancellableScope& scope, ConsoleHandle& console){
        AbstractControllerContext context(scope, console.controller());
        enter_code(
            console, context,
            settings.keyboard_layout[console.index()],
            normalized_code, force_keyboard_mode,
            !settings.skip_plus,
            connect_controller_press
        );
    });

    return nullptr;
}



}
}
}
