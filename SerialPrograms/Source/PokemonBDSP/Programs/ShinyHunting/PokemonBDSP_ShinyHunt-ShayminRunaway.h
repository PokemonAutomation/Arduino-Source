#ifndef POKEMONBDSP_SHINYHUNTRUNAWAY_H
#define POKEMONBDSP_SHINYHUNTRUNAWAY_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"
#include "PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShinyHuntShayminRunaway_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntShayminRunaway_Descriptor();
};


class ShinyHuntShayminRunaway : public SingleSwitchProgramInstance{
public:
    ShinyHuntShayminRunaway(const ShinyHuntShayminRunaway_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;


private:
    struct Stats;
    bool start_encounter(SingleSwitchProgramEnvironment& env) const;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    BooleanCheckBoxOption BIKE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
//    TimeExpressionOption<uint16_t> WATCHDOG_TIMER;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
};



}
}
}
#endif // POKEMONBDSP_SHINYHUNTRUNAWAY_H
