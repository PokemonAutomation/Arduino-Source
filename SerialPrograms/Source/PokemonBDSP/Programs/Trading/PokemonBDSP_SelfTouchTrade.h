/*  Self Touch Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SelfTouchTrade_H
#define PokemonAutomation_PokemonBDSP_SelfTouchTrade_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class SelfTouchTrade_Descriptor : public MultiSwitchProgramDescriptor{
public:
    SelfTouchTrade_Descriptor();
};


class SelfTouchTrade : public MultiSwitchProgramInstance{
public:
    SelfTouchTrade(const SelfTouchTrade_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(MultiSwitchProgramEnvironment& env) override;

private:
    struct Stats;

private:
    EnumDropdownOption HOSTING_SWITCH;
    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
