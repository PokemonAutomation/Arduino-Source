/*  Shiny Hunt - Sewer Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Sewer_Hunter_H
#define PokemonAutomation_PokemonLZA_Sewer_Hunter_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_SewerHunter_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_SewerHunter_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_SewerHunter : public SingleSwitchProgramInstance {
public:
    ShinyHunt_SewerHunter();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    enum class Route{
        KLEFKI,
        KLEFKI_INKAY_GOOMY,
        LITWICK,
        LITWICK_SKRELP,
        SKRELP,
        SKRELP_INKAY,
        ARIADOS
    };

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    MillisecondsOption DURATION;
    EnumDropdownOption<Route> ROUTE;
    ShinySoundDetectedActionOption SHINY_DETECTED;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
