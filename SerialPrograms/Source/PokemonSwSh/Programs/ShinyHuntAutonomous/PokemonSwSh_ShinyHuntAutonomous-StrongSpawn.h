/*  Shiny Hunt Autonomous - StrongSpawn
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousStrongSpawn_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntAutonomousStrongSpawn_H

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



class ShinyHuntAutonomousStrongSpawn_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAutonomousStrongSpawn_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntAutonomousStrongSpawn : public SingleSwitchProgramInstance{
public:
    ShinyHuntAutonomousStrongSpawn();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    TimeRollbackHoursOption TIME_ROLLBACK_HOURS;

    EncounterBotLanguage LANGUAGE;
    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

//    SectionDivider m_advanced_options;
};

}
}
}
#endif
