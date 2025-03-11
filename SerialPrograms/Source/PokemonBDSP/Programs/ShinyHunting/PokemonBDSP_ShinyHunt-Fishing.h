/*  Shiny Hunt - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyHuntFishing_H
#define PokemonAutomation_PokemonBDSP_ShinyHuntFishing_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShinyHuntFishing_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntFishing_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntFishing : public SingleSwitchProgramInstance{
public:
    ShinyHuntFishing();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    void run_trigger(ProControllerContext& context) const;
    bool find_encounter(SingleSwitchProgramEnvironment& env) const;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    ShortcutDirectionOption SHORTCUT;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption EXIT_BATTLE_TIMEOUT0;
};



}
}
}
#endif
