/*  Self Touch Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_SelfTouchTrade_H
#define PokemonAutomation_PokemonLA_SelfTouchTrade_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "PokemonLA/Options/PokemonLA_TradeCountTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class SelfTouchTrade_Descriptor : public MultiSwitchProgramDescriptor{
public:
    SelfTouchTrade_Descriptor();
};


class SelfTouchTrade : public MultiSwitchProgramInstance{
public:
    SelfTouchTrade(const SelfTouchTrade_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    bool trade_one(MultiSwitchProgramEnvironment& env, std::map<std::string, int>& trades_left);
    bool move_to_next(ConsoleHandle& host, uint8_t& row, uint8_t& col);

private:
    class Stats;

    OCR::LanguageOCR LANGUAGE;

    EnumDropdownOption HOSTING_SWITCH;
    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    TradeCountTableOption TRADE_COUNTS;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
