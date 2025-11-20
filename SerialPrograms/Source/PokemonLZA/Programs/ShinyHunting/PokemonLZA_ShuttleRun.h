/*  Shiny Hunt - Shuttle Run
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Shuttle_Run_H
#define PokemonAutomation_PokemonLZA_Shuttle_Run_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

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

private:
    MillisecondsOption DURATION;
    // MillisecondsOption RUN_FORWARD_DURATION;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
