/*  Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StarterReset_H
#define PokemonAutomation_PokemonFRLG_StarterReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class StarterReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StarterReset_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class StarterReset : public SingleSwitchProgramInstance{
public:
    StarterReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    void obtain_starter(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY_STARTER;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



