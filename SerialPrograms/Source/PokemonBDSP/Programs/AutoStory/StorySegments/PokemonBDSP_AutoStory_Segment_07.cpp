/*  BDSP AutoStory - Segment 07
 *  From after acquiring the sixth badge until after acquiring the seventh badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_07.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_07::name()       const{ return "07: Sixth Badge to Seventh Badge"; }
std::string AutoStory_Segment_07::start_text() const{ return "Start: Sixth Gym Badge acquired (Canalave City)."; }
std::string AutoStory_Segment_07::end_text()   const{ return "End: Seventh Gym Badge acquired (Snowpoint City)."; }

void AutoStory_Segment_07::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_020().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_020::name()       const{ return "007 - " + AutoStory_Segment_07().name(); }
std::string AutoStory_Checkpoint_020::start_text() const{ return "Sixth Gym Badge acquired (Canalave City)."; }
std::string AutoStory_Checkpoint_020::end_text()   const{ return "Seventh Gym Badge acquired (Snowpoint City)."; }

void AutoStory_Checkpoint_020::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_020(env, context, options, stats);
}


void checkpoint_020(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 07 gameplay logic
        }
    );
}


}
}
}
