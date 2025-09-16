/*  Code Entry Settings Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_CodeEntrySettingsOption_H
#define PokemonAutomation_NintendoSwitch_CodeEntrySettingsOption_H

#include "Common/Cpp/Options/GroupOption.h"
//#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardEntryMappings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



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
    DigitEntryTimingsOption(bool switch2);

public:
    BooleanCheckBoxOption REORDERING;
    MillisecondsOption TIME_UNIT;
    MillisecondsOption HOLD;
    MillisecondsOption COOLDOWN;
};

class KeyboardEntryTimingsOption : public GroupOption{
public:
    KeyboardEntryTimingsOption(bool switch2);

public:
    BooleanCheckBoxOption REORDERING;
    MillisecondsOption TIME_UNIT;
    MillisecondsOption HOLD;
    MillisecondsOption COOLDOWN;
};

class KeyboardControllerTimingsOption : public GroupOption{
public:
    KeyboardControllerTimingsOption();

public:
    BooleanCheckBoxOption PARLLELIZE;
    MillisecondsOption TIME_UNIT;
    MillisecondsOption HOLD;
    MillisecondsOption COOLDOWN;
};





}
}
#endif
