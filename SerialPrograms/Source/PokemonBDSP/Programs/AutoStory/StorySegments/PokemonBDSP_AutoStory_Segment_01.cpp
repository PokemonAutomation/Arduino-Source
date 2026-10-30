/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *  From: https://github.com/PokemonAutomation/
 */

#include <array>
#include <cmath>
#include <memory>

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_Autostory_Segment_01_tools.h"

#include "PokemonBDSP_AutoStory_Segment_01.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {

static const std::string& mine_navigation_resource_path(){
    static const std::string path = RESOURCE_PATH() + "PokemonBDSP/Autostory/Oreburg_mine_nav/";
    return path;
}

enum class MoveDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

//  Left joystick vector for a direction (matches the game's joystick convention used in this file).
static std::pair<double, double> joystick_for(MoveDirection direction){
    switch (direction){
    case MoveDirection::UP:
        return {0.0, 1.0};
    case MoveDirection::DOWN:
        return {0.0, -1.0};
    case MoveDirection::LEFT:
        return {-1.0, 0.0};
    case MoveDirection::RIGHT:
        return {1.0, 0.0};
    }
    return {0.0, 0.0};
}

//  Runs from a wild battle that is already in progress (menu visible or about to be).
//  Throws OperationFailedException if it cannot escape within `timeout`.

// The main game menu is independent evidence that battle has ended.


//  Watches for a short dialogue box over several attempts. Returns true as soon as one is seen.
//  Returns false (without throwing) only if every attempt times out, so callers can skip
//  an optional interaction. Multiple attempts guard against a single missed detection.
static bool try_detect_dialogue(VideoStream& stream,
    ProControllerContext& context,
    const std::string& label,
    size_t attempts = 3,
    std::chrono::milliseconds window = 5000ms){
    for (size_t attempt = 0; attempt < attempts; attempt++){
        ShortDialogWatcher watcher_cyan(COLOR_CYAN);
        ShortDialogWatcher watcher_red(COLOR_RED);
        if (wait_until(stream, context, window, {{watcher_cyan}, {watcher_red}}) >= 0){
            stream.log(label + " dialogue detected.");
            return true;
        }
        stream.log(label + " dialogue not detected (attempt " + std::to_string(attempt + 1) + "/" +
            std::to_string(attempts) + ").",
            COLOR_ORANGE);
    }
    return false;
}

struct DoorwayWalkOptions {
    BattleType battle_type = BattleType::STANDARD;
    //  Active movement per step. Time spent in battles or waiting is never counted.
    std::chrono::milliseconds step = 200ms;
    //  Extra walking allowed beyond `travel_time` before giving up.
    std::chrono::milliseconds grace = 1600ms;
    //  How long to watch for a black screen after each step.
    std::chrono::milliseconds black_screen_window = 1500ms;
    //  After a black screen, how long to wait for a battle menu before deciding it was a doorway.
    std::chrono::milliseconds battle_menu_window = 8000ms;
    size_t max_battles = 10;
    //  Also treat a white screen as the doorway transition (e.g. leaving the Oreburgh Gate).
    bool detect_white_screen = false;
};

//  Reusable: walk in `direction` for a cumulative `travel_time` of active movement until
//  a black screen NOT followed by a battle menu (a doorway/area transition) is seen.
//  Wild battles (black screen followed by the battle menu, e.g. in caves or grass) are
//  escaped and do not consume the time budget.
//  Throws OperationFailedException if the doorway is not found within
//  travel_time + options.grace, if more than options.max_battles occur, or if a battle
//  cannot be escaped.
//  Returns the total active walking time used.
static std::chrono::milliseconds walk_until_doorway(VideoStream& stream,
    ProControllerContext& context,
    MoveDirection direction,
    std::chrono::milliseconds travel_time,
    const DoorwayWalkOptions& options = DoorwayWalkOptions()){
    enum class NavState {
        WALKING,
        BLACK_SCREEN,
        IN_BATTLE,
        DONE
    };

    const auto [x, y] = joystick_for(direction);
    const std::chrono::milliseconds max_travel = travel_time + options.grace;

    std::chrono::milliseconds walked = 0ms;
    size_t battles = 0;
    NavState state = NavState::WALKING;

    while (state != NavState::DONE){
        switch (state){
        case NavState::WALKING: {
            if (walked >= max_travel){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "walk_until_doorway: Doorway not found within time budget.",
                    stream);
            }
            context.wait_for_all_requests();
            pbf_move_left_joystick(context, {x, y}, options.step, 0ms);
            context.wait_for_all_requests();
            walked += options.step;

            BlackScreenWatcher black_screen;
            WhiteScreenWatcher white_screen;
            if (options.detect_white_screen){
                int ret = wait_until(stream, context, options.black_screen_window, {{black_screen}, {white_screen}});
                if (ret == 0){
                    state = NavState::BLACK_SCREEN;
                }else if (ret == 1){
                    stream.log("[AutoStory] White screen detected.", COLOR_ORANGE);
                    state = NavState::DONE;
                }
            }else if (wait_until(stream, context, options.black_screen_window, {{black_screen}}) == 0){
                state = NavState::BLACK_SCREEN;
            }
            break;
        }
        case NavState::BLACK_SCREEN: {
            stream.log("[AutoStory] Black screen detected. Checking for battle menu.", COLOR_ORANGE);
            BattleMenuWatcher battle_menu(options.battle_type, COLOR_YELLOW);
            if (wait_until(stream, context, options.battle_menu_window, {{battle_menu}}) == 0){
                state = NavState::IN_BATTLE;
            }else{
                state = NavState::DONE;
            }
            break;
        }
        case NavState::IN_BATTLE: {
            if (++battles > options.max_battles){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "walk_until_doorway: Too many battles while walking.",
                    stream);
            }
            stream.log("[AutoStory] Wild battle. Escaping. Walked " + std::to_string(walked.count()) + "ms of " +
                std::to_string(travel_time.count()) + "ms.",
                COLOR_ORANGE);
            escape_wild_battle(stream, context, options.battle_type);
            state = NavState::WALKING;
            break;
        }
        case NavState::DONE:
            break;
        }
    }
    stream.log("[AutoStory] Doorway transition detected after " + std::to_string(walked.count()) + "ms of walking.",
        COLOR_GREEN);
    return walked;
}


