/*  Shiny Hunt - Fly Spot Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Repeatedly fast travel to the same location to shiny hunt surrounding pokemon.
 *  This is mostly for a single spawner: alpha Swirlix near Nouveau Cafe (Truck No. 3).
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_FlySpotReset_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_FlySpotReset_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_FlySpotReset_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_FlySpotReset_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_FlySpotReset : public SingleSwitchProgramInstance{
public:
    ShinyHunt_FlySpotReset();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    enum class Route{
        NO_MOVEMENT,
        HYPERSPACE_WILD_ZONE,
        WILD_ZONE_19,
        ALPHA_PIDGEY,
        ALPHA_PIKACHU,
        CUSTOMISED_MACRO = 255,
    };

    private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    ShinySoundDetectedActionOption SHINY_DETECTED;
    EnumDropdownOption<Route> ROUTE;
    SimpleIntegerOption<uint64_t> NUM_RESETS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
