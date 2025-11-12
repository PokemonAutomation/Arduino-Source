/*  Stall Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_StallBuyer_H
#define PokemonAutomation_PokemonLZA_StallBuyer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class StallBuyerRow : public StaticTableRow{
public:
    StallBuyerRow(int index, std::string&& ordinal);

    LabelCellOption item;
    SimpleIntegerCell<uint16_t> quantity;
    std::string ordinal;
    int index;
};

class StallBuyerTable : public StaticTableOption {
public:
    StallBuyerTable();
    virtual std::vector<std::string> make_header() const;
};



class StallBuyer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StallBuyer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class StallBuyer : public SingleSwitchProgramInstance{
public:
    StallBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void make_purchase(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        StallBuyerRow& stall_buyer_row,
        uint16_t& purchases
    );

private:
    StallBuyerTable NUM_PURCHASE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
