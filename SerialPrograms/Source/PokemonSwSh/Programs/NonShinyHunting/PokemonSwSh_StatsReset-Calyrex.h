/*  Stats Reset - Calyrex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H
#define PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"

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



class StatsResetCalyrex : public SingleSwitchProgramInstance2{
public:
    StatsResetCalyrex();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    PokemonBallSelect BALL_SELECT;
    OCR::LanguageOCR LANGUAGE;

    BooleanCheckBoxOption CHECK_CALYREX_STATS;
    IVCheckerFilterOption CALYREX_HP;
    IVCheckerFilterOption CALYREX_ATTACK;
    IVCheckerFilterOption CALYREX_DEFENSE;
    IVCheckerFilterOption CALYREX_SPATK;
    IVCheckerFilterOption CALYREX_SPDEF;
    IVCheckerFilterOption CALYREX_SPEED;

    BooleanCheckBoxOption CHECK_HORSE_STATS;
    IVCheckerFilterOption HORSE_HP;
    IVCheckerFilterOption HORSE_ATTACK;
    IVCheckerFilterOption HORSE_DEFENSE;
    IVCheckerFilterOption HORSE_SPATK;
    IVCheckerFilterOption HORSE_SPDEF;
    IVCheckerFilterOption HORSE_SPEED;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
