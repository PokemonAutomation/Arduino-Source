/*  Shiny Hunt - Shaymin Runaway
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyHuntShaymin_H
#define PokemonAutomation_PokemonBDSP_ShinyHuntShaymin_H

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


class ShinyHuntShaymin_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntShaymin_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntShaymin : public SingleSwitchProgramInstance{
public:
    ShinyHuntShaymin();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    bool start_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context) const;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

//    ShortcutDirection SHORTCUT;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption EXIT_BATTLE_TIMEOUT0;
};



}
}
}
#endif
