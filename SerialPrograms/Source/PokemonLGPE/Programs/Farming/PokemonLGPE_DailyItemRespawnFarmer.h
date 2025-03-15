/*  LGPE Daily Item Respawn Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_DailyItemRespawnFarmer_H
#define PokemonAutomation_PokemonLGPE_DailyItemRespawnFarmer_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

class DailyItemRespawnFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DailyItemRespawnFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class DailyItemRespawnFarmer : public SingleSwitchProgramInstance{
public:
    DailyItemRespawnFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
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
    
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif



