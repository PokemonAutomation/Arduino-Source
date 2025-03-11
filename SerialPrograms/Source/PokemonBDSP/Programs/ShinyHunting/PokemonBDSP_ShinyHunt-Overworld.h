/*  Shiny Hunt - Overworld
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyHuntOverworld_H
#define PokemonAutomation_PokemonBDSP_ShinyHuntOverworld_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShinyHuntOverworld_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntOverworld_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntOverworld : public SingleSwitchProgramInstance{
public:
    ShinyHuntOverworld();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    OverworldTrigger TRIGGER_METHOD;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    BooleanCheckBoxOption RESET_GAME_WHEN_ERROR;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
//    MillisecondsOption WATCHDOG_TIMER0;
    MillisecondsOption EXIT_BATTLE_TIMEOUT0;
};



}
}
}
#endif
