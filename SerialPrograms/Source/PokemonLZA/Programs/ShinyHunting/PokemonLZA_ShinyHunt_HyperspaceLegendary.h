/*  Shiny Hunt - Hyperspace Legendary
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Shiny hunt legendary pokemon in Hyperspace.
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_HyperspaceLegendary_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_HyperspaceLegendary_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_HyperspaceLegendary_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_HyperspaceLegendary_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_HyperspaceLegendary : public SingleSwitchProgramInstance{
public:
    ShinyHunt_HyperspaceLegendary();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    enum class Legendary{
        VIRIZION
    };

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    ShinySoundDetectedActionOption SHINY_DETECTED;
    EnumDropdownOption<Legendary> LEGENDARY;
    SimpleIntegerOption<uint16_t> MIN_CALORIE_TO_CATCH;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
