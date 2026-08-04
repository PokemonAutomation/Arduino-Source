/*  BDSP AutoStory - Segment 03
 *  From after acquiring the second badge until after acquiring the third badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_03.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    void eterna_gym_to_jupiter(VideoStream& stream, ProControllerContext& context) {

    }
    void jupiter_to_coronet(VideoStream& stream, ProControllerContext& context) {

    }
    void coronet1(VideoStream& stream, ProControllerContext& context) {

    }
    void coronet_to_heartholme(VideoStream& stream, ProControllerContext& context) {

    }
    void heartholme_to_lost_tower(VideoStream& stream, ProControllerContext& context) {

    }
    static void lost_tower(VideoStream& stream, ProControllerContext& context) {

    }
    void lost_tower_to_veilstone_gym(VideoStream& stream, ProControllerContext& context) {

    }

static void gym_Puzzle_03(VideoStream& stream, ProControllerContext& context){

    //Upon entering the Gym.

    struct Move {
        DpadPosition dir;
        int count;
        std::chrono::milliseconds delay;
    };

    const Move sequence1[] = {
        {DPAD_RIGHT, 8, 0ms},
        {DPAD_UP, 6, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_UP, 2, 0ms},
    };

    const Move sequence2[] = {
        {DPAD_UP, 5, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 5, 0ms},
        {DPAD_UP, 7, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 2000ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_DOWN, 10, 0ms},
        {DPAD_RIGHT, 6, 0ms},
        {DPAD_UP, 7, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 1000ms},
        {DPAD_UP, 6, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 1000ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_DOWN, 13, 0ms},
        {DPAD_LEFT, 3, 0ms},
        {DPAD_UP, 4, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 8, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 4, 0ms},
        {DPAD_LEFT, 4, 0ms},
        {DPAD_DOWN, 2, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 1000ms},
        {DPAD_UP, 2, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_DOWN, 6, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 7, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 5, 0ms},
        {DPAD_UP, 7, 0ms},
        {DPAD_LEFT, 4, 0ms},
        {DPAD_LEFT, 1, 2000ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 5, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 2000ms},
        {DPAD_DOWN, 5, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_DOWN, 3, 0ms},
    };

    const Move sequence3[] = {
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 3000ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 4, 0ms},
        {DPAD_UP, 5, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 2000ms},
        {DPAD_DOWN, 5, 0ms},
        {DPAD_LEFT, 8, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 3000ms},
        {DPAD_UP, 11, 0ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_DOWN, 13, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_UP, 3, 0ms},
    };

    const Move sequence4[] = {
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 9, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 4, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_RIGHT, 1, 1000ms},
        {DPAD_RIGHT, 1, 0ms},
        {DPAD_DOWN, 8, 0ms},
        {DPAD_RIGHT, 4, 0ms},
        {DPAD_UP, 3, 0ms},
    };

    const Move sequence5[] = {
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_UP, 3, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_UP, 8, 0ms},
    };

    const Move sequence6[] = {
        {DPAD_DOWN, 8, 0ms},
        {DPAD_LEFT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 5, 0ms},
        {DPAD_RIGHT, 2, 0ms},
        {DPAD_DOWN, 3, 0ms},
        {DPAD_LEFT, 1, 0ms},
        {DPAD_LEFT, 1, 3000ms},
        {DPAD_DOWN, 6, 0ms},
    };

    for (const auto& move : sequence1){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 01 (Black Belt Rafael).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence2){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 02 (Black Belt Jeffrey).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence3){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 03 (Black Belt Darren).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence4){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 04 (Black Belt Colby).

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence5){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate the Gym battle.

    //Battle automation code.

    pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

    for (const auto& move : sequence6){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Exits Buiilding
    pbf_wait(context, 1000ms);
}

std::string AutoStory_Segment_03::name()       const{ return "03: Second Badge to Third Badge"; }
std::string AutoStory_Segment_03::start_text() const{ return "Start: Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Segment_03::end_text()   const{ return "End: Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Segment_03::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_0301().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0302().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0303().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0304().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0305().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_0306().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_0301::name()       const{ return "03.01 - TBC"; }
std::string AutoStory_Checkpoint_0301::start_text() const{ return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_0301::end_text()   const{ return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_0301::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    Checkpoint_0301(env, context, options, stats);
}


void Checkpoint_0301(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 03 gameplay logic
            eterna_gym_to_jupiter(env.console, context);
            
        },
            
        true
    );
}
std::string AutoStory_Checkpoint_0302::name()       const { return "03.02 - TBC"; }
std::string AutoStory_Checkpoint_0302::start_text() const { return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_0302::end_text()   const { return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_0302::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0302(env, context, options, stats);
}


void Checkpoint_0302(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 03 gameplay logic
            jupiter_to_coronet(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0303::name()       const { return "03.03 - TBC"; }
std::string AutoStory_Checkpoint_0303::start_text() const { return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_0303::end_text()   const { return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_0303::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0303(env, context, options, stats);
}


void Checkpoint_0303(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 03 gameplay logic
            coronet1(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0304::name()       const { return "03.04 - TBC"; }
std::string AutoStory_Checkpoint_0304::start_text() const { return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_0304::end_text()   const { return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_0304::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0304(env, context, options, stats);
}


void Checkpoint_0304(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 03 gameplay logic
            coronet_to_heartholme(env.console, context);

        },

        true
    );
}
std::string AutoStory_Checkpoint_0305::name()       const { return "03.05 - TBC"; }
std::string AutoStory_Checkpoint_0305::start_text() const { return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_0305::end_text()   const { return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_0305::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0305(env, context, options, stats);
}


void Checkpoint_0305(
	SingleSwitchProgramEnvironment& env,
	ProControllerContext& context,
	AutoStoryOptions options,
	AutoStoryStats& stats
) {
	checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
		[&](size_t /*attempt*/) {
			// TODO: implement Segment 03 gameplay logic
			heartholme_to_lost_tower(env.console, context);
			lost_tower(env.console, context);

        },

        true
    );
}

std::string AutoStory_Checkpoint_0306::name()       const { return "03.06 - TBC"; }
std::string AutoStory_Checkpoint_0306::start_text() const { return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_0306::end_text()   const { return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_0306::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0306(env, context, options, stats);
}


void Checkpoint_0306(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 03 gameplay logic
            lost_tower_to_veilstone_gym(env.console, context);
        },
        true
    );
}

std::string AutoStory_Checkpoint_0307::name()       const { return "03.07 - Inside Gym 3"; }
std::string AutoStory_Checkpoint_0307::start_text() const { return "Second Gym Badge acquired (Eterna City)."; }
std::string AutoStory_Checkpoint_0307::end_text()   const { return "Third Gym Badge acquired (Hearthome City)."; }

void AutoStory_Checkpoint_0307::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    Checkpoint_0307(env, context, options, stats);
}


void Checkpoint_0307(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            //#. Veilstone Gym navigation and battle
            gym_Puzzle_03(env.console, context);
        },
        true
    );
}

}
}
}
