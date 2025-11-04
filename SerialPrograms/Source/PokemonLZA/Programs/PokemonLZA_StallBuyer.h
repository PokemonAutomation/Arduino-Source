/*  Stall Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_StallBuyer_H
#define PokemonAutomation_PokemonLZA_StallBuyer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



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
    enum class ItemPosition{
        FirstItem,
        SecondItem,
        ThirdItem,
        FourthItem,
        FifthItem,
        SixthItem,
        SeventhItem
    };
    EnumDropdownOption<ItemPosition> ITEM_POSITION;
    SimpleIntegerOption<uint8_t> NUM_ITEM;
    SimpleIntegerOption<uint16_t> NUM_PURCHASE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
