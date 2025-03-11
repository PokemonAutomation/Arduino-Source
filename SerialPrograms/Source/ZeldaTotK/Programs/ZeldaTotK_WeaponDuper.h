/*  TotK Weapon Duper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ZeldaTotK_WeaponDuper_H
#define PokemonAutomation_ZeldaTotK_WeaponDuper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace ZeldaTotK{

class WeaponDuper_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WeaponDuper_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class WeaponDuper : public SingleSwitchProgramInstance{
public:
    WeaponDuper();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> ATTEMPTS;
    SimpleIntegerOption<uint32_t> WEAPON_MENU_BUTTON_COUNT;
    SimpleIntegerOption<uint32_t> DPAD_RIGHT_BUTTON_COUNT;
    SimpleIntegerOption<uint32_t> DPAD_DOWN_BUTTON_COUNT;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



