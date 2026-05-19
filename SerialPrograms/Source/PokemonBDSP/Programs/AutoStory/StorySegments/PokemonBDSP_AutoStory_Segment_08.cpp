/*  BDSP AutoStory - Segment 08
 *  From after acquiring the seventh badge until after acquiring the eighth badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_08.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  Legendary encounter (from Routes/Legendary_Segment.cpp)
// ---------------------------------------------------------------------------

static void approach_legendary(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_wait(context, 2000ms);
    pbf_press_button(context, BUTTON_A, 100ms, 300ms);
    pbf_wait(context, 2000ms);
}

static void enter_legendary_battle(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 3000ms);
}

static void weaken_legendary(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 1500ms);
    pbf_press_dpad(context, DPAD_UP, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 2000ms);
}

static void throw_pokeball_at_legendary(VideoStream& /*stream*/, ProControllerContext& context){
    for (int i = 0; i < 3; i++){
        pbf_press_button(context, BUTTON_A, 100ms, 100ms);
        pbf_wait(context, 1000ms);
        pbf_press_dpad(context, DPAD_DOWN, 100ms, 100ms);
        pbf_press_button(context, BUTTON_A, 100ms, 100ms);
        pbf_wait(context, 2500ms);
    }
}

static void confirm_legendary_catch(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_wait(context, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 2000ms);
}

static void exit_legendary_encounter(VideoStream& /*stream*/, ProControllerContext& context){
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_wait(context, 2000ms);
}


// ---------------------------------------------------------------------------
//  Segment / Checkpoint classes
// ---------------------------------------------------------------------------

std::string AutoStory_Segment_08::name()       const{ return "08: Seventh Badge to Eighth Badge"; }
std::string AutoStory_Segment_08::start_text() const{ return "Start: Seventh Gym Badge acquired (Snowpoint City)."; }
std::string AutoStory_Segment_08::end_text()   const{ return "End: Eighth Gym Badge acquired (Sunyshore City)."; }

void AutoStory_Segment_08::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_021().run_checkpoint(env, context, options, stats);
    checkpoint_021_legendary(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_021::name()       const{ return "008 - " + AutoStory_Segment_08().name(); }
std::string AutoStory_Checkpoint_021::start_text() const{ return "Seventh Gym Badge acquired (Snowpoint City)."; }
std::string AutoStory_Checkpoint_021::end_text()   const{ return "Eighth Gym Badge acquired (Sunyshore City)."; }

void AutoStory_Checkpoint_021::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_021(env, context, options, stats);
}


void checkpoint_021(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 08 gameplay logic (seventh badge area)
        }
    );
}

void checkpoint_021_legendary(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            approach_legendary(env.console, context);
            enter_legendary_battle(env.console, context);
            weaken_legendary(env.console, context);
            throw_pokeball_at_legendary(env.console, context);
            confirm_legendary_catch(env.console, context);
            exit_legendary_encounter(env.console, context);
        }
    );
}


}
}
}
