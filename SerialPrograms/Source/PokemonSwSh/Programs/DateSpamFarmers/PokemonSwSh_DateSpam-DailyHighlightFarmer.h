/*  Daily Highlight Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DailyHighlightFarmer_H
#define PokemonAutomation_PokemonSwSh_DailyHighlightFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DailyHighlightFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DailyHighlightFarmer_Descriptor();
};



class DailyHighlightFarmer : public SingleSwitchProgramInstance{
public:
    DailyHighlightFarmer();

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



