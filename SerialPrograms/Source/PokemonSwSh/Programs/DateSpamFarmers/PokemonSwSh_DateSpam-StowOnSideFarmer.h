/*  Stow-On-Side Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StowOnSideFarmer_H
#define PokemonAutomation_PokemonSwSh_StowOnSideFarmer_H

#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StowOnSideFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StowOnSideFarmer_Descriptor();
};



class StowOnSideFarmer : public SingleSwitchProgramInstance{
public:
    StowOnSideFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



