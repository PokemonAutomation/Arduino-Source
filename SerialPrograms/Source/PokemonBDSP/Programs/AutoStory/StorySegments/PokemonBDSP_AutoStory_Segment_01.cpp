/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_01.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    void jubilife_to_oreburgh_gate(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {

	}

    void oreburgh_gate_to_first_gym(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {

    }
    // ---------------------------------------------------------------------------
    //  Segment 1 Part ? Gym navigation
    //  Navigates the player through the first Gym in Oreburgh City and completes the Gym battle.
    // ---------------------------------------------------------------------------

    static void gym_Puzzle_01(VideoStream& stream, ProControllerContext& context){

        //Upon entering the Gym.

        struct Move {
        DpadPosition dir;
        int count;
        };
        const Move sequence[] = {
            {DPAD_UP, 3},
            {DPAD_LEFT, 4},
            {DPAD_UP, 5},
            {DPAD_RIGHT, 8},
            {DPAD_UP, 11},
            {DPAD_LEFT, 5},
            {DPAD_UP, 4},
        };

        for (const auto& move : sequence) {
            for (int i = 0; i < move.count; i++) {
                pbf_press_dpad(context, move.dir, 80ms, 300ms);
            }
        }

        //Button presses to initiate the Gym battle.

        //Battle automation code.

        //Button presses after battle with small cutscene of badge being put in case between dialogue boxes.

        for (int i = 0; i < 21; i++) {
            pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
        }

        //Exits Buiilding
        pbf_wait(context, 1000ms);
    }

    std::string AutoStory_Segment_01::name()       const { return "01: Jubilife City to First Badge"; }
    std::string AutoStory_Segment_01::start_text() const { return "Start: Recieved Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki."; }
    std::string AutoStory_Segment_01::end_text()   const { return "End: First Gym Badge acquired (Oreburgh City)."; }

    void AutoStory_Segment_01::run_segment(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const {
        stats.m_segment++;
        env.update_stats();
        context.wait_for_all_requests();
        env.console.log("Start Segment " + name(), COLOR_ORANGE);

        AutoStory_checkpoint_005().run_checkpoint(env, context, options, stats);
        AutoStory_checkpoint_006().run_checkpoint(env, context, options, stats);
        context.wait_for_all_requests();
        env.console.log("End Segment " + name(), COLOR_GREEN);
    }


    std::string AutoStory_checkpoint_005::name()       const { return "005 - " + AutoStory_Segment_01().name(); }
    std::string AutoStory_checkpoint_005::start_text() const { return "Recieved Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki."; }
    std::string AutoStory_checkpoint_005::end_text()   const { return "Standing outside the Oreburgh Gate on the Jubilife City side."; }

    void AutoStory_checkpoint_005::run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const {
        checkpoint_005(env, context, options, stats);
    }


    void checkpoint_005(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) {
        checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
            [&](size_t /*attempt*/) {
                jubilife_to_oreburgh_gate(env, env.console, context);

            }
        );
    }

    std::string AutoStory_checkpoint_006::name()       const { return "006 - " + AutoStory_Segment_01().name(); }
    std::string AutoStory_checkpoint_006::start_text() const { return AutoStory_checkpoint_005().end_text(); }
    std::string AutoStory_checkpoint_006::end_text()   const { return "Standing At the first gym leader after defeating him."; }

    void AutoStory_checkpoint_006::run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const {
        checkpoint_006(env, context, options, stats);
    }


    void checkpoint_006(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) {
        checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
            [&](size_t /*attempt*/) {
                oreburgh_gate_to_first_gym(env, env.console, context);
                //#. Oreburgh Gym navigation and battle
                gym_Puzzle_01(env.console, context);
                
            }
        );
    }

   
    
}
}
}
