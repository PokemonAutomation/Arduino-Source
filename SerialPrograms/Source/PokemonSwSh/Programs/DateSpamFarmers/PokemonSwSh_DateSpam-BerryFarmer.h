/*  Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BerryFarmer_H
#define PokemonAutomation_PokemonSwSh_BerryFarmer_H

#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BerryFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BerryFarmer_Descriptor();
};



class BerryFarmer : public SingleSwitchProgramInstance{
public:
    BerryFarmer();

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



