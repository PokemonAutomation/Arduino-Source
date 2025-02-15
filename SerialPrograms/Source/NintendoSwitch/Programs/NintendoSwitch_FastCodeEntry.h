/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FastCodeEntry_H
#define PokemonAutomation_NintendoSwitch_FastCodeEntry_H

#include <stdint.h>
#include <string>
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{


using namespace std::chrono_literals;



enum class KeyboardLayout{
    QWERTY,
    AZERTY
};

class FastCodeEntrySettingsOption : public GroupOption{
public:
    FastCodeEntrySettingsOption(LockMode lock_while_program_is_running);

public:
    EnumDropdownOption<KeyboardLayout> KEYBOARD_LAYOUT;
    BooleanCheckBoxOption SKIP_PLUS;

    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption DIGIT_REORDERING;
    MillisecondsOption SCROLL_DELAY0;
    MillisecondsOption WRAP_DELAY0;
};




//  Internals for testing only.

struct CodeboardDelays{
    Milliseconds hold;
    Milliseconds cool;
    Milliseconds press_delay;
    Milliseconds move_delay;
    Milliseconds wrap_delay;
};

struct CodeboardPosition{
    uint8_t row;
    uint8_t col;
};

enum class CodeboardAction : uint8_t{
    ENTER_CHAR  = 0xf0,
    SCROLL_LEFT = 0xf1,
    NORM_MOVE_UP    =        (uint8_t)DpadPosition::DPAD_UP,
    NORM_MOVE_RIGHT =        (uint8_t)DpadPosition::DPAD_RIGHT,
    NORM_MOVE_DOWN  =        (uint8_t)DpadPosition::DPAD_DOWN,
    NORM_MOVE_LEFT  =        (uint8_t)DpadPosition::DPAD_LEFT,
    WRAP_MOVE_UP    = 0x80 | (uint8_t)DpadPosition::DPAD_UP,
    WRAP_MOVE_RIGHT = 0x80 | (uint8_t)DpadPosition::DPAD_RIGHT,
    WRAP_MOVE_DOWN  = 0x80 | (uint8_t)DpadPosition::DPAD_DOWN,
    WRAP_MOVE_LEFT  = 0x80 | (uint8_t)DpadPosition::DPAD_LEFT,
};

struct CodeboardActionWithDelay{
    CodeboardAction action;
    Milliseconds delay;
};

void codeboard_enter_digits(
    Logger& logger, ProControllerContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool reordering, const CodeboardDelays& delays
);



//  This is the one you want to call.

struct FastCodeEntrySettings{
    KeyboardLayout keyboard_layout = KeyboardLayout::QWERTY;
    bool include_plus = true;
    Milliseconds scroll_delay = 64ms;
    Milliseconds wrap_delay = 64ms;
    bool digit_reordering = false;

    FastCodeEntrySettings() = default;
    FastCodeEntrySettings(const FastCodeEntrySettingsOption& option);
};
void enter_alphanumeric_code(
    Logger& logger,
    ProControllerContext& context,
    const FastCodeEntrySettings& settings,
    const std::string& code
);







}
}
#endif
