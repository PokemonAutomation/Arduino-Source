/*  Shiny Hunt - Wild Zone Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_WildZoneEntrance_H
#define PokemonAutomation_PokemonLZA_WildZoneEntrance_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"
#include "PokemonLZA/Programs/PokemonLZA_Locations.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class WildZoneOption : public EnumDropdownOption<WildZone>{
public:
    WildZoneOption();
};


class ShinyHunt_WildZoneEntrance_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_WildZoneEntrance_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_WildZoneEntrance : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ShinyHunt_WildZoneEntrance();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;

    WildZoneOption WILD_ZONE;
    IntegerEnumDropdownOption MOVEMENT;

    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    MillisecondsOption WALK_TIME_IN_ZONE;
    BooleanCheckBoxOption RUNNING;

    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    // Record when we should zoom out the map for flyable fast travel icon detection on map during fast traveling.
    bool to_max_zoom_level_on_map = true;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
