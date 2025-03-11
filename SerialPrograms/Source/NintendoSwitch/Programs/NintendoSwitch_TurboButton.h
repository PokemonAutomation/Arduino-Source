/*  Turbo Button
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboButton_H
#define PokemonAutomation_NintendoSwitch_TurboButton_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TurboButton_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TurboButton_Descriptor();
};


class TurboButton : public SingleSwitchProgramInstance{
public:
    TurboButton();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    IntegerEnumDropdownOption BUTTON;
    MillisecondsOption PRESS_DURATION0;
    MillisecondsOption RELEASE_DURATION0;
    SimpleIntegerOption<uint64_t> TOTAL_PRESSES;
};



}
}
#endif

