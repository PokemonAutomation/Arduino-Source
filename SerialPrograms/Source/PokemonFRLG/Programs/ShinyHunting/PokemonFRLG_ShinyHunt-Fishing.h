/*  Shiny Hunt - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#ifndef PokemonAutomation_PokemonFRLG_ShinyHuntFishing_H
#define PokemonAutomation_PokemonFRLG_ShinyHuntFishing_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {

class ShinyHuntFishing_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHuntFishing_Descriptor();

    struct Stats;
    std::unique_ptr<StatsTracker> make_stats() const override;
};

class ShinyHuntFishing : public SingleSwitchProgramInstance {
public:
    ShinyHuntFishing();

    void program(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
        ) override;

    // Disable strict border detection
    void start_program_border_check(
        VideoStream&,
        FeedbackType
        ) override {}

private:
    SimpleIntegerOption<int> BITE_WAIT_MS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;

    EventNotificationsOption NOTIFICATIONS;
};

}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation

#endif