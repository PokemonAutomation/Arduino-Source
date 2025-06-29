/*  E Shiny Deoxys
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_ShinyHuntDeoxys_H
#define PokemonAutomation_PokemonRSE_ShinyHuntDeoxys_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

class ShinyHuntDeoxys_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntDeoxys_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ShinyHuntDeoxys : public SingleSwitchProgramInstance{
public:
    ShinyHuntDeoxys();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class StartPos{
        boat,
        rock_unsolved,
        rock_solved,
    };
    EnumDropdownOption<StartPos> STARTPOS;

    MillisecondsOption WALK_UP_DOWN_TIME0;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    void solve_puzzle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};

}
}
}
#endif

