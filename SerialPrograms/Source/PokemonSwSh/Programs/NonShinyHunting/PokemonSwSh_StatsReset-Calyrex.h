/*  Stats Reset - Calyrex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H
#define PokemonAutomation_PokemonSwSh_StatsResetCalyrex_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
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



class StatsResetCalyrex : public SingleSwitchProgramInstance{
public:
    StatsResetCalyrex();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    PokemonBallSelectOption BALL_SELECT;
    OCR::LanguageOCROption LANGUAGE;

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
