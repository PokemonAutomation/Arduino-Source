/*  Shiny Hunt - Sewer Whirlipede Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Sewer_Whirlipede_Runner_H
#define PokemonAutomation_PokemonLZA_Sewer_Whirlipede_Runner_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_SewerWhirlipedeRunner_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_SewerWhirlipedeRunner_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_SewerWhirlipedeRunner : public SingleSwitchProgramInstance {
public:
    ShinyHunt_SewerWhirlipedeRunner();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    MillisecondsOption DURATION;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
