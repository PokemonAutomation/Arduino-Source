/*  Shiny Hunt - Fixed Point
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyHuntFlagPin_H
#define PokemonAutomation_PokemonLA_ShinyHuntFlagPin_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Options/PokemonLA_TravelLocation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ShinyHuntFlagPin_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyHuntFlagPin_Descriptor();
};


class ShinyHuntFlagPin : public SingleSwitchProgramInstance{
public:
    ShinyHuntFlagPin(const ShinyHuntFlagPin_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env);

private:
    class Stats;

    TravelLocationOption TRAVEL_LOCATION;

    SimpleIntegerOption<uint16_t> STOP_DISTANCE;
    FloatingPointOption FLAG_REACHED_DELAY;
    SimpleIntegerOption<uint64_t> NAVIGATION_TIMEOUT;

    ShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
