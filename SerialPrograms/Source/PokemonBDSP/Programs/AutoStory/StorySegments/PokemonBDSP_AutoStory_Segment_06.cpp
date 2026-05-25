/*  BDSP AutoStory - Segment 06
 *  From after acquiring the fifth badge until after acquiring the sixth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Utils/PokemonBDSP_AutoStory_Battle.h"
#include "PokemonBDSP_AutoStory_Segment_06.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string AutoStory_Segment_06::name()       const{ return "06: Fifth Badge to Sixth Badge"; }
std::string AutoStory_Segment_06::start_text() const{ return "Start: Fifth Gym Badge acquired (Veilstone City)."; }
std::string AutoStory_Segment_06::end_text()   const{ return "End: Sixth Gym Badge acquired (Canalave City)."; }

void AutoStory_Segment_06::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_019().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_019::name()       const{ return "019 - " + AutoStory_Segment_06().name(); }
std::string AutoStory_Checkpoint_019::start_text() const{ return "Fifth Gym Badge acquired (Veilstone City)."; }
std::string AutoStory_Checkpoint_019::end_text()   const{ return "Sixth Gym Badge acquired (Canalave City)."; }

void AutoStory_Checkpoint_019::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_019(env, context, options, stats);
}

static bool test_opening_menu(
    VideoStream& stream,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    stream.log("test_opening_menu: testing");

    open_menu(stream, context, MenuCursorPosition::BAG, 8);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    open_menu(stream, context, MenuCursorPosition::POKEDEX, 8);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    open_menu(stream, context, MenuCursorPosition::CARD, 8);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();

    return true;
}


void checkpoint_019(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 06 gameplay logic
            if (!test_opening_menu(env.console, context)){
                OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT, "test_opening_menu: transition not detected.", env.console);
            }
        }
    );
}


}
}
}
