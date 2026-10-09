/*  Turbo A
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TurboA_H
#define PokemonAutomation_NintendoSwitch_TurboA_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TurboA_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TurboA_Descriptor();
};



class TurboA : public SingleSwitchProgramInstance{
public:
    using Descriptor = TurboA_Descriptor;
    TurboA();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // How fast to mash A. Every press is one command sent to the microcontroller
    // over the serial port, plus one "command finished" reply sent back, so the
    // press rate directly sets how busy the serial link is.
    // Slower speeds lower that traffic and could make serial connection more stable.
    enum class MashSpeed{
        Fast,       // ~16 presses/s. The standard mash timing.
        Reduced,    // 5 presses/s.
        Minimal,    // 2 presses/s.
    };

    // Mash A for `duration` at the speed chosen in `MASH_SPEED`.
    void mash_A(ProControllerContext& context, Milliseconds duration) const;

private:
    StartInGripOrGameOption START_LOCATION;
    EnumDropdownOption<MashSpeed> MASH_SPEED;
    MillisecondsOption TIME_LIMIT;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
};




}
}
#endif



