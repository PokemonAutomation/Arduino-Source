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
    WildZoneOption()
        : EnumDropdownOption<WildZone>(
            "<b>Wild Zone:</b>",
            {
                {WildZone::WILD_ZONE_1,  "wild-zone-1",  "Wild Zone 1"},
                {WildZone::WILD_ZONE_2,  "wild-zone-2",  "Wild Zone 2"},
                {WildZone::WILD_ZONE_3,  "wild-zone-3",  "Wild Zone 3"},
                {WildZone::WILD_ZONE_4,  "wild-zone-4",  "Wild Zone 4"},
                {WildZone::WILD_ZONE_5,  "wild-zone-5",  "Wild Zone 5"},
                {WildZone::WILD_ZONE_6,  "wild-zone-6",  "Wild Zone 6"},
                {WildZone::WILD_ZONE_7,  "wild-zone-7",  "Wild Zone 7"},
                {WildZone::WILD_ZONE_8,  "wild-zone-8",  "Wild Zone 8"},
                {WildZone::WILD_ZONE_9,  "wild-zone-9",  "Wild Zone 9"},
                {WildZone::WILD_ZONE_10, "wild-zone-10", "Wild Zone 10"},
                {WildZone::WILD_ZONE_11, "wild-zone-11", "Wild Zone 11"},
                {WildZone::WILD_ZONE_12, "wild-zone-12", "Wild Zone 12"},
                {WildZone::WILD_ZONE_13, "wild-zone-13", "Wild Zone 13"},
                {WildZone::WILD_ZONE_14, "wild-zone-14", "Wild Zone 14"},
                {WildZone::WILD_ZONE_15, "wild-zone-15", "Wild Zone 15"},
                {WildZone::WILD_ZONE_16, "wild-zone-16", "Wild Zone 16"},
                {WildZone::WILD_ZONE_17, "wild-zone-17", "Wild Zone 17"},
                {WildZone::WILD_ZONE_18, "wild-zone-18", "Wild Zone 18"},
                {WildZone::WILD_ZONE_19, "wild-zone-19", "Wild Zone 19"},
                {WildZone::WILD_ZONE_20, "wild-zone-20", "Wild Zone 20"},
            },
            LockMode::UNLOCK_WHILE_RUNNING,
            WildZone::WILD_ZONE_1
        )
    {}
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
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