// ----------------------------------------------------------------
//  Segment 1 Part 1: Jubilife City to Oreburgh Gate
//  Navigates the player from Jubilife City to the Oreburgh Gate, including the
//  first Barry fight.
// ---------------------------------------------------------------------------
void jubilife_to_oreburgh_gate(SingleSwitchProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context, AutoStoryStats& stats){
    // Wait for game to fully accept input after save/overworld return
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1465ms, 0ms);
    pbf_wait(context, 127ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 644ms, 0ms);
    pbf_wait(context, 698ms);
    leave_pokemon_center_before_barry_1(stream, context);
    pbf_wait(context, 2340ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 742ms, 0ms);
    pbf_wait(context, 348ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2694ms, 0ms);
    pbf_wait(context, 482ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 3810ms, 0ms);

    wait_for_dialogue(stream, context, "Phase 1 (Barry 1 fight dialog)");
    pbf_mash_button(context, BUTTON_B, 5000ms);
    fight_trainer_using_moves(stream, context, stats, "Route trainer", false);
    pbf_mash_button(context, BUTTON_B, 5000ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2761ms, 0ms);
    pbf_wait(context, 1647ms);
    walk_through_grass(stream, context, 1, 0, 5, "right through grass");
    wait_for_dialogue(stream, context, "Dallas fight dialog)");
    pbf_mash_button(context, BUTTON_B, 5000ms);
    fight_trainer_using_moves(stream, context, stats, "Route trainer", false); //fighting Dallas
    context.wait_for_all_requests();
    walk_through_grass(stream, context, 1, 0, 5, "right through grass");
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 600ms, 0ms);
    walk_through_grass(stream, context, 1, 0, 10, "right through grass");
    //there is a small chance that the player dodges sSebastian, so this
    //function addresses that edge case
    if (try_detect_dialogue(stream, context, "Sebastian fight")){
        pbf_mash_button(context, BUTTON_B, 5000ms);
        fight_trainer_using_moves(stream, context, stats, "Route trainer", false); //fighting Sebastian
        context.wait_for_all_requests();
    }else{
        stream.log("Sebastian dialogue never appeared. Skipping fight.", COLOR_ORANGE);
    }
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2000ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 8000ms, 0ms);
    pbf_wait(context, 200ms);
    context.wait_for_all_requests();
    enter_cave_with_down_right_search(stream, context, "jubilife_to_oreburgh_gate", 10000ms);
}
void oreburgh_gate_nav(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    pbf_wait(context, 2000ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 879ms, 0ms);
    wait_for_dialogue(stream, context, "HM Rock Smash Dialog");
    pbf_mash_button(context, BUTTON_B, 15000ms);
    walk_through_grass(stream, context, 1, 0, 10, "right through gate");
    //Similar to sebastian, an edge case exists where we run into too many
    //wild battles and don't reach curtis or diana. This detect dialog function solves that.
    while (!try_detect_dialogue(stream, context, "Curtis fight", 1, 3000ms)){
        walk_through_grass(stream, context, 1, 0, 1, "right through gate");
    }
    pbf_mash_button(context, BUTTON_B, 5000ms);
    fight_trainer_using_moves(stream, context, stats, "Route trainer", false); //fighting Curtis
    walk_through_grass(stream, context, 1, 0, 10, "right through gate");
    while (!try_detect_dialogue(stream, context, "Diana fight", 1, 3000ms)){
        walk_through_grass(stream, context, 1, 0, 1, "right through gate");
    }
    pbf_mash_button(context, BUTTON_B, 5000ms);
    fight_trainer_using_moves(stream, context, stats, "Route trainer", false); //fighting Diana
    //  Walk right until the gate exit transition (white screen, or black screen without a battle menu).
    //  Wild battles are escaped without using up the budget. Throws on failure so the
    //  checkpoint is retried.
    DoorwayWalkOptions gate_exit_options;
    gate_exit_options.detect_white_screen = true;
    walk_until_doorway(stream, context, MoveDirection::RIGHT, 4000ms, gate_exit_options);
    context.wait_for_all_requests();
}
// ---------------------------------------------------------------------------
//  Segment 1 Part 2: Oreburgh Gate to Oreburgh Mine
//  Assumes that the player is standing outside the Oreburgh Gate on the Jubilife City side.
//  Navigates to the mine.
//  ----------------------------------------------------------------------------
void oreburgh_gate_to_mine(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1056ms, 0ms);
    while (!try_detect_dialogue(stream, context, "Phase 2 (oreburgh tour)", 1, 3000ms)){
        pbf_move_left_joystick(context, {1.000000, -0.000000}, 1056ms, 0ms);
    }
    pbf_mash_button(context, BUTTON_B, 12000ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 319ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -1.000000}, 332ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 3124ms, 0ms);
    pbf_wait(context, 23ms);
    bool black_screen_found = false;
    for (size_t attempt = 0; attempt < 3 && !black_screen_found; attempt++){
        BlackScreenWatcher black_screen;
        int ret = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 4740ms, 0ms);
            },
            {black_screen});
        if (ret == 0){
            stream.log("Black screen detected.");
            black_screen_found = true;
        }
    }
    context.wait_for_all_requests();

    if (!black_screen_found){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "oreburgh_gate_to_mine: No black screen after 3 attempts.",
            stream);
    }

    //  Let the mine entrance transition fully finish so the next step's black screen
    //  watcher can't pick up the tail of this one.
    BlackScreenOverWatcher black_over;
    wait_until(stream, context, 6000ms, {{black_over}});
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
}

// Diagnostic only: identifies the entrance start without moving the player.


struct MineCenterOffset {
    bool valid;
    double offset;
    double score;
};

// Finds the sideways shift of stationary scenery relative to the center picture.
// Positive offset: scenery shifted right, so the player is left of center.
static MineCenterOffset measure_mine_center_offset(const ImageViewRGB32& reference, const ImageViewRGB32& live){
    if (!reference || !live){
        return {false, 0.0, 1000.0};
    }

    // Use a fixed working size so the search works at different capture resolutions.
    const ImageRGB32 reference_scaled = reference.scale_to(960, 540);
    const ImageRGB32 live_scaled = live.scale_to(960, 540);

    // Entrance beam and both posts. Avoid the player and shadow.
    const std::array<ImageFloatBox, 3> boxes = {ImageFloatBox(0.35, 0.285, 0.30, 0.07),
        ImageFloatBox(0.355, 0.37, 0.035, 0.23),
        ImageFloatBox(0.615, 0.37, 0.035, 0.23)};

    const int search_radius = (int)std::lround(0.10 * live_scaled.width());
    std::vector<double> scores(2 * search_radius + 1);
    std::vector<double> worst_regions(2 * search_radius + 1);

    int best_shift = 0;
    double best_score = 1000.0;

    // Search up to 10% of the screen width in either direction.
    for (int shift = -search_radius; shift <= search_radius; shift++){
        double total = 0.0;
        double worst = 0.0;

        for (const ImageFloatBox& box : boxes){
            const ImageViewRGB32 reference_crop = extract_box_reference(reference_scaled, box);

            // Translate the same crop horizontally in the live image.
            const ImageViewRGB32 live_crop = extract_box_reference(live_scaled, box, shift, 0);

            const double score = ImageMatch::pixel_RMSD(reference_crop, live_crop);

            total += score;
            if (score > worst){
                worst = score;
            }
        }

        const double average = total / boxes.size();
        const size_t index = static_cast<size_t>(shift + search_radius);

        scores[index] = average;
        worst_regions[index] = worst;

        if (average < best_score){
            best_score = average;
            best_shift = shift;
        }
    }

    // Reject another similarly good match well away from the chosen position.
    double alternative_score = 1000.0;

    for (int shift = -search_radius; shift <= search_radius; shift++){
        if (std::abs(shift - best_shift) <= std::lround(0.0125 * live_scaled.width())){
            continue;
        }

        const double score = scores[static_cast<size_t>(shift + search_radius)];
        if (score < alternative_score){
            alternative_score = score;
        }
    }

    const size_t best_index = static_cast<size_t>(best_shift + search_radius);

    const bool valid = best_score <= 20.0 && worst_regions[best_index] <= 30.0 &&
        alternative_score - best_score >= 3.0 && std::abs(best_shift) < search_radius;

    return {valid, static_cast<double>(best_shift) / live_scaled.width(), best_score};
}
// Test only: align with the entrance center, then stop.
static bool oreburgh_mine_alignment_test(VideoStream& stream, ProControllerContext& context){
    const ImageRGB32 reference(mine_navigation_resource_path() + "mine_start_center.png");

    constexpr size_t MAX_MOVEMENTS = 12;
    constexpr double CENTER_TOLERANCE = 0.008;

    size_t movements = 0;
    size_t center_streak = 0;

    context.wait_for_all_requests();
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    while (true){
        const VideoSnapshot frame = stream.video().snapshot();

        const MineCenterOffset measurement = measure_mine_center_offset(reference, frame);

        stream.log("[Mine align] Offset=" + std::to_string(measurement.offset) +
            ", score=" + std::to_string(measurement.score) + (measurement.valid ? ", trusted" : ", unsure"));

        if (!measurement.valid){
            stream.log("[Mine align] Entrance position unsure. Stopping.", COLOR_ORANGE);
            return false;
        }

        if (std::abs(measurement.offset) <= CENTER_TOLERANCE){
            center_streak++;

            if (center_streak >= 3){
                stream.log("[Mine align] Center reached. Test complete.", COLOR_GREEN);
                return true;
            }

            pbf_wait(context, 250ms);
            context.wait_for_all_requests();
            continue;
        }

        center_streak = 0;

        // Check the latest position before enforcing the movement limit.
        if (movements >= MAX_MOVEMENTS){
            stream.log("[Mine align] Movement limit reached. Stopping.", COLOR_ORANGE);
            return false;
        }

        const double x = measurement.offset > 0.0 ? 1.0 : -1.0;

        // Smaller movements when close to the center.
        const auto duration = std::abs(measurement.offset) < 0.025 ? 40ms : 80ms;

        stream.log(x > 0.0 ? "[Mine align] Moving a little right." : "[Mine align] Moving a little left.");

        BlackScreenWatcher black;
        BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
        BattleDialogWatcher dialog(COLOR_YELLOW);

        const int result = run_until<ProControllerContext>(stream,
            context,
            [x, duration](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {x, 0.0}, duration, 0ms);
                pbf_wait(ctx, 500ms);
                ctx.wait_for_all_requests();
            },
            {black, menu, dialog});

        if (result >= 0){
            stream.log("[Mine align] Battle or transition detected. "
                "Stopping the alignment test.",
                COLOR_ORANGE);
            return false;
        }

        movements++;
    }
}
// Confirm a battle before using the existing escape helper.
static bool mine_escape_and_settle(VideoStream& stream, ProControllerContext& context){
    BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);

    if (wait_until(stream, context, 12s, {{menu}}) != 0){
        stream.log("[Mine encounter] Battle menu not confirmed. Stopping.", COLOR_ORANGE);
        return false;
    }

    stream.log("[Mine encounter] Battle confirmed. Escaping.");
    escape_wild_battle(stream, context);
    context.wait_for_all_requests();

    // Check the return waits instead of ignoring their results.
    BlackScreenWatcher visible(COLOR_YELLOW,
        ImageFloatBox(0.1, 0.1, 0.8, 0.8),
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);

    if (wait_until(stream, context, 8s, {{visible}}) != 0){
        stream.log("[Mine encounter] Screen did not return. Stopping.", COLOR_ORANGE);
        return false;
    }


    pbf_wait(context, 800ms);
    context.wait_for_all_requests();

    stream.log("[Mine encounter] Escaped. Returning to the rock-walking loop.", COLOR_GREEN);
    return true;
}

