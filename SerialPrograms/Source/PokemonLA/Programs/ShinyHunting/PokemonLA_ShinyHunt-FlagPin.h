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


class ShinyHuntFlagPin_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntFlagPin_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntFlagPin : public SingleSwitchProgramInstance{
public:
    ShinyHuntFlagPin();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    TravelLocationOption TRAVEL_LOCATION;

    SimpleIntegerOption<uint16_t> ENROUTE_DISTANCE;

    EnumDropdownOption RESET_METHOD;

    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption SHINY_DETECTED_DESTINATION;

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
