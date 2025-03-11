/*  Stats Reset - Calyrex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H
#define PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_IvJudgeOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class StatsResetCalyrex_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsResetCalyrex_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class StatsResetCalyrex : public SingleSwitchProgramInstance{
public:
    StatsResetCalyrex();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    PokemonBallSelectOption BALL_SELECT;
    OCR::LanguageOCROption LANGUAGE;

    BooleanCheckBoxOption CHECK_CALYREX_STATS;
    IVJudgeFilterOption CALYREX_HP;
    IVJudgeFilterOption CALYREX_ATTACK;
    IVJudgeFilterOption CALYREX_DEFENSE;
    IVJudgeFilterOption CALYREX_SPATK;
    IVJudgeFilterOption CALYREX_SPDEF;
    IVJudgeFilterOption CALYREX_SPEED;

    BooleanCheckBoxOption CHECK_HORSE_STATS;
    IVJudgeFilterOption HORSE_HP;
    IVJudgeFilterOption HORSE_ATTACK;
    IVJudgeFilterOption HORSE_DEFENSE;
    IVJudgeFilterOption HORSE_SPATK;
    IVJudgeFilterOption HORSE_SPDEF;
    IVJudgeFilterOption HORSE_SPEED;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