// Temporary test: wiggle along the corridor until a battle is detected.
static bool mine_force_encounter_test(VideoStream& stream, ProControllerContext& context){
    stream.log("[Mine encounter] Starting forced encounter wiggle.");

    const auto deadline = std::chrono::steady_clock::now() + 60s;

    size_t pulses = 0;

    while (std::chrono::steady_clock::now() < deadline){
        // Alternate down and up. Do not move sideways.
        const double y = pulses % 2 == 0 ? -1.0 : 1.0;

        BlackScreenWatcher black;
        BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
        BattleDialogWatcher dialog(COLOR_YELLOW);

        const int result = run_until<ProControllerContext>(stream,
            context,
            [y](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0.0, y}, 120ms, 0ms);
                pbf_wait(ctx, 400ms);
                ctx.wait_for_all_requests();
            },
            {black, menu, dialog});

        if (result >= 0){
            stream.log("[Mine encounter] Wiggle interrupted. Checking for battle.");
            return mine_escape_and_settle(stream, context);
        }

        pulses++;
    }

    stream.log("[Mine encounter] No encounter within 60 seconds. "
        "Test incomplete; stopping.",
        COLOR_ORANGE);
    return false;
}

// Watch throughout an action, including its movement and picture checks.
// After an encounter, restart the action using fresh watcher state.
template <typename Action>
static bool mine_run_with_encounters(VideoStream& stream, ProControllerContext& context, Action action){
    for (size_t encounters = 0;;){
        BlackScreenWatcher black;
        BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
        BattleDialogWatcher dialog(COLOR_YELLOW);
        bool completed = false;

        const int result = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                completed = action(ctx);
            },
            {black, menu, dialog});

        if (result < 0){
            return completed;
        }

        if (++encounters > 10){
            stream.log("[Mine encounter] Encounter limit reached.", COLOR_ORANGE);
            return false;
        }

        if (!mine_escape_and_settle(stream, context)){
            return false;
        }

        stream.log("[Mine encounter] Escaped; resuming interrupted action.");
    }
}

static bool mine_timed_walk(VideoStream& stream,
    ProControllerContext& context,
    double x,
    double y,
    std::chrono::milliseconds duration){
    auto remaining = duration;

    return mine_run_with_encounters(stream, context, [&](ProControllerContext& ctx){
        while (remaining > 0ms){
            const auto pulse = remaining < 100ms ? remaining : 100ms;
            pbf_move_left_joystick(ctx, {x, y}, pulse, 0ms);
            pbf_wait(ctx, 700ms);
            ctx.wait_for_all_requests();

            // An interrupted pulse is retried after escaping.
            // This is a timing budget, not exact distance tracking.
            remaining -= pulse;
        }
        return true;
    });
}


struct MinePictureScore {
    double average = 1000.0;
    double worst = 1000.0;
    double horizontal = 0;
    double vertical = 0;
};

// Supports four, five, or six regions. Existing four-region callers keep
// their original search ranges through the default arguments.
template <size_t Regions>
static MinePictureScore mine_score_picture(const ImageRGB32& reference,
    const ImageRGB32& live,
    const std::array<ImageFloatBox, Regions>& boxes,
    double horizontal_range = 1.0 / 60,
    double vertical_range = 2.0 / 135){
    MinePictureScore best;
    const int radius_x = (int)std::lround(horizontal_range * live.width());
    const int radius_y = (int)std::lround(vertical_range * live.height());
    for (int y = -radius_y; y <= radius_y; y++){
        for (int x = -radius_x; x <= radius_x; x++){
            double total = 0.0;
            double worst = 0.0;
            for (const ImageFloatBox& box : boxes){
                const ImageViewRGB32 ref = extract_box_reference(reference, box);
                const ImageViewRGB32 patch = extract_box_reference(live, box, x, y);
                const double score = ImageMatch::pixel_RMSD(ref, patch);
                total += score;
                if (score > worst){
                    worst = score;
                }
            }
            const double average = total / boxes.size();
            if (average < best.average){
                best.average = average;
                best.worst = worst;
                best.horizontal = (double)x / live.width();
                best.vertical = (double)y / live.height();
            }
        }
    }
    return best;
}


static bool mine_check_picture(VideoStream& stream,
    ProControllerContext& context,
    const char* filename,
    const std::array<ImageFloatBox, 4>& boxes,
    const std::string& label,
    size_t max_attempts = 8,
    double max_average = 20.0,
    double max_patch = 30.0){
    const ImageRGB32 reference(mine_navigation_resource_path() + filename);
    const ImageRGB32 reference_scaled = reference.scale_to(960, 540);

    const bool confirmed = mine_run_with_encounters(stream, context, [&](ProControllerContext& ctx){
        // The watcher stays active throughout settling and sampling.
        // There is no separate 500ms watcher wait before each sample.
        ctx.wait_for_all_requests();
        pbf_wait(ctx, 700ms);
        ctx.wait_for_all_requests();

        // Restart confirmation after a battle; do not reuse old matches.
        size_t matching_frames = 0;

        for (size_t attempt = 0; attempt < max_attempts; attempt++){
            const VideoSnapshot frame = stream.video().snapshot();
            if (!frame){
                stream.log(label + ": no video frame.", COLOR_ORANGE);
                return false;
            }

            const ImageRGB32 live = frame->scale_to(960, 540);
            const MinePictureScore score = mine_score_picture(reference_scaled, live, boxes);

            stream.log(
                label + ": average=" + std::to_string(score.average) + ", worst patch=" + std::to_string(score.worst));

            if (score.average <= max_average && score.worst <= max_patch){
                if (++matching_frames >= 3){
                    return true;
                }
            }else{
                matching_frames = 0;
            }

            pbf_wait(ctx, 250ms);
            ctx.wait_for_all_requests();
        }

        return false;
    });

    stream.log(label + (confirmed ? ": confirmed." : ": not confirmed."), confirmed ? COLOR_GREEN : COLOR_ORANGE);
    return confirmed;
}

struct MineWalkOptions {
    double x = 0.0;
    double y = 0.0;
    std::chrono::milliseconds pulse = 40ms;
    std::chrono::milliseconds settle = 500ms;
    std::chrono::milliseconds initial_settle = 0ms;
    size_t max_movements = 30;
    double horizontal_range = 0.0125;
    double vertical_range = 1.0 / 90;
    double max_average = 20.0;
    double max_patch = 30.0;
};

