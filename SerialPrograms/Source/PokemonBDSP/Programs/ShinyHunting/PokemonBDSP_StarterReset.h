/*  Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StarterReset_H
#define PokemonAutomation_PokemonBDSP_StarterReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_NameSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class StarterReset_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    StarterReset_Descriptor();
};


class StarterReset : public SingleSwitchProgramInstance{
public:
    StarterReset(const StarterReset_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;


private:
    struct Stats;
    void run_trigger(const BotBaseContext& context) const;
    bool find_encounter(SingleSwitchProgramEnvironment& env) const;

private:
//    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    Pokemon::PokemonNameSelect STARTER;

    BooleanCheckBoxOption VIDEO_ON_SHINY;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
