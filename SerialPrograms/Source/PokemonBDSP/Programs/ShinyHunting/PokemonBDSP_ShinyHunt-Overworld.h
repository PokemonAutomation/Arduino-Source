/*  Shiny Hunt - Overworld
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyHuntOverworld_H
#define PokemonAutomation_PokemonBDSP_ShinyHuntOverworld_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShinyHuntOverworld_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntOverworld_Descriptor();
};


class ShinyHuntOverworld : public SingleSwitchProgramInstance{
public:
    ShinyHuntOverworld(const ShinyHuntOverworld_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;


private:
    struct Stats;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    OverworldTrigger TRIGGER_METHOD;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
//    TimeExpressionOption<uint16_t> WATCHDOG_TIMER;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
};



}
}
}
#endif
