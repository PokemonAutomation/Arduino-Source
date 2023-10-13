/*  Gimmighoul Chest Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GimmighoulChestFarm_H
#define PokemonAutomation_PokemonSwSh_GimmighoulChestFarm_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

class GimmighoulChestFarmer_Descriptor : public SingleSwitchProgramDescriptor {
public:
    GimmighoulChestFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class GimmighoulChestFarmer : public SingleSwitchProgramInstance {
public:
    GimmighoulChestFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    enum class StartLocation {
        FlyPoint,
        InFrontOfChest,
    };

    SimpleIntegerOption<uint32_t> PP;
    EnumDropdownOption<StartLocation> START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



