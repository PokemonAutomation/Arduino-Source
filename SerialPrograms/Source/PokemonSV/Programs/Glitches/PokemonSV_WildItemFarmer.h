/*  Wild Item Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_WildItemFarmer_H
#define PokemonAutomation_PokemonSwSh_WildItemFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class NormalBattleMenuWatcher;


class WildItemFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WildItemFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class WildItemFarmer : public SingleSwitchProgramInstance{
public:
    WildItemFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void refresh_pp(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool verify_item_held(SingleSwitchProgramEnvironment& env, ProControllerContext& context, NormalBattleMenuWatcher& battle_menu);
    void run_program(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    SimpleIntegerOption<uint16_t> ITEMS_TO_CLONE;
//    IntegerEnumDropdownOption TRICK_MOVE_SLOT;
    SimpleIntegerOption<uint8_t> INITIAL_TRICK_PP;
    BooleanCheckBoxOption VERIFY_ITEM_CLONED;
    BooleanCheckBoxOption ENABLE_FORWARD_RUN;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



