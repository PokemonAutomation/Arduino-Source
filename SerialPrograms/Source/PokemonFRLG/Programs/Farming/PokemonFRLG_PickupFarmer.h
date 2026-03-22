/*  Pickup Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PickupFarmer_H
#define PokemonAutomation_PokemonFRLG_PickupFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class PickupFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PickupFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class PickupFarmer : public SingleSwitchProgramInstance{
public:
    PickupFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class GameLocation{
        route1,
        route22
    };

    enum class TravelMethod{
        fly,
        teleport
    };


    EnumDropdownOption<GameLocation> GAME_LOCATION;
    EnumDropdownOption<TravelMethod> TRAVEL_METHOD;

    SimpleIntegerOption<uint64_t> MAX_ENCOUNTERS;
    SimpleIntegerOption<uint64_t> BATTLES_PER_ITEM_CHECK;

    BooleanCheckBoxOption PREVENT_EVOLUTION;
    BooleanCheckBoxOption STOP_ON_MOVE_LEARN;
    BooleanCheckBoxOption IGNORE_SHINIES;

    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



