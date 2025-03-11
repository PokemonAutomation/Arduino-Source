/*  Activate Menu Glitch (1.1.3)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ActivateMenuGlitch113_H
#define PokemonAutomation_PokemonBDSP_ActivateMenuGlitch113_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ActivateMenuGlitch113_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ActivateMenuGlitch113_Descriptor();
};



class ActivateMenuGlitch113 : public SingleSwitchProgramInstance{
public:
    ActivateMenuGlitch113();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    MillisecondsOption FLY_A_TO_X_DELAY0;
};



}
}
}
#endif
