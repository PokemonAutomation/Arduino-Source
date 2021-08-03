/*  Berry Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BerryFarmer_H
#define PokemonAutomation_PokemonSwSh_BerryFarmer_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BerryFarmer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    BerryFarmer_Descriptor();
};



class BerryFarmer : public SingleSwitchProgramInstance{
public:
    BerryFarmer(const BerryFarmer_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;

    SimpleInteger<uint32_t> SKIPS;
    SimpleInteger<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



