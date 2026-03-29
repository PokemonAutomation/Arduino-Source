/*  RS Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_AudioStarterReset_H
#define PokemonAutomation_PokemonRSE_AudioStarterReset_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

class AudioStarterReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AudioStarterReset_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class AudioStarterReset : public SingleSwitchProgramInstance{
public:
    AudioStarterReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    enum class Target{
        treecko,
        torchic,
        mudkip,
    };
    EnumDropdownOption<Target> TARGET;

    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY_POOCH;
    EventNotificationOption NOTIFICATION_SHINY_STARTER;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



