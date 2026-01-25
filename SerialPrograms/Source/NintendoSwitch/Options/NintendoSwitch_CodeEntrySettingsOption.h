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
#include "Controllers/ControllerTypes.h"
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



class CodeboardTimingsOption : public GroupOption{
public:
    CodeboardTimingsOption(
        std::string label,
        bool switch2,
        ControllerPerformanceClass performance_class
    );

private:
    static Milliseconds get_unit_timing(ControllerPerformanceClass performance_class);
    Milliseconds unit;

public:
    BooleanCheckBoxOption REORDERING;
    MillisecondsOption HOLD;
    MillisecondsOption COOLDOWN;
    MillisecondsOption PRESS_DELAY;
    MillisecondsOption MOVE_DELAY;
    MillisecondsOption SCROLL_DELAY;
    MillisecondsOption WRAP_DELAY;
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
