/*  Shiny Hunt - Hyperspace Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Hunt for shinies in Hyperspace Wild Zone using two methods:
 *  1. Fly Spot Reset - Repeatedly fast travel to reset spawns
 *  2. Shuttle Run - Run forward and backward with customizable times
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_HyperspaceHunter_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_HyperspaceHunter_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_HyperspaceHunter_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_HyperspaceHunter_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_HyperspaceHunter : public SingleSwitchProgramInstance{
public:
    ShinyHunt_HyperspaceHunter();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    enum class HuntMethod{
        FLY_SPOT_RESET,
        SHUTTLE_RUN,
    };

private:
    void use_fly_spot_reset(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void use_shuttle_run(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    ShinySoundDetectedActionOption SHINY_DETECTED;
    EnumDropdownOption<HuntMethod> HUNT_METHOD;

    SimpleIntegerOption<uint64_t> NUM_RESETS;
    SimpleIntegerOption<uint16_t> MIN_CALORIE_REMAINING;

    // Shuttle Run only options
    MillisecondsOption FORWARD_RUN_TIME;
    MillisecondsOption BACKWARD_RUN_TIME;
    MillisecondsOption FORWARD_RUN_ROLL_TIME;
    MillisecondsOption BACKWARD_RUN_ROLL_TIME;
    MillisecondsOption SHINY_CHECK_RUN_TIME;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
