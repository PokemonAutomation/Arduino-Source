/*  Self Touch Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SelfTouchTrade_H
#define PokemonAutomation_PokemonBDSP_SelfTouchTrade_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class SelfTouchTrade_Descriptor : public MultiSwitchProgramDescriptor{
public:
    SelfTouchTrade_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class SelfTouchTrade : public MultiSwitchProgramInstance{
public:
    SelfTouchTrade();
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    enum class HostingSwitch{
        Switch0,
        Switch1
    };
    EnumDropdownOption<HostingSwitch> HOSTING_SWITCH;
    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
