/*  BDSP AutoStory - Segment 09
 *  From after acquiring the eighth badge until defeating the Elite Four and Champion.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_09.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_09::name()       const{ return "09: Eighth Badge to Champion"; }
std::string AutoStory_Segment_09::start_text() const{ return "Start: Eighth Gym Badge acquired (Sunyshore City)."; }
std::string AutoStory_Segment_09::end_text()   const{ return "End: Elite Four and Champion defeated, credits roll."; }

void AutoStory_Segment_09::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_022().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_022::name()       const{ return "022 - " + AutoStory_Segment_09().name(); }
std::string AutoStory_Checkpoint_022::start_text() const{ return "Eighth Gym Badge acquired (Sunyshore City)."; }
std::string AutoStory_Checkpoint_022::end_text()   const{ return "Elite Four and Champion defeated, credits roll."; }

void AutoStory_Checkpoint_022::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_022(env, context, options, stats);
}


void checkpoint_022(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 09 gameplay logic
        },
        true
    );
}


}
}
}
