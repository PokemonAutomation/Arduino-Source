/*  Self Touch Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_SelfTouchTrade_H
#define PokemonAutomation_PokemonLA_SelfTouchTrade_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "PokemonLA/Options/PokemonLA_TradeCountTable.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


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
    bool trade_one(
        MultiSwitchProgramEnvironment& env, CancellableScope& scope,
        std::map<std::string, SimpleIntegerCell<uint8_t>*>& trades_left
    );
    bool move_to_next(Logger& logger, ProControllerContext& host, uint8_t& row, uint8_t& col);

private:
    OCR::LanguageOCROption LANGUAGE;

    enum class HostingSwitch{
        Switch0,
        Switch1
    };
    EnumDropdownOption<HostingSwitch> HOSTING_SWITCH;

    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    TradeCountTable TRADE_COUNTS;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