template <size_t Regions, size_t Endpoints>
static bool mine_walk_to_endpoints(VideoStream& stream,
    ProControllerContext& context,
    const std::array<const char*, Endpoints>& files,
    const std::array<std::array<ImageFloatBox, Regions>, Endpoints>& boxes,
    const MineWalkOptions& options,
    const std::string& label,
    bool force_encounter = false){
    std::array<std::unique_ptr<ImageRGB32>, Endpoints> references;
    for (size_t index = 0; index < Endpoints; index++){
        const ImageRGB32 image(mine_navigation_resource_path() + files[index]);
        references[index] = std::make_unique<ImageRGB32>(image.scale_to(960, 540));
    }

    size_t movements = 0;
    bool forced_completed = false;

    while (true){
        bool request_forced_test = false;
        const bool reached = mine_run_with_encounters(stream, context, [&](ProControllerContext& ctx){
            if (options.initial_settle > 0ms){
                pbf_wait(ctx, options.initial_settle);
                ctx.wait_for_all_requests();
            }
            // Reset the match history whenever an encounter interrupts us.
            size_t matching_frames = 0;
            int previous_endpoint = -1;

            while (true){
                ctx.wait_for_all_requests();
                const VideoSnapshot frame = stream.video().snapshot();
                if (!frame){
                    stream.log(label + ": no video frame.", COLOR_ORANGE);
                    return false;
                }

                const ImageRGB32 live = frame->scale_to(960, 540);
                int accepted = -1;
                double accepted_average = 1000.0;

                for (size_t endpoint = 0; endpoint < Endpoints; endpoint++){
                    const MinePictureScore score = mine_score_picture(*references[endpoint],
                        live,
                        boxes[endpoint],
                        options.horizontal_range,
                        options.vertical_range);
                    stream.log(label + " Endpoint " + std::to_string(endpoint + 1) +
                        ": average=" + std::to_string(score.average) + ", worst patch=" + std::to_string(score.worst) +
                        ", screen-relative offset=(" + std::to_string(score.horizontal) + "," +
                        std::to_string(score.vertical) + ")");

                    if (score.average <= options.max_average && score.worst <= options.max_patch &&
                        score.average < accepted_average){
                        accepted = static_cast<int>(endpoint);
                        accepted_average = score.average;
                    }
                }

                if (accepted >= 0){
                    matching_frames = accepted == previous_endpoint ? matching_frames + 1 : 1;
                    previous_endpoint = accepted;
                    if (matching_frames >= 3){
                        stream.log(label + " Endpoint " + std::to_string(accepted + 1) + " confirmed.", COLOR_GREEN);
                        return true;
                    }
                    pbf_wait(ctx, 250ms);
                    ctx.wait_for_all_requests();
                    continue;
                }

                matching_frames = 0;
                previous_endpoint = -1;

                // Exit the guard before invoking the diagnostic wiggle:
                // it has its own encounter watcher and must not be nested.
                if (force_encounter && !forced_completed && movements >= 4){
                    request_forced_test = true;
                    return false;
                }

                if (movements >= options.max_movements){
                    stream.log(label + ": movement limit reached.", COLOR_ORANGE);
                    return false;
                }

                // Count issued commands, including interrupted commands.
                movements++;
                pbf_move_left_joystick(ctx, {options.x, options.y}, options.pulse, 0ms);
                pbf_wait(ctx, options.settle);
                ctx.wait_for_all_requests();
            }
        });

        if (reached){
            return true;
        }
        if (!request_forced_test){
            return false;
        }
        if (!mine_force_encounter_test(stream, context)){
            return false;
        }
        forced_completed = true;
        stream.log(label + ": forced encounter completed; resuming.");
    }
}

static bool
oreburgh_mine_walk_to_first_rock(VideoStream& stream, ProControllerContext& context, bool force_encounter = false){
    const std::array<const char*, 1> files = {"mine_path_01.png"};
    const std::array<std::array<ImageFloatBox, 6>, 1> boxes = {{{{ImageFloatBox(0.482, 0.585, 0.055, 0.055),
                    ImageFloatBox(0.335, 0.670, 0.050, 0.055),
                    ImageFloatBox(0.412, 0.670, 0.050, 0.055),
                    ImageFloatBox(0.570, 0.752, 0.045, 0.055),
                    ImageFloatBox(0.645, 0.752, 0.045, 0.055),
                    ImageFloatBox(0.570, 0.290, 0.080, 0.120)}}}};

    MineWalkOptions options;
    options.x = 0.0;
    options.y = -1.0;
    options.pulse = 80ms;
    options.max_movements = 40;
    options.horizontal_range = 0.025;
    return mine_walk_to_endpoints(stream, context, files, boxes, options, "[Mine path 01]", force_encounter);
}

static bool oreburgh_mine_move_right(VideoStream& stream, ProControllerContext& context){
    const std::array<const char*, 2> files = {"mine_path_01_1.png", "mine_path_01_2.png"};
    const std::array<std::array<ImageFloatBox, 6>, 2> boxes = {{{{ImageFloatBox(0.400, 0.585, 0.055, 0.055),
                    ImageFloatBox(0.245, 0.670, 0.050, 0.055),
                    ImageFloatBox(0.322, 0.670, 0.050, 0.055),
                    ImageFloatBox(0.480, 0.752, 0.045, 0.055),
                    ImageFloatBox(0.555, 0.752, 0.045, 0.055),
                    ImageFloatBox(0.485, 0.290, 0.080, 0.090)}},
            {{ImageFloatBox(0.325, 0.585, 0.055, 0.055),
                    ImageFloatBox(0.170, 0.670, 0.050, 0.055),
                    ImageFloatBox(0.244, 0.670, 0.050, 0.055),
                    ImageFloatBox(0.395, 0.752, 0.045, 0.055),
                    ImageFloatBox(0.470, 0.752, 0.045, 0.055),
                    ImageFloatBox(0.415, 0.290, 0.080, 0.090)}}}};

    MineWalkOptions options;
    options.x = 1.0;
    options.y = 0.0;
    options.pulse = 40ms;
    options.max_movements = 20;
    options.horizontal_range = 0.0125;
    return mine_walk_to_endpoints(stream, context, files, boxes, options, "[Mine right]");
}

static bool oreburgh_mine_down_past_central_rock(VideoStream& stream, ProControllerContext& context){
    const std::array<const char*, 2> files = {"mine_path_02_1.png", "mine_path_02_2.png"};
    const std::array<std::array<ImageFloatBox, 6>, 2> boxes = {{{{ImageFloatBox(0.345, 0.420, 0.040, 0.045),
                    ImageFloatBox(0.195, 0.500, 0.040, 0.045),
                    ImageFloatBox(0.270, 0.500, 0.040, 0.045),
                    ImageFloatBox(0.410, 0.590, 0.040, 0.045),
                    ImageFloatBox(0.480, 0.590, 0.040, 0.045),
                    ImageFloatBox(0.445, 0.180, 0.060, 0.090)}},
            {{ImageFloatBox(0.420, 0.420, 0.040, 0.045),
                    ImageFloatBox(0.270, 0.500, 0.040, 0.045),
                    ImageFloatBox(0.345, 0.500, 0.040, 0.045),
                    ImageFloatBox(0.485, 0.590, 0.040, 0.045),
                    ImageFloatBox(0.555, 0.590, 0.040, 0.045),
                    ImageFloatBox(0.520, 0.180, 0.060, 0.090)}}}};

    MineWalkOptions options;
    options.x = 0.0;
    options.y = -1.0;
    options.pulse = 40ms;
    options.max_movements = 30;
    // options.horizontal_range = 0.0125;
    // Path 02 can be shifted horizontally after the first right endpoint.
    // The screenshot regression matches at about -2.08% of screen width.
    options.horizontal_range = 0.025;
    return mine_walk_to_endpoints(stream, context, files, boxes, options, "[Mine down 02]");
}

