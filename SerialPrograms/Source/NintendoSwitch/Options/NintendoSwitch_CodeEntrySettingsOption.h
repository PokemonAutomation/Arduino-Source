/*  Code Entry Settings Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_CodeEntrySettingsOption_H
#define PokemonAutomation_NintendoSwitch_CodeEntrySettingsOption_H

#include "Common/Cpp/Options/GroupOption.h"
//#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



enum class KeyboardLayout{
    QWERTY,
    AZERTY
};
class KeyboardLayoutOption : public EnumDropdownOption<KeyboardLayout>{
public:
    KeyboardLayoutOption();
    KeyboardLayoutOption(std::string label);
};

class CodeEntrySkipPlusOption : public BooleanCheckBoxOption{
public:
    CodeEntrySkipPlusOption();
};




class DigitEntryTimingsOption : public GroupOption{
public:
    DigitEntryTimingsOption();

public:
    BooleanCheckBoxOption DIGIT_REORDERING;

    MillisecondsOption BUTTON_HOLD;
    MillisecondsOption BUTTON_COOLDOWN;

    MillisecondsOption PRESS_DELAY;
    MillisecondsOption SCROLL_DELAY;
};

class KeyboardEntryTimingsOption : public GroupOption{
public:
    KeyboardEntryTimingsOption();

public:
    BooleanCheckBoxOption DIGIT_REORDERING;

    MillisecondsOption BUTTON_HOLD;
    MillisecondsOption BUTTON_COOLDOWN;

    MillisecondsOption PRESS_DELAY;
    MillisecondsOption SCROLL_DELAY;
    MillisecondsOption WRAP_DELAY;
};





}
}
#endif
