/*  E Shiny Mew
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_ShinyHuntMew_H
#define PokemonAutomation_PokemonRSE_ShinyHuntMew_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

class ShinyHuntMew_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntMew_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ShinyHuntMew : public SingleSwitchProgramInstance{
public:
    ShinyHuntMew();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;

    MillisecondsOption MEW_WAIT_TIME;
    MillisecondsOption DOOR_TO_GRASS_TIME;
    MillisecondsOption RIGHT_GRASS_1_TIME;
    MillisecondsOption UP_GRASS_1_TIME;
    MillisecondsOption RIGHT_GRASS_2_TIME;
    MillisecondsOption FACE_UP_TIME;

    void enter_mew(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void exit_mew(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

};

}
}
}
#endif

