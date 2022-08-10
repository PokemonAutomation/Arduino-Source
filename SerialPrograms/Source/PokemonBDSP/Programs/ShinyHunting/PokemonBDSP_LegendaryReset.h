/*  Shiny Hunt - Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_LegendaryReset_H
#define PokemonAutomation_PokemonBDSP_LegendaryReset_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class LegendaryReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LegendaryReset_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class LegendaryReset : public SingleSwitchProgramInstance2{
public:
    LegendaryReset();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    BooleanCheckBoxOption WALK_UP;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
