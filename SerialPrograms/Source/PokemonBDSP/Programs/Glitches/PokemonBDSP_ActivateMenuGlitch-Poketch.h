/*  Activate Menu Glitch (Poketch)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ActivateMenuGlitchPoketch_H
#define PokemonAutomation_PokemonBDSP_ActivateMenuGlitchPoketch_H

#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ActivateMenuGlitchPoketch_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ActivateMenuGlitchPoketch_Descriptor();
};



class ActivateMenuGlitchPoketch : public SingleSwitchProgramInstance{
public:
    ActivateMenuGlitchPoketch(const ActivateMenuGlitchPoketch_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    TimeExpressionOption<uint16_t> FLY_A_TO_X_DELAY;
};



}
}
}
#endif
