/*  Shiny Hunt - Fixed Point
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyHuntFlagPin_H
#define PokemonAutomation_PokemonLA_ShinyHuntFlagPin_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Options/PokemonLA_TravelLocation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ShinyHuntFlagPin_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntFlagPin_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntFlagPin : public SingleSwitchProgramInstance{
public:
    ShinyHuntFlagPin();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void run_iteration(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        bool& fresh_from_reset
    );

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    WildTravelLocationOption TRAVEL_LOCATION;

    SimpleIntegerOption<uint16_t> ENROUTE_DISTANCE;

    ResetMethodOption RESET_METHOD;

    OverworldShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    OverworldShinyDetectedActionOption SHINY_DETECTED_DESTINATION;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> STOP_DISTANCE;
    FloatingPointOption FLAG_REACHED_DELAY;
    SimpleIntegerOption<uint64_t> NAVIGATION_TIMEOUT;
};





}
}
}
#endif
