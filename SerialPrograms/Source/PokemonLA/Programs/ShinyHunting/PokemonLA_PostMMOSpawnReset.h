/*  Post MMO Spawn Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_PostMMOSpawnReset_H
#define PokemonAutomation_PokemonLA_PostMMOSpawnReset_H

#include "Common/Cpp/Options/TimeExpressionOption.h"
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
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    TimeExpressionOption<int16_t> TURN_DURATION;
    TimeExpressionOption<uint16_t> FORWARD_DURATION;
    TimeExpressionOption<uint16_t> WAIT_DURATION;

    ShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
