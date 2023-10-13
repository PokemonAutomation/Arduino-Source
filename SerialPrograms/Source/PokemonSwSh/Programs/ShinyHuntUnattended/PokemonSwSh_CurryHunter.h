/*  Curry Bot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_CurryHunter_H
#define PokemonAutomation_PokemonSwSh_CurryHunter_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class CurryHunter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CurryHunter_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class CurryHunter : public SingleSwitchProgramInstance{
public:
    CurryHunter();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TimeExpressionOption<uint16_t> WALK_UP_DELAY;
    SimpleIntegerOption<uint16_t> SMALL_POKEMON;
    BooleanCheckBoxOption TAKE_VIDEO;
    SimpleIntegerOption<uint32_t> ITERATIONS;
};




}
}
}
#endif
