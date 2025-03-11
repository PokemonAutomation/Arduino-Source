/*  Activate Menu Glitch (Poketch)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ActivateMenuGlitchPoketch_H
#define PokemonAutomation_PokemonBDSP_ActivateMenuGlitchPoketch_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ActivateMenuGlitch112_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ActivateMenuGlitch112_Descriptor();
};



class ActivateMenuGlitch112 : public SingleSwitchProgramInstance{
public:
    ActivateMenuGlitch112();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    MillisecondsOption FLY_A_TO_X_DELAY0;
};



}
}
}
#endif
