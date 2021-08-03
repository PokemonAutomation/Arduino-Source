/*  Loto Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LotoFarmer_H
#define PokemonAutomation_PokemonSwSh_LotoFarmer_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class LotoFarmer_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    LotoFarmer_Descriptor();
};



class LotoFarmer : public SingleSwitchProgramInstance{
public:
    LotoFarmer(const LotoFarmer_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;

    SimpleInteger<uint32_t> SKIPS;
    TimeExpression<uint16_t> MASH_B_DURATION;
};


}
}
}
#endif



