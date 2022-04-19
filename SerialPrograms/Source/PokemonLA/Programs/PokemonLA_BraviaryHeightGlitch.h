/*  Braviary Height Glitch
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_BraviaryHeightGlitch_H
#define PokemonAutomation_PokemonLA_BraviaryHeightGlitch_H

#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class BraviaryHeightGlitch_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    BraviaryHeightGlitch_Descriptor();
};


class BraviaryHeightGlitch : public SingleSwitchProgramInstance{
public:
    BraviaryHeightGlitch(const BraviaryHeightGlitch_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;
};





}
}
}
#endif
