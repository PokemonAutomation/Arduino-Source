/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_EggHatcher_H
#define PokemonAutomation_PokemonSV_EggHatcher_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class EggHatcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggHatcher_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class EggHatcher : public SingleSwitchProgramInstance{
public:
    EggHatcher();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Start at box system, where party is empty, the program will:
    // - load one column to party
    // - hatch
    // - return to box system and offline paty
    // Repeat for all six columns of a box
    void hatch_one_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    enum class StartLocation{
        ZeroGateFlyingSpot,
        AnywhereOnRide,
        AnywhereOffRide,
    };
    EnumDropdownOption<StartLocation> START_LOCATION;

    SimpleIntegerOption<uint8_t> BOXES;
    BooleanCheckBoxOption HAS_CLONE_RIDE_POKEMON;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif
