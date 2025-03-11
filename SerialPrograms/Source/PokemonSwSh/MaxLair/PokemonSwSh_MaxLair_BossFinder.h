/*  Max Lair (Boss Finder)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_BossFinder_H
#define PokemonAutomation_PokemonSwSh_MaxLair_BossFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "Options/PokemonSwSh_MaxLair_Options.h"
#include "Options/PokemonSwSh_MaxLair_Options_Consoles.h"
#include "Options/PokemonSwSh_MaxLair_Options_Hosting.h"
#include "Options/PokemonSwSh_MaxLair_Options_BossAction.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MaxLairBossFinder_Descriptor : public MultiSwitchProgramDescriptor{
public:
    MaxLairBossFinder_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MaxLairBossFinder : public MultiSwitchProgramInstance{
public:
    MaxLairBossFinder();

    virtual std::string check_validity() const override;
    virtual void update_active_consoles(size_t switch_count) override;
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;


private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    MaxLairInternal::Consoles CONSOLES;
    MaxLairInternal::BossActionTable BOSS_LIST;
    MaxLairInternal::HostingSettings HOSTING;

    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
