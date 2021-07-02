/*  Stow-On-Side Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StowOnSideFarmer_H
#define PokemonAutomation_PokemonSwSh_StowOnSideFarmer_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StowOnSideFarmer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    StowOnSideFarmer_Descriptor();
};



class StowOnSideFarmer : public SingleSwitchProgramInstance{
public:
    StowOnSideFarmer(const StowOnSideFarmer_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    SimpleInteger<uint32_t> SKIPS;
    SimpleInteger<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



