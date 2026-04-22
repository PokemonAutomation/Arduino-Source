/*  SID Helper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_SidHelper_H
#define PokemonAutomation_PokemonFRLG_SidHelper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class SidHelper_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SidHelper_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class SidHelper : public SingleSwitchProgramInstance {
public:
    SidHelper();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override {}

private:
    SimpleIntegerOption<uint32_t> TARGET_ADVANCES;
    SimpleIntegerOption<uint8_t> NUM_CANDIDATES;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_SIDS;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
     



#endif
