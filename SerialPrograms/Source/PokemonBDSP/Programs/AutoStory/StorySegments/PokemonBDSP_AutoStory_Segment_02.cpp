/*  BDSP AutoStory - Segment 02
 *  From after acquiring the first badge until after acquiring the second badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_02.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    void roark_to_mars(VideoStream& stream, ProControllerContext& context) {

    }
    void mars_to_eterna_forest(VideoStream& stream, ProControllerContext& context) {

    }
    void eterna_forest(VideoStream& stream, ProControllerContext& context) {

    }
    void eterna_forest_exit_to_gym(VideoStream& stream, ProControllerContext& context) {

    }
static void gym_Puzzle_02(VideoStream& stream, ProControllerContext& context){

    //Upon entering the Gym.

    struct Move {
        DpadPosition dir;
        int count;
    };

    const Move sequence1[] = {
        {DPAD_UP, 5},
        {DPAD_LEFT, 2},
        {DPAD_UP, 9},
        {DPAD_RIGHT, 2},
        {DPAD_UP, 7},
        {DPAD_LEFT, 3},
        {DPAD_UP, 2},
        {DPAD_LEFT, 5},
        {DPAD_DOWN, 4},
    };

    const Move sequence2[] = {
        {DPAD_UP, 6},
        {DPAD_LEFT, 6},
        {DPAD_DOWN, 9},
        {DPAD_RIGHT, 2},
        {DPAD_DOWN, 5},
        {DPAD_LEFT, 2},
        {DPAD_DOWN, 4},
        {DPAD_LEFT, 2},
        {DPAD_DOWN, 6},
        {DPAD_RIGHT, 5},
    };

    const Move sequence3[] = {
        {DPAD_LEFT, 5},
        {DPAD_UP, 6},
        {DPAD_RIGHT, 2},
        {DPAD_UP, 5},
        {DPAD_RIGHT, 2},
        {DPAD_UP, 5},
        {DPAD_LEFT, 2},
        {DPAD_UP, 8},
        {DPAD_RIGHT, 7},
        {DPAD_DOWN, 2},
        {DPAD_RIGHT, 5},
        {DPAD_DOWN, 6},
        {DPAD_RIGHT, 2},
        {DPAD_DOWN, 5},
        {DPAD_LEFT, 2},
        {DPAD_DOWN, 4},
        {DPAD_RIGHT, 6},
        {DPAD_UP, 3},
        {DPAD_RIGHT, 2},
    };

    const Move sequence4[] = {
        {DPAD_LEFT, 2},
        {DPAD_DOWN, 4},
        {DPAD_LEFT, 6},
        {DPAD_DOWN, 6},
        {DPAD_RIGHT, 11},
        {DPAD_UP, 3},
        {DPAD_RIGHT, 2},
        {DPAD_UP, 16},
        {DPAD_LEFT, 6},
    };

    const Move sequence5[] = {
        {DPAD_RIGHT, 6},
        {DPAD_DOWN, 17},
        {DPAD_LEFT, 5},
        {DPAD_DOWN, 2},
        {DPAD_LEFT, 8},
        {DPAD_UP, 11},
        {DPAD_RIGHT, 2},
        {DPAD_UP, 7},
        {DPAD_LEFT, 2},
        {DPAD_UP, 7},
    };

    const Move sequence6[] = {
        {DPAD_DOWN, 10},
        {DPAD_RIGHT, 2},
        {DPAD_DOWN, 4},
        {DPAD_LEFT, 2},
        {DPAD_DOWN, 13},
        {DPAD_RIGHT, 2},
        {DPAD_DOWN, 2},
    };

    for(int i=0; i < 7; i++){
        pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
    }

    pbf_mash_button(context, BUTTON_A, 2500ms); //Delay may need increasing depending on length of dialogue.
    pbf_press_dpad(context, DPAD_UP, 80ms, 2000ms);

    for (const auto& move : sequence1){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }

    //Button presses to initiate Trainer battle 01 (Lass Caroline).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue.

   for (const auto& move : sequence2){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }

    //Button presses to initiate Trainer battle 02 (Aroma Lady Jenna).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue.

    for (const auto& move : sequence3){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }

    //Button presses to initiate Trainer battle 03 (Aroma Lady Angela).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence4){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }

    //Button presses to initiate Trainer battle 04 (Beauty Lindsay).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence5){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }

    //Button presses to initiate Gym battle.

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence6){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }

    pbf_wait(context, 2000ms);

    for(int i=0; i < 9; i++){
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    }
  
    //Exits Buiilding
    pbf_wait(context, 1000ms);
}

std::string AutoStory_Segment_02::name()       const{ return "02: First Badge to Second Badge"; }
std::string AutoStory_Segment_02::start_text() const{ return "Start: First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_Segment_02::end_text()   const{ return "End: Second Gym Badge acquired (Eterna City)."; }

void AutoStory_Segment_02::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_checkpoint_007().run_checkpoint(env, context, options, stats);
    AutoStory_checkpoint_008().run_checkpoint(env, context, options, stats);
    AutoStory_checkpoint_009().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}
std::string AutoStory_checkpoint_007::name()       const { return "007 - " + AutoStory_Segment_02().name(); }
std::string AutoStory_checkpoint_007::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_007::end_text()   const { return "Standing in front of the scientist in Valley windworks after beating Mars."; }

void AutoStory_checkpoint_007::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    checkpoint_007(env, context, options, stats);
}


void checkpoint_007(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 02 gameplay logic
            roark_to_mars(env.console, context);
            
        }
    );
}
std::string AutoStory_checkpoint_008::name()       const { return "008 - " + AutoStory_Segment_02().name(); }
std::string AutoStory_checkpoint_008::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_008::end_text()   const { return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_checkpoint_008::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    checkpoint_008(env, context, options, stats);
}


void checkpoint_008(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 02 gameplay logic
            mars_to_eterna_forest(env.console, context);
         
        }
    );
}

std::string AutoStory_checkpoint_009::name()       const{ return "009 - " + AutoStory_Segment_02().name(); }
std::string AutoStory_checkpoint_009::start_text() const{ return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_009::end_text()   const{ return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_checkpoint_009::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_009(env, context, options, stats);
}


void checkpoint_009(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 02 gameplay logic
            eterna_forest(env.console, context);
            
        }
    );
}
std::string AutoStory_checkpoint_010::name()       const { return "010 - " + AutoStory_Segment_02().name(); }
std::string AutoStory_checkpoint_010::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_010::end_text()   const { return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_checkpoint_010::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    checkpoint_010(env, context, options, stats);
}


void checkpoint_010(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 02 gameplay logic
            eterna_forest_exit_to_gym(env.console, context);
            //#. Eterna Gym navigation and battle
            gym_Puzzle_02(env.console, context);
        }
    );
}

}
}
}
