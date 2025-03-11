/*  Shiny Hunt Autonomous - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousFishing_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousFishing_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class ShinyHuntAutonomousFishing_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousFishing_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntAutonomousFishing : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousFishing();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    EncounterBotLanguage LANGUAGE;
    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption EXIT_BATTLE_TIMEOUT0;
    MillisecondsOption FISH_RESPAWN_TIME0;
};


}
}
}
#endif
