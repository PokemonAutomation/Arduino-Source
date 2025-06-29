/*  LGPE Daily Item Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_DailyItemFarmer_H
#define PokemonAutomation_PokemonLGPE_DailyItemFarmer_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

class DailyItemFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DailyItemFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class DailyItemFarmer : public SingleSwitchProgramInstance{
public:
    DailyItemFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    void start_local_trade(SingleSwitchProgramEnvironment& env, JoyconContext& context);

    SimpleIntegerOption<uint32_t> ATTEMPTS;

    enum class LinkCode{
        Pikachu,
        Eevee,
        Bulbasaur,
        Charmander,
        Squirtle,
        Pidgey,
        Caterpie,
        Rattata,
        Jigglypuff,
        Diglett,
    };
    EnumDropdownOption<LinkCode> LINK_CODE;
    
    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif



