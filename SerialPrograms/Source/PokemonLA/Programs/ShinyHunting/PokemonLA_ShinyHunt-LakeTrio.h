/*  Shiny Hunt - Lake Trio
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyHuntLakeTrio_H
#define PokemonAutomation_PokemonLA_ShinyHuntLakeTrio_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
//#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ShinyHuntLakeTrio_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntLakeTrio_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntLakeTrio : public SingleSwitchProgramInstance{
public:
    ShinyHuntLakeTrio();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
//    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::EncounterBotLanguage LANGUAGE;

    BooleanCheckBoxOption VIDEO_ON_SHINY;

//    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