static bool oreburgh_mine_move_left_to_gap(VideoStream& stream, ProControllerContext& context){
    const std::array<const char*, 1> files = {"mine_path_03.png"};
    const std::array<std::array<ImageFloatBox, 5>, 1> boxes = {{{{ImageFloatBox(0.340, 0.495, 0.040, 0.045),
                    ImageFloatBox(0.410, 0.495, 0.040, 0.045),
                    ImageFloatBox(0.555, 0.580, 0.040, 0.045),
                    ImageFloatBox(0.625, 0.580, 0.040, 0.045),
                    ImageFloatBox(0.580, 0.180, 0.060, 0.090)}}}};

    MineWalkOptions options;
    options.x = -1.0;
    options.y = 0.0;
    options.pulse = 40ms;
    options.max_movements = 30;
    options.horizontal_range = 0.0125;
    return mine_walk_to_endpoints(stream, context, files, boxes, options, "[Mine left 03]");
}


// Confirm we are far enough down before beginning the leftward section.


// Each picture stage owns its movement and matching settings.
struct MinePictureStep {
    const char* file;
    const char* label;
    std::array<ImageFloatBox, 4> boxes;
    double x;
    double y;
    std::chrono::milliseconds pulse = 80ms;
    size_t max_movements = 30;
    double max_average = 20.0;
    double max_patch = 30.0;
};

static bool mine_run_picture_step(VideoStream& stream, ProControllerContext& context, const MinePictureStep& step){
    MineWalkOptions options;
    options.x = step.x;
    options.y = step.y;
    options.pulse = step.pulse;
    options.max_movements = step.max_movements;
    options.max_average = step.max_average;
    options.max_patch = step.max_patch;
    options.horizontal_range = 1.0 / 60;
    options.vertical_range = 2.0 / 135;
    options.initial_settle = 700ms;
    options.settle = 700ms;
    const std::array<const char*, 1> files = {step.file};
    const std::array<std::array<ImageFloatBox, 4>, 1> boxes = {{step.boxes}};
    return mine_walk_to_endpoints(stream, context, files, boxes, options, step.label);
}

static bool oreburgh_mine_confirm_05(VideoStream& stream, ProControllerContext& context){
    const MinePictureStep step = {"mine_path_05.png",
        "[Mine endpoint 05]",
        {ImageFloatBox(0.240, 0.240, 0.055, 0.080),
            ImageFloatBox(0.390, 0.220, 0.050, 0.090),
            ImageFloatBox(0.360, 0.090, 0.030, 0.030),
            ImageFloatBox(0.550, 0.140, 0.040, 0.040)},
        0.0,
        -1.0,
        60ms,
        10,
        20.0,
        30.0};
    return mine_run_picture_step(stream, context, step);
}

// Timed movement through paths 6 and 7, with picture checks.
static bool oreburgh_mine_timed_to_07(VideoStream& stream, ProControllerContext& context){
    stream.log("[Mine 04-07] Walking down for 1500ms.");

    if (!mine_timed_walk(stream, context, 0.0, -1.0, 1500ms)){
        return false;
    }

    if (!oreburgh_mine_confirm_05(stream, context)){
        return false;
    }

    stream.log("[Mine 04-07] Walking left for 3000ms.");

    if (!mine_timed_walk(stream, context, -1.0, 0.0, 3000ms)){
        return false;
    }

    // Path 6: crate, nearby small rock, large rock, upper-right small rock.
    const std::array<ImageFloatBox, 4> path6_boxes = {ImageFloatBox(0.350, 0.250, 0.085, 0.075),
        ImageFloatBox(0.405, 0.510, 0.035, 0.040),
        ImageFloatBox(0.520, 0.300, 0.050, 0.080),
        ImageFloatBox(0.620, 0.350, 0.030, 0.040)};

    if (!mine_check_picture(stream, context, "mine_path_06.png", path6_boxes, "[Mine midpoint 06]")){
        return false;
    }

    stream.log("[Mine 04-07] Path 6 confirmed; walking down for 600ms.");

    if (!mine_timed_walk(stream, context, 0.0, -1.0, 600ms)){
        return false;
    }

    // Path 7: crate and three visible rock surfaces.
    const std::array<ImageFloatBox, 4> path7_boxes = {ImageFloatBox(0.365, 0.025, 0.070, 0.055),
        ImageFloatBox(0.420, 0.270, 0.035, 0.040),
        ImageFloatBox(0.560, 0.415, 0.025, 0.040),
        ImageFloatBox(0.530, 0.650, 0.060, 0.090)};

    return mine_check_picture(stream, context, "mine_path_07.png", path7_boxes, "[Mine endpoint 07]");
}

// Move left from path 7 to the position shown in path 8.
static bool oreburgh_mine_left_to_08(VideoStream& stream, ProControllerContext& context){
    const MinePictureStep step = {"mine_path_08.png",
        "[Mine endpoint 08]",
        {ImageFloatBox(0.495, 0.035, 0.070, 0.055),
            ImageFloatBox(0.550, 0.270, 0.035, 0.040),
            ImageFloatBox(0.695, 0.420, 0.030, 0.040),
            ImageFloatBox(0.555, 0.585, 0.035, 0.040)},
        -1.0,
        0.0,
        40ms,
        20,
        20.0,
        30.0};
    return mine_run_picture_step(stream, context, step);
}

// Walk down along the left wall to path 9.

static bool oreburgh_mine_down_to_09(VideoStream& stream, ProControllerContext& context){
    const MinePictureStep step = {"mine_path_09.png",
        "[Mine endpoint 09]",
        {ImageFloatBox(0.550, 0.115, 0.030, 0.040),
            ImageFloatBox(0.550, 0.390, 0.035, 0.040),
            ImageFloatBox(0.620, 0.390, 0.025, 0.035),
            ImageFloatBox(0.500, 0.720, 0.085, 0.080)},
        0.0,
        -1.0,
        40ms,
        25,
        25.0,
        35.0};
    return mine_run_picture_step(stream, context, step);
}


// Require dialogue to appear first.
// Then finish only after it has been absent for one continuous second.
static bool mine_finish_item_dialogue(VideoStream& stream, ProControllerContext& context){
    ShortDialogDetector dialogue(COLOR_YELLOW);

    bool saw_dialogue = false;
    auto absent_since = std::chrono::steady_clock::now();

    const auto start = std::chrono::steady_clock::now();
    const auto deadline = start + 30s;

    stream.log("[Mine item] Starting interaction.");

    while (std::chrono::steady_clock::now() < deadline){
        context.wait_for_all_requests();
        const VideoSnapshot frame = stream.video().snapshot();

        if (!frame){
            stream.log("[Mine item] No video frame. Stopping.", COLOR_ORANGE);
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        const bool visible = dialogue.detect(frame);

        if (visible){
            if (!saw_dialogue){
                stream.log("[Mine item] Dialogue detected.");
            }

            saw_dialogue = true;
            absent_since = now;

            // Short mash bursts allow frequent checks.
            pbf_mash_button(context, BUTTON_A, 100ms);
        }else if (!saw_dialogue){
            if (now - start >= 10s){
                stream.log("[Mine item] No dialogue appeared. Stopping.", COLOR_ORANGE);
                return false;
            }

            // Try to start the interaction.
            pbf_press_button(context, BUTTON_A, 60ms, 100ms);
        }else if (now - absent_since >= 1s){
            stream.log("[Mine item] Dialogue absent for one second. Finished.", COLOR_GREEN);
            return true;
        }

        pbf_wait(context, 100ms);
        context.wait_for_all_requests();
    }

    stream.log("[Mine item] Dialogue timeout. Stopping.", COLOR_ORANGE);
    return false;
}


static bool oreburgh_mine_09_to_14(VideoStream& stream, ProControllerContext& context){
    const std::array<ImageFloatBox, 4> path10_boxes = {
        ImageFloatBox(0.405, 0.720, 0.085, 0.080), // Crate
        ImageFloatBox(0.480, 0.120, 0.030, 0.040), // Upper rock
        ImageFloatBox(0.575, 0.420, 0.045, 0.080), // Large right rock
        ImageFloatBox(0.550, 0.720, 0.030, 0.040)  // Lower small rock
    };

    const MinePictureStep path10_boxes_step =
        {"mine_path_10.png", "[Mine endpoint 10]", path10_boxes, 1.0, 0.0, 80ms, 30, 20.0, 30.0};
    if (!mine_run_picture_step(stream, context, path10_boxes_step)){
        return false;
    }

    const std::array<ImageFloatBox, 4> path11_boxes = {ImageFloatBox(0.405, 0.560, 0.070, 0.080),
        ImageFloatBox(0.475, 0.270, 0.030, 0.040),
        ImageFloatBox(0.575, 0.290, 0.045, 0.080),
        ImageFloatBox(0.550, 0.575, 0.030, 0.040)};

    const MinePictureStep path11_boxes_step =
        {"mine_path_11.png", "[Mine endpoint 11]", path11_boxes, 0.0, -1.0, 80ms, 30, 20.0, 30.0};
    if (!mine_run_picture_step(stream, context, path11_boxes_step)){
        return false;
    }

    const std::array<ImageFloatBox, 4> path12_boxes = {ImageFloatBox(0.260, 0.560, 0.080, 0.080),
        ImageFloatBox(0.355, 0.270, 0.025, 0.035),
        ImageFloatBox(0.435, 0.290, 0.040, 0.070),
        ImageFloatBox(0.415, 0.585, 0.025, 0.035)};

    const MinePictureStep path12_boxes_step =
        {"mine_path_12.png", "[Mine endpoint 12]", path12_boxes, 1.0, 0.0, 80ms, 30, 20.0, 30.0};
    if (!mine_run_picture_step(stream, context, path12_boxes_step)){
        return false;
    }

    const std::array<ImageFloatBox, 4> path13_boxes = {
        ImageFloatBox(0.290, 0.230, 0.070, 0.060), // Upper crate
        ImageFloatBox(0.285, 0.390, 0.070, 0.060), // Lower crate
        ImageFloatBox(0.425, 0.265, 0.025, 0.035), // Upper small rock
        ImageFloatBox(0.405, 0.580, 0.030, 0.040)  // Bottom-left small rock
    };

    const MinePictureStep path13_boxes_step =
        {"mine_path_13.png", "[Mine endpoint 13]", path13_boxes, 0.0, -1.0, 80ms, 30, 20.0, 30.0};
    if (!mine_run_picture_step(stream, context, path13_boxes_step)){
        return false;
    }

    // Turn left, then verify position before starting the interaction.
    pbf_move_left_joystick(context, {-1.0, 0.0}, 40ms, 0ms);
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    if (!mine_check_picture(stream, context, "mine_path_14.png", path13_boxes, "[Mine position 14]", 3)){
        return false;
    }
    stream.log("[Mine item] Position confirmed. Pressing A.");

    pbf_press_button(context, BUTTON_A, 80ms, 200ms);
    context.wait_for_all_requests();

    return mine_finish_item_dialogue(stream, context);
}

// Compare a reference patch with a shifted live patch using the same
// brightness adjustment as the native ImageMatchDetector.
static double mine_offset_patch_score(const ImageRGB32& reference,
    const ImageRGB32& live,
    const ImageFloatBox& box,
    double horizontal,
    double vertical){
    const ImageViewRGB32 reference_crop = extract_box_reference(reference, box);
    ImageFloatBox shifted = box;
    shifted.x += horizontal;
    shifted.y += vertical;
    const ImageViewRGB32 live_crop = extract_box_reference(live, shifted);
    ImageRGB32 adjusted = live_crop.scale_to(reference_crop.width(), reference_crop.height());

    const auto reference_average = ImageMatch::pixel_average(reference_crop, reference_crop);
    const auto live_average = ImageMatch::pixel_average(adjusted, reference_crop);
    auto scale = reference_average / live_average;
    if (std::isnan(scale.r)){
        scale.r = 1.0;
    }
    if (std::isnan(scale.g)){
        scale.g = 1.0;
    }
    if (std::isnan(scale.b)){
        scale.b = 1.0;
    }
    scale.bound(0.8, 1.2);
    ImageMatch::scale_brightness(adjusted, scale);

    return ImageMatch::pixel_RMSD(reference_crop, adjusted);
}

struct MineRoarkOffsetScore {
    double left = 1000.0;
    double right = 1000.0;
    double helmet = 1000.0;
    double horizontal = 0;
    double vertical = 0;
    double helmet_horizontal = 0;
    double helmet_vertical = 0;
};

static MineRoarkOffsetScore mine_score_roark_offsets(const ImageRGB32& reference, const ImageRGB32& live){
    const ImageFloatBox left_box(0.415, 0.400, 0.050, 0.095);
    const ImageFloatBox right_box(0.555, 0.400, 0.055, 0.095);
    const ImageFloatBox helmet_box(0.485, 0.335, 0.035, 0.045);

    MineRoarkOffsetScore best;
    double best_worst_rock = 1000.0;
    double best_total = 2000.0;

    // Both rocks must use the SAME offset. They cannot match independently.
    // Search ranges are screen fractions; enumerate pixels only for scoring.
    const int radius_x = (int)std::lround(live.width() / 60.0);
    const int radius_y = (int)std::lround(live.height() * 2.0 / 135);
    for (int y = -radius_y; y <= radius_y; y++){
        for (int x = -radius_x; x <= radius_x; x++){
            const double dx = (double)x / live.width(), dy = (double)y / live.height();
            const double left = mine_offset_patch_score(reference, live, left_box, dx, dy);
            const double right = mine_offset_patch_score(reference, live, right_box, dx, dy);
            const double worst = left > right ? left : right;
            const double total = left + right;

            if (worst < best_worst_rock || (worst == best_worst_rock && total < best_total)){
                best_worst_rock = worst;
                best_total = total;
                best.left = left;
                best.right = right;
                best.horizontal = (double)x / live.width();
                best.vertical = (double)y / live.height();
            }
        }
    }

    // Roark's helmet may move slightly relative to the stationary rocks.
    // Keep this extra search small so another red object cannot match far away.
    const int helmet_radius_x = (int)std::lround(live.width() * 0.00625);
    const int helmet_radius_y = (int)std::lround(live.height() / 90.0);
    for (int extra_y = -helmet_radius_y; extra_y <= helmet_radius_y; extra_y++){
        for (int extra_x = -helmet_radius_x; extra_x <= helmet_radius_x; extra_x++){
            const double score = mine_offset_patch_score(reference,
                live,
                helmet_box,
                best.horizontal + (double)extra_x / live.width(),
                best.vertical + (double)extra_y / live.height());
            if (score < best.helmet){
                best.helmet = score;
                best.helmet_horizontal = (double)extra_x / live.width();
                best.helmet_vertical = (double)extra_y / live.height();
            }
        }
    }

    return best;
}

static bool oreburgh_mine_align_to_18(VideoStream& stream, ProControllerContext& context){
    const ImageRGB32 reference(mine_navigation_resource_path() + "Mine_path_18.png");
    const ImageRGB32 reference_scaled = reference.scale_to(960, 540);
    const double max_rmsd = 65.0;
    size_t movements = 0;

    return mine_run_with_encounters(stream, context, [&](ProControllerContext& ctx){
        // Fresh confirmation after any encounter.
        size_t matching_frames = 0;
        pbf_wait(ctx, 500ms);
        ctx.wait_for_all_requests();

        while (true){
            const VideoSnapshot frame = stream.video().snapshot();
            if (!frame){
                stream.log("[Mine endpoint 18] No video frame.", COLOR_ORANGE);
                return false;
            }

            const ImageRGB32 live = frame->scale_to(960, 540);
            const MineRoarkOffsetScore score = mine_score_roark_offsets(reference_scaled, live);

            stream.log("[Mine endpoint 18] Left rock=" + std::to_string(score.left) +
                ", Right rock=" + std::to_string(score.right) + ", Helmet=" + std::to_string(score.helmet) +
                ", shared screen-relative offset=(" + std::to_string(score.horizontal) + "," +
                std::to_string(score.vertical) + "), helmet screen-relative extra=(" +
                std::to_string(score.helmet_horizontal) + "," + std::to_string(score.helmet_vertical) + ")");

            if (score.left <= max_rmsd && score.right <= max_rmsd && score.helmet <= max_rmsd){
                if (++matching_frames >= 3){
                    stream.log("[Mine endpoint 18] Rocks and helmet confirmed.", COLOR_GREEN);
                    return true;
                }
                pbf_wait(ctx, 250ms);
                ctx.wait_for_all_requests();
                continue;
            }

            matching_frames = 0;
            if (movements >= 80){
                stream.log("[Mine endpoint 18] Movement limit reached.", COLOR_ORANGE);
                return false;
            }

            const bool approaching = score.left <= 120.0 && score.right <= 120.0 && score.helmet <= 150.0;
            const bool near = score.left <= 100.0 && score.right <= 100.0 && score.helmet <= 100.0;
            const auto pulse = near ? 20ms : approaching ? 40ms : 60ms;

            // Use the outer encounter watcher; do not nest mine_timed_walk.
            // Count issued pulses, including any interrupted by a battle.
            movements++;
            pbf_move_left_joystick(ctx, {1.0, 0.0}, pulse, 0ms);
            pbf_wait(ctx, 700ms);
            ctx.wait_for_all_requests();
        }
    });
}

static bool oreburgh_mine_14_to_19(VideoStream& stream, ProControllerContext& context){
    const std::array<ImageFloatBox, 4> path15_boxes = {
        ImageFloatBox(0.140, 0.220, 0.080, 0.070), // Upper crate
        ImageFloatBox(0.125, 0.370, 0.080, 0.070), // Lower crate
        ImageFloatBox(0.285, 0.250, 0.035, 0.045), // Small upper rock
        ImageFloatBox(0.325, 0.560, 0.035, 0.045)  // Lower rock
    };

    const MinePictureStep path15_boxes_step =
        {"mine_path_15_actual.png", "[Mine endpoint 15]", path15_boxes, 1.0, 0.0, 80ms, 30, 20.0, 30.0};
    if (!mine_run_picture_step(stream, context, path15_boxes_step)){
        return false;
    }

    // Move below the worker before continuing right.
    const std::array<ImageFloatBox, 4> path16_boxes = {
        ImageFloatBox(0.145, 0.160, 0.080, 0.070), // Upper crate
        ImageFloatBox(0.135, 0.305, 0.080, 0.070), // Lower crate
        ImageFloatBox(0.290, 0.195, 0.035, 0.045), // Small upper rock
        ImageFloatBox(0.330, 0.495, 0.035, 0.045)  // Lower rock
    };

    const MinePictureStep path16_boxes_step =
        {"mine_path_16.png", "[Mine endpoint 16]", path16_boxes, 0.0, -1.0, 80ms, 30, 20.0, 30.0};
    if (!mine_run_picture_step(stream, context, path16_boxes_step)){
        return false;
    }

    // Skip 17 and find the actual stopping point at 18.
    if (!oreburgh_mine_align_to_18(stream, context)){
        return false;
    }

    // Position 19 is the upward turn toward Roark.
    pbf_move_left_joystick(context, {0.0, 1.0}, 40ms, 0ms);
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    stream.log("[Mine endpoint 19] Position 18 confirmed; north turn sent.", COLOR_GREEN);
    return true;
}

bool oreburgh_mine_route(VideoStream& stream, ProControllerContext& context, const MineRouteTestOptions& options){
    stream.log("[Mine resources] " + mine_navigation_resource_path());
    struct Step {
        const char* name;
        bool (*run)(VideoStream&, ProControllerContext&);
    };

    const std::array<Step, 10> steps = {{{"Align entrance", oreburgh_mine_alignment_test},
            {"Walk to first rock",
                [](VideoStream& video, ProControllerContext& controller){
                    return oreburgh_mine_walk_to_first_rock(video, controller, false);
                }},
            {"Move right of first rock", oreburgh_mine_move_right},
            {"Move below first rock", oreburgh_mine_down_past_central_rock},
            {"Move left into center gap", oreburgh_mine_move_left_to_gap},
            {"Reach position 07", oreburgh_mine_timed_to_07},
            {"Reach position 08", oreburgh_mine_left_to_08},
            {"Reach position 09", oreburgh_mine_down_to_09},
            {"Reach 14 and collect item", oreburgh_mine_09_to_14},
            {"Reach 18 and turn north for 19", oreburgh_mine_14_to_19}}};

    const size_t first = static_cast<size_t>(options.first);
    const size_t last = static_cast<size_t>(options.last);

    if (first > last || last >= steps.size()){
        stream.log("[Mine route] Invalid stage range.", COLOR_ORANGE);
        return false;
    }

    for (size_t index = first; index <= last; index++){
        const Step& step = steps[index];

        stream.log(std::string("[Mine route] Starting: ") + step.name);

        bool succeeded = false;

        try {
            succeeded = step.run(stream, context);
        } catch (OperationFailedException& error){
            stream.log(std::string("[Mine route] Failed during ") + step.name + ": " + error.message(), COLOR_ORANGE);

            if (options.reset_on_failure){
                throw;
            }

            stream.log("[Mine route] Test stopped without requesting a reset.", COLOR_ORANGE);
            return false;
        }

        if (!succeeded){
            stream.log(std::string("[Mine route] Stage failed: ") + step.name, COLOR_ORANGE);

            if (options.reset_on_failure){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    std::string("Mine route failed during: ") + step.name,
                    stream);
            }

            return false;
        }

        stream.log(std::string("[Mine route] Completed: ") + step.name, COLOR_GREEN);
    }

    stream.log("[Mine route] Selected stages completed.", COLOR_GREEN);
    return true;
}

// Start Roark's conversation once, then advance it with B.
// Require dialogue to be seen before accepting its disappearance.

static bool mine_talk_to_roark(VideoStream& stream, ProControllerContext& context){
    stream.log("[Mine Roark] Pressing A to start conversation.");
    pbf_press_button(context, BUTTON_A, 80ms, 100ms);
    context.wait_for_all_requests();

    // The opening cutscene precedes the first dialogue box.
    // Do not mash B or measure dialogue absence during that cutscene.
    wait_for_dialogue(stream, context, "[Mine Roark] First box", 30s);

    ShortDialogDetector dialogue(COLOR_YELLOW);
    const auto deadline = std::chrono::steady_clock::now() + 60s;
    bool timing_absence = false;
    auto absent_since = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() < deadline){
        const VideoSnapshot frame = stream.video().snapshot();
        if (!frame){
            stream.log("[Mine Roark] No video frame.", COLOR_ORANGE);
            return false;
        }
        const auto now = std::chrono::steady_clock::now();
        if (dialogue.detect(frame)){
            timing_absence = false;
        }else{
            if (!timing_absence){
                timing_absence = true;
                absent_since = now;
            }
            if (now - absent_since >= 4s){
                stream.log("[Mine Roark] Dialogue absent for 4 seconds. Finished.", COLOR_GREEN);
                return true;
            }
        }
        pbf_mash_button(context, BUTTON_B, 100ms);
        context.wait_for_all_requests();
    }
    stream.log("[Mine Roark] Conversation timed out.", COLOR_ORANGE);
    return false;
}

// Retain the developer test entry point; both callers share the same route.
bool oreburgh_mine_first_leg_test(VideoStream& stream,
    ProControllerContext& context,
    const MineRouteTestOptions& options){
    return oreburgh_mine_route(stream, context, options);
}

// Shared final step for the live route and the entrance-start test.
static bool mine_talk_and_escape(VideoStream& stream, ProControllerContext& context){
    if (!mine_talk_to_roark(stream, context)){
        return false;
    }
    stream.log("[Mine exit] Finding Escape Rope in the Bag.");
    if (!use_escape_rope(stream, context, 8)){
        return false;
    }
    stream.log("[Mine exit] Roark conversation and Escape Rope exit succeeded.", COLOR_GREEN);
    return true;
}

bool oreburgh_mine_route_and_escape_test(VideoStream& stream, ProControllerContext& context){
    MineRouteTestOptions route;
    route.reset_on_failure = false;
    if (!oreburgh_mine_first_leg_test(stream, context, route)){
        return false;
    }
    try {
        if (!mine_talk_and_escape(stream, context)){
            stream.log("[Mine exit test] Failed. Stopping without resetting.", COLOR_ORANGE);
            return false;
        }
    } catch (OperationFailedException& error){
        stream.log("[Mine exit test] " + error.message(), COLOR_ORANGE);
        stream.log("[Mine exit test] Stopped without requesting a reset.", COLOR_ORANGE);
        return false;
    }
    return true;
}

void oreburgh_mine_nav(VideoStream& stream, ProControllerContext& context){
    walk_through_grass(stream, context, 0, -1, 1, "down into mine");
    context.wait_for_all_requests();
    walk_until_doorway(stream, context, MoveDirection::DOWN, 4350ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1824ms);

    MineRouteTestOptions route;
    route.reset_on_failure = true;

    if (!oreburgh_mine_route(stream, context, route)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Mine route did not complete.",
            stream);
    }


    if (!mine_talk_and_escape(stream, context)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Mine route: Roark conversation or Escape Rope exit failed.",
            stream);
    }
}
// ---------------------------------------------------------------------------
//  Segment 1 Part 3: Oreburgh Mine to Gym
//  Assumes that escape rope was picked up and used to exit the mine.
//  Follows the validated blind route into the gym.
//  ----------------------------------------------------------------------------
void oreburgh_mine_to_gym(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){

    stream.log("[Mine to gym] Following recorded route from the mine exit.");
    pbf_wait(context, 700ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 3995ms, 0ms);
    pbf_wait(context, 672ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 681ms, 0ms);
    pbf_wait(context, 468ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1081ms, 0ms);
    pbf_wait(context, 420ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2445ms, 0ms);
    pbf_wait(context, 1056ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 95ms, 0ms);
    pbf_wait(context, 4787ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1309ms, 0ms);

    //pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 4450ms, 0ms);
    //pbf_wait(context, 3635ms);
    //pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 640ms, 0ms);
    //pbf_wait(context, 3795ms);
    context.wait_for_all_requests();

    //stream.log("[Mine to gym] Staying west of the hiker; moving north along the left fence.");
    //pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2000ms, 0ms);
    //pbf_wait(context, 602ms);
    //pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2364ms, 0ms);
    //pbf_wait(context, 5069ms);
    //pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1079ms, 0ms);
    //pbf_wait(context, 605ms);
    //pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 728ms, 0ms);
    //pbf_wait(context, 1081ms);
    //pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 155ms, 0ms);
    //pbf_wait(context, 2642ms);
    //pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 64ms, 0ms);
    //pbf_wait(context, 5340ms);
    //context.wait_for_all_requests();

    //pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 541ms, 0ms);
    //pbf_wait(context, 140ms);
    //pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 47ms, 0ms);
    //context.wait_for_all_requests();
    //stream.log("[Mine to gym] Recorded movement finished; waiting for gym entry to settle.");
    // walk_until_doorway(stream, context, MoveDirection::UP, 5s);
    // Finish entering the doorway with a bounded blind input.
    //pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1000ms, 0ms);
    //context.wait_for_all_requests();
    //pbf_wait(context, 1500ms);
    //context.wait_for_all_requests();
    stream.log("[Mine to gym] Blind mine-to-gym sequence completed.", COLOR_GREEN);
}

// ---------------------------------------------------------------------------
//  Segment 1 Part 4: Gym navigation
//  Navigates the player through the first Gym in Oreburgh City and completes the Gym battle.
// ---------------------------------------------------------------------------

void gym_Puzzle_01(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){

    //Upon entering the Gym.
    // The entry camera sequence lasts about 7.5 seconds in the reference
    // video. Allow another 500ms before sending puzzle movement inputs.
    context.wait_for_all_requests();
    env.log("[Gym puzzle] Waiting for the entry cutscene to finish.");
    pbf_wait(context, 8000ms);
    context.wait_for_all_requests();

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

    for (const auto& move : sequence){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }

    //Button presses to initiate the Gym battle.
    pbf_press_button(context, BUTTON_A, 180ms, 0ms);
    wait_for_dialogue(stream, context, "Phase 3 (Roark fight dialog)");
    pbf_mash_button(context, BUTTON_B, 5000ms);
    //Barry 1 battle logic can be used here since the moves don't matter this early.
    fight_trainer_using_moves(stream, context, stats, "Roark", false);

    context.wait_for_all_requests();
    wait_for_dialogue(stream, context, "Phase 4 (Roark post battle dialog)");
    ShortDialogDetector post_battle_dialogue(COLOR_YELLOW);
    const auto dialogue_deadline = std::chrono::steady_clock::now() + 180s;
    auto dialogue_absent_since = std::chrono::steady_clock::now();
    bool dialogue_finished = false;
    stream.log("[Gym post battle] Clearing dialogue until absent for 4 seconds.");
    while (std::chrono::steady_clock::now() < dialogue_deadline){
        const VideoSnapshot frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Gym post battle: No video frame while clearing dialogue.",
                stream);
        }
        const auto now = std::chrono::steady_clock::now();
        if (post_battle_dialogue.detect(frame)){
            dialogue_absent_since = now;
        }else if (now - dialogue_absent_since >= 4s){
            dialogue_finished = true;
            break;
        }
        pbf_mash_button(context, BUTTON_B, 100ms);
        context.wait_for_all_requests();
    }
    if (!dialogue_finished){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Gym post battle: Dialogue did not finish within 180 seconds.",
            stream);
    }
    stream.log("[Gym post battle] Dialogue absent for 4 seconds. Walking down.");

    for (int i = 0; i < 21; i++){
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
    }

    // Exit the building.
    pbf_wait(context, 1000ms);
}


std::string AutoStory_Segment_01::name() const {
    return "01: Jubilife City to First Badge";
}
std::string AutoStory_Segment_01::start_text() const {
    return "Start: Received Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki.";
}
std::string AutoStory_Segment_01::end_text() const {
    return "End: First Gym Badge acquired (Oreburgh City).";
}

void AutoStory_Segment_01::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_checkpoint_0101().run_checkpoint(env, context, options, stats);
    AutoStory_checkpoint_0102().run_checkpoint(env, context, options, stats);
    AutoStory_checkpoint_0103().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_checkpoint_0101::name() const {
    return "01.01 - Maxed Starter from Home";
}
std::string AutoStory_checkpoint_0101::start_text() const {
    return "Received Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki.";
}
std::string AutoStory_checkpoint_0101::end_text() const {
    return "Standing outside the Oreburgh Gate on the Jubilife City side.";
}
// ========================
// Checkpoint 0101 Wrapper
// ========================
void AutoStory_checkpoint_0101::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0101(env, context, options, stats);
}


void checkpoint_0101(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            jubilife_to_oreburgh_gate(env, env.console, context, stats);
            oreburgh_gate_nav(env, env.console, context, stats);
        },
        true);
}

// ========================
// Checkpoint 0102 Wrapper
// ========================

std::string AutoStory_checkpoint_0102::name() const {
    return "01.02 - Oreburgh Mine";
}
std::string AutoStory_checkpoint_0102::start_text() const {
    return AutoStory_checkpoint_0101().end_text();
}
std::string AutoStory_checkpoint_0102::end_text() const {
    return "Inside Oreburgh Gym, ready to navigate to Roark.";
}

void AutoStory_checkpoint_0102::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0102(env, context, options, stats);
}


void checkpoint_0102(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            oreburgh_gate_to_mine(env, env.console, context);
            oreburgh_mine_nav(env.console, context);
            // Standalone test entry points are not part of the checkpoint route.
            // oreburgh_mine_first_leg_test(env.console, context);
            // oreburgh_mine_route_and_escape_test(env.console, context);
            oreburgh_mine_to_gym(env, env.console, context);
        },

        true);
}

// ========================
// Checkpoint 0103 Wrapper
// ========================

std::string AutoStory_checkpoint_0103::name() const {
    return "01.03 - Oreburgh Gym";
}
std::string AutoStory_checkpoint_0103::start_text() const {
    return AutoStory_checkpoint_0102().end_text();
}
std::string AutoStory_checkpoint_0103::end_text() const {
    return "Standing at Roark after defeating him and acquiring the first badge.";
}

void AutoStory_checkpoint_0103::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    checkpoint_0103(env, context, options, stats);
}


void checkpoint_0103(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            //#. Oreburgh Gym navigation and battle
            gym_Puzzle_01(env, env.console, context, stats);
        },

        true);
}


}
}
}
