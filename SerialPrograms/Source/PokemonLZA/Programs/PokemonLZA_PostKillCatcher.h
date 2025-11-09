/*  Post-Kill Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_PostKillCatcher_H
#define PokemonAutomation_PokemonLZA_PostKillCatcher_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class PostKillCatcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PostKillCatcher_Descriptor();
};

class PostKillCatcher : public SingleSwitchProgramInstance{
public:
    PostKillCatcher();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<int8_t> RIGHT_SCROLLS;
    MillisecondsOption SCROLL_HOLD;
    MillisecondsOption SCROLL_RELEASE;
    MillisecondsOption POST_THROW_WAIT;
};





}
}
}
#endif
