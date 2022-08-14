/*  Watt Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_WattFarmer_H
#define PokemonAutomation_PokemonSwSh_WattFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class WattFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WattFarmer_Descriptor();
};



class WattFarmer : public SingleSwitchProgramInstance{
public:
    WattFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    TimeExpressionOption<uint16_t> GRIP_MENU_WAIT;

    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint16_t> SAVE_ITERATIONS;
};


}
}
}
#endif



