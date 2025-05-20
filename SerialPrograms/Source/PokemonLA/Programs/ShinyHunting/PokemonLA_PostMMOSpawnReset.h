/*  Post MMO Spawn Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_PostMMOSpawnReset_H
#define PokemonAutomation_PokemonLA_PostMMOSpawnReset_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class PostMMOSpawnReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PostMMOSpawnReset_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class PostMMOSpawnReset : public SingleSwitchProgramInstance{
public:
    PostMMOSpawnReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    MillisecondsOption TURN_DURATION0;
    MillisecondsOption FORWARD_DURATION0;
    MillisecondsOption WAIT_DURATION0;

    OverworldShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
