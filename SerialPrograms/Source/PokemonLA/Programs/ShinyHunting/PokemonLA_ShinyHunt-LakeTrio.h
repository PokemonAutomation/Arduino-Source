/*  Shiny Hunt - Lake Trio
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyHuntLakeTrio_H
#define PokemonAutomation_PokemonLA_ShinyHuntLakeTrio_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
//#include "PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ShinyHuntLakeTrio_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntLakeTrio_Descriptor();
};


class ShinyHuntLakeTrio : public SingleSwitchProgramInstance{
public:
    ShinyHuntLakeTrio(const ShinyHuntLakeTrio_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context) override;


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
