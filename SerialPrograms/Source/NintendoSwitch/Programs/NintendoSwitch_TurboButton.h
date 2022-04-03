/*  Turbo Button
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboButton_H
#define PokemonAutomation_NintendoSwitch_TurboButton_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TurboButton_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TurboButton_Descriptor();
};


class TurboButton : public SingleSwitchProgramInstance{
public:
    TurboButton(const TurboButton_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    EnumDropdownOption BUTTON;
    TimeExpressionOption<uint16_t> PRESS_DURATION;
    TimeExpressionOption<uint16_t> RELEASE_DURATION;
    SimpleIntegerOption<uint64_t> TOTAL_PRESSES;
};



}
}
#endif

