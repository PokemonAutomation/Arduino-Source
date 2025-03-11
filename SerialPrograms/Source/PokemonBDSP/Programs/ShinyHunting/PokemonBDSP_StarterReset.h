/*  Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StarterReset_H
#define PokemonAutomation_PokemonBDSP_StarterReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_NameSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class StarterReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StarterReset_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class StarterReset : public SingleSwitchProgramInstance{
public:
    StarterReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    void run_trigger(ProControllerContext& context) const;
    bool find_encounter(SingleSwitchProgramEnvironment& env) const;

private:
//    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    StringSelectDatabase STARTER_DATABASE;
    StringSelectOption STARTER;

    BooleanCheckBoxOption USE_SOUND_DETECTION;
    BooleanCheckBoxOption VIDEO_ON_SHINY;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
