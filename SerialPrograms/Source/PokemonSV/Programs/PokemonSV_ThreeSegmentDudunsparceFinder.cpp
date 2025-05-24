/*  Three-Segment Dudunsparce Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_ThreeSegmentDudunsparceFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


ThreeSegmentDudunsparceFinder_Descriptor::ThreeSegmentDudunsparceFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ThreeSegmentDudunsparceFinder",
        "Game Name", "Three-Segment Dudunsparce Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ThreeSegmentDudunsparceFinder.md",
        "<Description of this program>.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_ProController},
        FasterIfTickPrecise::NOT_FASTER
    )
{}
struct ThreeSegmentDudunsparceFinder_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Attempts"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_attempts;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> ThreeSegmentDudunsparceFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ThreeSegmentDudunsparceFinder::ThreeSegmentDudunsparceFinder()
    : GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ThreeSegmentDudunsparceFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    ThreeSegmentDudunsparceFinder_Descriptor::Stats& stats = env.current_stats<ThreeSegmentDudunsparceFinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 100);

    try{
        
    } catch(OperationFailedException&){
        stats.m_errors++;
        env.update_stats();
        throw;
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
