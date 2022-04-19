/*  Alpha Froslass Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_FroslassFinder_H
#define PokemonAutomation_PokemonLA_FroslassFinder_H
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class FroslassFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    FroslassFinder_Descriptor();
};

class FroslassFinder : public SingleSwitchProgramInstance{
public:
    FroslassFinder(const FroslassFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    class Stats;
    class RunRoute;

    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    ShinyDetectedActionOption SHINY_DETECTED;
    BooleanCheckBoxOption SKIP_PATH_SHINY;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif

