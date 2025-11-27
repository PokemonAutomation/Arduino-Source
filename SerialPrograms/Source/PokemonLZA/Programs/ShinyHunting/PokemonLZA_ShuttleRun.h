/*  Shiny Hunt - Shuttle Run
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Shuttle_Run_H
#define PokemonAutomation_PokemonLZA_Shuttle_Run_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_ShuttleRun_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_ShuttleRun_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_ShuttleRun : public SingleSwitchProgramInstance {
public:
    ShinyHunt_ShuttleRun();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    enum class Routes{
        KLEFKI,
        KLEFKI_INKAY_GOOMY,
        LITWICK,
        SKRELP,
        SKRELP_INKAY,
        SKRELP_ARIADOS,
        SCRAGGY,
        WILD_ZONE_19,
        FIRE_STARTERS,
        DRATINI,
        CUSTOMISED_MACRO = 255,
    };

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    MillisecondsOption DURATION;
    EnumDropdownOption<Routes> ROUTE;
    ShinySoundDetectedActionOption SHINY_DETECTED;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
