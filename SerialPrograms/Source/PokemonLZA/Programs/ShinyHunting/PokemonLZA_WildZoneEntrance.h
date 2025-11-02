/*  Shiny Hunt - Wild Zone Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_WildZoneEntrance_H
#define PokemonAutomation_PokemonLZA_WildZoneEntrance_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_WildZoneEntrance_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_WildZoneEntrance_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_WildZoneEntrance : public SingleSwitchProgramInstance {
public:
    ShinyHunt_WildZoneEntrance();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    MillisecondsOption WALK_IN_ZONE;

    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
