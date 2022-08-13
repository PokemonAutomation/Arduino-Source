/*  Loto Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LotoFarmer_H
#define PokemonAutomation_PokemonSwSh_LotoFarmer_H

#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class LotoFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LotoFarmer_Descriptor();
};



class LotoFarmer : public SingleSwitchProgramInstance{
public:
    LotoFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    SimpleIntegerOption<uint32_t> SKIPS;
    TimeExpressionOption<uint16_t> MASH_B_DURATION;
};


}
}
}
#endif



