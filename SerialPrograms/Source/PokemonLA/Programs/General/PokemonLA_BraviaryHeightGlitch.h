/*  Braviary Height Glitch
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_BraviaryHeightGlitch_H
#define PokemonAutomation_PokemonLA_BraviaryHeightGlitch_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class BraviaryHeightGlitch_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BraviaryHeightGlitch_Descriptor();
};


class BraviaryHeightGlitch : public SingleSwitchProgramInstance{
public:
    BraviaryHeightGlitch();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
};





}
}
}
#endif
