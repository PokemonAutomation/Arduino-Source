/*  Alpha Froslass Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_FroslassFinder_H
#define PokemonAutomation_PokemonLA_FroslassFinder_H

#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class FroslassFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FroslassFinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class FroslassFinder : public SingleSwitchProgramInstance{
public:
    FroslassFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

private:
    class RunRoute;

    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    TimeExpressionOption<uint16_t> DASH_DURATION;

    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption SHINY_DETECTED_DESTINATION;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif

