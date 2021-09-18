/*  Stats Reset - Calyrex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H
#define PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


class StatsResetCalyrex_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    StatsResetCalyrex_Descriptor();
};



class StatsResetCalyrex : public SingleSwitchProgramInstance{
public:
    StatsResetCalyrex(const StatsResetCalyrex_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    IVCheckerReader m_iv_checker_reader;

    PokemonBallSelect BALL_SELECT;
    OCR::LanguageOCR LANGUAGE;
    BooleanCheckBoxOption CHECK_CALYREX_STATS;
    IVCheckerOption CALYREX_HP;
    IVCheckerOption CALYREX_ATTACK;
    IVCheckerOption CALYREX_DEFENSE;
    IVCheckerOption CALYREX_SPATK;
    IVCheckerOption CALYREX_SPDEF;
    IVCheckerOption CALYREX_SPEED;
    BooleanCheckBoxOption CHECK_HORSE_STATS;
    IVCheckerOption HORSE_HP;
    IVCheckerOption HORSE_ATTACK;
    IVCheckerOption HORSE_DEFENSE;
    IVCheckerOption HORSE_SPATK;
    IVCheckerOption HORSE_SPDEF;
    IVCheckerOption HORSE_SPEED;
};


}
}
}
#endif
