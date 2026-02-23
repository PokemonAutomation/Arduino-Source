/*  Legendary Run Away
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_LegendaryRunAway_H
#define PokemonAutomation_PokemonFRLG_LegendaryRunAway_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class LegendaryRunAway_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LegendaryRunAway_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class LegendaryRunAway : public SingleSwitchProgramInstance{
public:
    LegendaryRunAway();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class Target{
        hooh,
        lugia,
    };
    EnumDropdownOption<Target> TARGET;

    void reset_hooh(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void reset_lugia(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif


