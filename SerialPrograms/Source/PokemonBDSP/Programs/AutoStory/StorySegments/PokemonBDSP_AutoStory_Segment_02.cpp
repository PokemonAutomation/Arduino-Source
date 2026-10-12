/*  BDSP AutoStory - Segment 02
 *  From after acquiring the first badge until after acquiring the second badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include <cmath>
#include <opencv2/imgproc.hpp>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_02.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    // Match only the mart emblem, avoiding moving NPCs and changing ground shadows.
    static cv::Mat mart_grayscale(const ImageRGB32& image) {
        cv::Mat pixels((int)image.height(), (int)image.width(), CV_8UC4,
            const_cast<uint32_t*>(image.data()), image.bytes_per_row());
        cv::Mat gray;
        cv::cvtColor(pixels, gray, cv::COLOR_BGRA2GRAY);
        return gray;
    }

    static void align_to_oreburgh_mart(VideoStream& stream, ProControllerContext& context) {
        const ImageRGB32 reference(RESOURCE_PATH() +
            "PokemonBDSP/Autostory/OreburghMart/MartEmblem.png");
        const ImageRGB32 small_reference = reference.scale_to(54, 43);
        const cv::Mat emblem = mart_grayscale(small_reference);
        context.wait_for_all_requests();
        pbf_wait(context, 700ms);
        context.wait_for_all_requests();
        size_t confirmations = 0;
        for (size_t attempt = 0; attempt < 40; attempt++) {
            const VideoSnapshot frame = stream.video().snapshot();
            if (!frame) {
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Mart alignment: No video frame.", stream);
            }
            const ImageRGB32 small_frame = frame->scale_to(960, 540);
            const cv::Mat gray = mart_grayscale(small_frame);
            cv::Mat scores;
            cv::matchTemplate(gray(cv::Rect(0, 0, 960, 360)), emblem, scores, cv::TM_CCOEFF_NORMED);
            double score;
            cv::Point location;
            cv::minMaxLoc(scores, nullptr, &score, nullptr, &location);
            // Reject an ambiguous landmark rather than steering toward another building.
            cv::Mat competitors = scores.clone();
            const cv::Rect exclusion = cv::Rect(location.x - 15, location.y - 15, 31, 31)
                & cv::Rect(0, 0, scores.cols, scores.rows);
            competitors(exclusion).setTo(-1);
            double runner_up;
            cv::minMaxLoc(competitors, nullptr, &runner_up);
            if (score < 0.93 || score - runner_up < 0.10) {
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Mart alignment: Emblem match missing or ambiguous.", stream);
            }
            // Reference emblem top-left at half resolution. The reference player
            // stands directly outside the door, 64 full-resolution pixels left
            // of the emblem center; do not steer to the emblem center itself.
            const int dx = location.x - 482;
            const int dy = location.y - 127;
            stream.log("[Mart alignment] score=" + std::to_string(score) +
                ", offset=(" + std::to_string(dx * 2) + "," + std::to_string(dy * 2) + ")");
            if (std::abs(dx) <= 3 && std::abs(dy) <= 3) {
                if (++confirmations >= 3) {
                    stream.log("[Mart alignment] Door position confirmed.");
                    return;
                }
                pbf_wait(context, 250ms);
            } else {
                confirmations = 0;
                const bool horizontal = std::abs(dx) >= std::abs(dy);
                const int error = horizontal ? dx : dy;
                const auto duration = std::abs(error) > 15 ? 80ms : 40ms;
                pbf_move_left_joystick(context,
                    {horizontal ? (dx > 0 ? 1.0 : -1.0) : 0.0,
                     horizontal ? 0.0 : (dy < 0 ? 1.0 : -1.0)}, duration, 0ms);
                pbf_wait(context, 350ms);
            }
            context.wait_for_all_requests();
        }
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Mart alignment: Could not reach the doorway reference position.", stream);
    }

    void roark_to_mars(VideoStream& stream, ProControllerContext& context) {
        pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 3995ms, 0ms);
        pbf_wait(context, 672ms);
        pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 681ms, 0ms);
        pbf_wait(context, 468ms);
        pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1081ms, 0ms);
        pbf_wait(context, 420ms);
        pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2445ms, 0ms);
        pbf_wait(context, 1056ms);
        pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 95ms, 0ms);
        align_to_oreburgh_mart(stream, context);
        pbf_wait(context, 4787ms);
        pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1309ms, 0ms);

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

    AutoStory_checkpoint_0201().run_checkpoint(env, context, options, stats);
    AutoStory_checkpoint_0202().run_checkpoint(env, context, options, stats);
    AutoStory_checkpoint_0203().run_checkpoint(env, context, options, stats);
    AutoStory_checkpoint_0204().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}
std::string AutoStory_checkpoint_0201::name()       const { return "02.01 - Outside Gym 1"; }
std::string AutoStory_checkpoint_0201::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_0201::end_text()   const { return "Standing in front of the scientist in Valley windworks after beating Mars."; }

void AutoStory_checkpoint_0201::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    checkpoint_0201(env, context, options, stats);
}


void checkpoint_0201(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 02 gameplay logic
            roark_to_mars(env.console, context);
            
        },
            
        true
    );
}
std::string AutoStory_checkpoint_0202::name()       const { return "02.02 - TBC"; }
std::string AutoStory_checkpoint_0202::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_0202::end_text()   const { return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_checkpoint_0202::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    checkpoint_0202(env, context, options, stats);
}


void checkpoint_0202(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 02 gameplay logic
            mars_to_eterna_forest(env.console, context);
         
        },
         
        true
    );
}

std::string AutoStory_checkpoint_0203::name()       const{ return "02.03 - TBC"; }
std::string AutoStory_checkpoint_0203::start_text() const{ return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_0203::end_text()   const{ return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_checkpoint_0203::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    checkpoint_0203(env, context, options, stats);
}


void checkpoint_0203(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/){
            // TODO: implement Segment 02 gameplay logic
            eterna_forest(env.console, context);
            
        },
            
        true
    );
}

std::string AutoStory_checkpoint_0204::name()       const { return "02.04 - TBC"; }
std::string AutoStory_checkpoint_0204::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_0204::end_text()   const { return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_checkpoint_0204::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    checkpoint_0204(env, context, options, stats);
}


void checkpoint_0204(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {
            // TODO: implement Segment 02 gameplay logic
            eterna_forest_exit_to_gym(env.console, context);

        },
        true
    );
}

std::string AutoStory_checkpoint_0205::name()       const { return "02.05 - Inside Gym"; }
std::string AutoStory_checkpoint_0205::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
std::string AutoStory_checkpoint_0205::end_text()   const { return "Second Gym Badge acquired (Eterna City)."; }

void AutoStory_checkpoint_0205::run_checkpoint(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const {
    checkpoint_0205(env, context, options, stats);
}


void checkpoint_0205(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) {
    checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
        [&](size_t /*attempt*/) {

            //#. Eterna Gym navigation and battle
            gym_Puzzle_02(env.console, context);
        },
        true
    );
}

}
}
}
