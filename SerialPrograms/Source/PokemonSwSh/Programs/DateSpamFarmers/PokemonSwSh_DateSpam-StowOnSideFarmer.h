/*  Stow-On-Side Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StowOnSideFarmer_H
#define PokemonAutomation_PokemonSwSh_StowOnSideFarmer_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

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

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



