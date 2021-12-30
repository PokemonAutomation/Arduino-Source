/*  Max Lair (Strong-Boss Mode)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_StrongBoss_H
#define PokemonAutomation_PokemonSwSh_MaxLair_StrongBoss_H

#include "CommonFramework/Options/FloatingPointOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "Options/PokemonSwSh_MaxLair_Options.h"
#include "Options/PokemonSwSh_MaxLair_Options_Consoles.h"
#include "Options/PokemonSwSh_MaxLair_Options_Hosting.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MaxLairStrongBoss_Descriptor : public MultiSwitchProgramDescriptor{
public:
    MaxLairStrongBoss_Descriptor();
};


class MaxLairStrongBoss : public MultiSwitchProgramInstance{
public:
    enum class StopCondition{
        STOP_ON_SHINY_LEGENDARY,
        STOP_ON_NOTHING,
    };

public:
    MaxLairStrongBoss(const MaxLairStrongBoss_Descriptor& descriptor);

    virtual QString check_validity() const override;
    virtual void update_active_consoles() override;

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(MultiSwitchProgramEnvironment& env) override;


private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    FloatingPointOption MIN_WIN_RATE;
    MaxLairInternal::BossSlot BOSS_SLOT;

    MaxLairInternal::Consoles CONSOLES;
    MaxLairInternal::HostingSettings HOSTING;

    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
