/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SelfBoxTrade_H
#define PokemonAutomation_PokemonBDSP_SelfBoxTrade_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class SelfBoxTrade_Descriptor : public MultiSwitchProgramDescriptor{
public:
    SelfBoxTrade_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class SelfBoxTrade : public MultiSwitchProgramInstance{
public:
    SelfBoxTrade();
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
