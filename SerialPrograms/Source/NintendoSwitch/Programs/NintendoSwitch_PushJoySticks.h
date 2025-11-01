/*  Push Joy Sticks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PushJoySticks_H
#define PokemonAutomation_NintendoSwitch_PushJoySticks_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class PushJoySticks_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PushJoySticks_Descriptor();
};


class PushJoySticks : public SingleSwitchProgramInstance{
public:
    PushJoySticks();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    IntegerEnumDropdownOption JOYCON_CLICK;
    SimpleIntegerOption<uint8_t> LEFT_X;
    SimpleIntegerOption<uint8_t> LEFT_Y;
    SimpleIntegerOption<uint8_t> RIGHT_X;
    SimpleIntegerOption<uint8_t> RIGHT_Y;
    MillisecondsOption PRESS_DURATION;
    MillisecondsOption RELEASE_DURATION;
};



}
}
#endif

