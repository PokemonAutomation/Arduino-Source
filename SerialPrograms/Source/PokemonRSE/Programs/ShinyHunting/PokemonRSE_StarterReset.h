/*  RS Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_StarterReset_H
#define PokemonAutomation_PokemonRSE_StarterReset_H

//#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

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
    enum class Target{
        treecko,
        torchic,
        mudkip,
    };
    EnumDropdownOption<Target> TARGET;

//    MillisecondsOption STARTER_WAIT0;

    EventNotificationOption NOTIFICATION_SHINY_STARTER;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



