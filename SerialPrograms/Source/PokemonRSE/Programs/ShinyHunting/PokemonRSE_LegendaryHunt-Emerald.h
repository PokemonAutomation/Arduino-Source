/*  Legendary Hunt - Emerald
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_LegendaryHuntEmerald_H
#define PokemonAutomation_PokemonRSE_LegendaryHuntEmerald_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

class LegendaryHuntEmerald_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LegendaryHuntEmerald_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class LegendaryHuntEmerald : public SingleSwitchProgramInstance{
public:
    LegendaryHuntEmerald();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    enum class Target{
        regis,
        groudon,
        kyogre,
        hooh,
        lugia,
    };
    EnumDropdownOption<Target> TARGET;

    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    //For easier testing after release
    //regis are up/down only and don't really need
    SectionDividerOption m_advanced_options;
    MillisecondsOption KYOGRE_RIGHT_FIRST;
    MillisecondsOption KYOGRE_UP_FIRST;
    MillisecondsOption KYOGRE_RIGHT_SECOND;
    MillisecondsOption HOOH_UP_DOWN;
    MillisecondsOption HOOH_LEFT_RIGHT;
    MillisecondsOption LUGIA_UP_DOWN0;
    MillisecondsOption LUGIA_LEFT_RIGHT0;

    void reset_regi(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void reset_groudon(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void reset_kyogre(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void reset_hooh(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void reset_lugia(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};

}
}
}
#endif

