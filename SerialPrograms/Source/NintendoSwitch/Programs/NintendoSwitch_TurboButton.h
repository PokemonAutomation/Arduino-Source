/*  Turbo Button
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboButton_H
#define PokemonAutomation_NintendoSwitch_TurboButton_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    IntegerEnumDropdownOption BUTTON;
    TimeExpressionOption<uint16_t> PRESS_DURATION;
    TimeExpressionOption<uint16_t> RELEASE_DURATION;
    SimpleIntegerOption<uint64_t> TOTAL_PRESSES;
};



}
}
#endif

