/*  Curry Bot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_CurryHunter_H
#define PokemonAutomation_PokemonSwSh_CurryHunter_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class CurryHunter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CurryHunter_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class CurryHunter : public SingleSwitchProgramInstance2{
public:
    CurryHunter();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    TimeExpressionOption<uint16_t> WALK_UP_DELAY;
    BooleanCheckBoxOption TAKE_VIDEO;
    SimpleIntegerOption<uint32_t> ITERATIONS;
};




}
}
}
#endif
