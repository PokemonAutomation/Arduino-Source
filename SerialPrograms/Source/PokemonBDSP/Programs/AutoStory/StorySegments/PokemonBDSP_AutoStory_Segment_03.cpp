
/*  BDSP AutoStory - Segment 03
 *  From after acquiring the second badge until after acquiring the third badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <cmath>
#include <functional>
#include <vector>

#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Shopping.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_Autostory_Segment_01_tools.h"
#include "PokemonBDSP/Programs/AutoStory/Utils/PokemonBDSP_OverworldMatchers.h"

#include "PokemonBDSP_AutoStory_Segment_02.h"

#include "PokemonBDSP_AutoStory_Segment_03.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {
static void segment03_finish_transition(VideoStream& stream, ProControllerContext& context, const std::string& label){
    BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
    if (wait_until(stream, context, 30s, {{visible}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label + ": Transition did not finish.",
            stream);
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
}

// Stationary destination confirmation: do not steer during an NPC reward
// scene or declare arrival from a fade that has only just started.
static void segment03_confirm_location(VideoStream& stream,
    ProControllerContext& context,
    OverworldMatchResult (*matcher)(const ImageRGB32&),
    const std::string& label,
    size_t observations = 20,
    bool advance_dialogue = false,
    bool require_target = false,
    OverworldMatchResult (*instruction_card)(const ImageRGB32&) = nullptr){
    context.wait_for_all_requests();
    size_t confirmations = 0;
    for (size_t observation = 0; observation < observations; observation++){
        const auto frame = stream.video().snapshot();
        // Instruction cards require B; A cannot dismiss the Bike Riding card.
        // Require the grayscale Bike illustration and language-independent B icon.
        if (frame && instruction_card && instruction_card(*frame.frame).located){
            confirmations = 0;
            stream.log("[" + label + "] Bike illustration and B button icon detected; dismissing instruction card.");
            pbf_press_button(context, BUTTON_B, 80ms, 250ms);
            context.wait_for_all_requests();
            continue;
        }
        const auto match = frame ? matcher(*frame.frame) : OverworldMatchResult{};
        if (require_target ? match.at_target : match.located){
            if (++confirmations >= 3){
                stream.log("[" + label + "] Scene confirmed.", COLOR_GREEN);
                return;
            }
            pbf_wait(context, 250ms);
        }else{
            confirmations = 0;
            if (advance_dialogue){
                pbf_press_button(context, BUTTON_A, 150ms, 500ms);
            }else{
                pbf_wait(context, 500ms);
            }
        }
        context.wait_for_all_requests();
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        label + ": Expected scene not confirmed.",
        stream);
}

// Watch the approach itself, cancelling queued movement as soon as the
// trainer speaks or a battle starts. The existing battle engine owns combat.
static void segment03_expected_encounter(VideoStream& stream,
    ProControllerContext& context, AutoStoryStats& stats,
    const std::string& label,
    const std::function<void(ProControllerContext&)>& approach,
    Milliseconds encounter_wait = 45000ms,
    bool require_encounter = true){
    context.wait_for_all_requests();
    ShortDialogWatcher dialogue;
    StartBattleDetector starting(stream.overlay());
    BattleMenuWatcher menu(BattleType::TRAINER);
    const int encountered = run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
            approach(ctx);
            pbf_wait(ctx, encounter_wait);
        },
        {{dialogue}, {starting}, {menu}});
    if (encountered < 0){
        if (!require_encounter){
            stream.log("[" + label + "] No encounter within " + std::to_string(encounter_wait.count()) +
                "ms after approach; trainer dodged, continuing.",
                COLOR_GREEN);
            return;
        }
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label + ": Expected trainer encounter did not start.",
            stream);
    }
    stream.log("[" + label + "] Encounter detected; route movement cancelled.");
    fight_trainer_using_moves(stream, context, stats, label, false);
}

static void segment03_wait_image(VideoStream& stream,
    ProControllerContext& context,
    const std::string& resource,
    const ImageFloatBox& box,
    const std::string& label){
    auto reference = std::make_shared<ImageRGB32>(RESOURCE_PATH() + resource);
    ImageMatchWatcher ready(reference, box, 25.0, false, 250ms);
    if (wait_until(stream, context, 5s, {{ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label + ": Visual confirmation missing.",
            stream);
    }
}

static void segment03_register_bike(VideoStream& stream, ProControllerContext& context, Language language){
    // Receiving Bike adds new flags after the earlier pre-shop Bag pass.
    if (!clear_new_item_flags(stream, context)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Eterna Bike: Could not clear reward flags.",
            stream);
    }
    BagItemTemplate bike;
    bike.reference_path = "PokemonBDSP/Autostory/EternaBikeShop/BikeBag.png";
    bike.label = "bike";
    bike.reference_row = 4;
    if (!find_bag_item(stream, context, bike, 8, 8, 128, 60s, false, language)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Eterna Bike: Bike selection not confirmed.",
            stream);
    }
    // Locate the Key Items tab and Bike first, then use the fixed Register
    // sequence without depending on popup, wheel, or confirmation images.
    pbf_press_button(context, BUTTON_A, 150ms, 500ms);
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 250ms);
    pbf_press_button(context, BUTTON_A, 150ms, 500ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 250ms);
    pbf_press_button(context, BUTTON_A, 150ms, 500ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    stream.log("[Eterna Bike] Registration inputs sent; preserving the reward's mounted state.", COLOR_GREEN);
}

// Stop at a completed doorway transition so remaining outdoor movement cannot run inside the next
// map. Cancel at either fade color and resume only after the fade clears. Recover the descent
// endpoint before moving east. Correct height while still west of the honey-tree collision
// column.
static void segment03_align_lower_stair_lane(VideoStream& stream, ProControllerContext& context){
    size_t confirmations = 0, missing = 0;
    for (size_t observation = 0; observation < 40; observation++){
        context.wait_for_all_requests();
        const auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 208 lower stair lane: No video frame.",
                stream);
        }
        const auto match = match_route208_lower_stair_lane(*frame.frame);
        stream.log("[Route 208 lower stair lane] located=" + std::to_string(match.located) +
            ", target=" + std::to_string(match.at_target) + ", offset=(" + std::to_string(match.dx) + "," +
            std::to_string(match.dy) + ")");
        if (!match.located){
            confirmations = 0;
            if (++missing > 6){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 208 lower stair lane: Cliff/tree anchors missing; no blind search.",
                    stream);
            }
            pbf_wait(context, 500ms);
            continue;
        }
        missing = 0;
        if (match.at_target && !match.coarse){
            if (++confirmations >= 3){
                return;
            }
            pbf_wait(context, 250ms);
            continue;
        }
        confirmations = 0;
        const bool vertical = match.coarse || std::abs(match.dy) > 1.0 / 54;
        const double error = vertical ? match.dy * 9.0 / 16 : match.dx;
        const auto duration = std::abs(error) > 1.0 / 24 ? 80ms : std::abs(error) > 1.0 / 64 ? 40ms : 16ms;
        pbf_move_left_joystick(context,
            {vertical ? 0.0 : (error > 0 ? 1.0 : -1.0), vertical ? (error < 0 ? 1.0 : -1.0) : 0.0},
            duration,
            500ms);
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        "Route 208 lower stair lane: Could not confirm the safe descent endpoint.",
        stream);
}

static bool segment03_cycling_move(VideoStream& stream,
    ProControllerContext& context, AutoStoryStats& stats,
    double x,
    double y,
    Milliseconds duration,
    Milliseconds rest,
    const std::string& label,
    bool require_transition = false){
    context.wait_for_all_requests();
    BlackScreenWatcher black;
    WhiteScreenWatcher white;
    BattleMenuWatcher battle(BattleType::TRAINER);
    const int result = run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {x, y}, duration, rest);
        },
        {{black}, {white}, {battle}});
    if (result == 2){
        fight_trainer_using_moves(stream, context, stats, label + " encounter", false);
        return false;
    }
    if (result < 0){
        if (require_transition){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label + ": Transition did not start.",
                stream);
        }
        return false;
    }
    BlackScreenWatcher black_gone(COLOR_RED,
        {0.1, 0.1, 0.8, 0.8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    WhiteScreenWatcher white_gone(COLOR_RED,
        {0.1, 0.1, 0.8, 0.8},
        500,
        10,
        WhiteScreenWatcher::FinderType::GONE,
        500ms);
    const int finished = result == 1 ? wait_until(stream, context, 30s, {{white_gone}})
        : wait_until(stream, context, 30s, {{black_gone}});
    if (finished != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label + ": Transition did not finish.",
            stream);
    }
    // A cleared fade can precede restored movement control.
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    return true;
}

// Returns true if an encounter began during alignment. The battle strip
// then owns it and counts completion without replaying entry movement.
static bool segment03_align_cycle_start(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    size_t confirmed = 0, missing = 0;
    bool have_pose = false;
    double previous_dx = 0, previous_dy = 0;
    const WallClock deadline = current_time() + 30s;
    for (size_t observation = 0; current_time() < deadline; observation++){
        const auto frame = stream.video().snapshot();
        const auto match = frame ? match_cycling_road_bridge_approach(*frame.frame) : OverworldMatchResult{};
        const bool stable = match.located && have_pose && std::abs(match.dx - previous_dx) <= 1.0 / 192 &&
            std::abs(match.dy - previous_dy) <= 1.0 / 108;
        std::string diagnostic = "[Cycling Road bridge] observation=" + std::to_string(observation + 1) +
            " located=" + std::to_string(match.located) + " target=" + std::to_string(match.at_target) +
            " stable=" + std::to_string(stable) +
            " offset=" + std::to_string(match.dx) + "," + std::to_string(match.dy);
        for (const auto& anchor : match.landmarks){
            diagnostic += " [" + anchor.name + " coverage=" + std::to_string(anchor.score) +
                " accepted=" + std::to_string(anchor.accepted) + "]";
        }
        stream.log(diagnostic);
        double x = 0, y = 0;
        Milliseconds hold = 0ms, rest = 250ms;
        if (!match.located){
            confirmed = 0;
            have_pose = false;
            if (++missing >= 6){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Cycling Road: Bridge deck and right rail not acquired.",
                    stream);
            }
            rest = 500ms;
        }else{
            missing = 0;
            if (std::abs(match.dx) > .32 || std::abs(match.dy) > .60){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Cycling Road: Bridge alignment offset exceeds bounds.",
                    stream);
            }
            if (!stable){
                // The camera keeps easing after the bicycle stops. Require
                // two agreeing observations before every new correction.
                confirmed = 0;
            }else if (match.at_target){
                if (++confirmed >= 3){
                    stream.log(
                        "[Cycling Road] Upper white platform/right rail pose confirmed; starting watched downhill entry.",
                        COLOR_GREEN);
                    return false;
                }
            }else{
                confirmed = 0;
                // Align against the rail on the stationary upper white platform, before the
                // downhill section.
                if (std::abs(match.dx) > .012){
                    x = match.dx > 0 ? 1 : -1;
                    // Use long pulses while far from the rail, then shorten them
                    // before the next settled observation to avoid overshoot.
                    const double distance = std::abs(match.dx);
                    if (distance > .12){
                        hold = 800ms;
                    }else if (distance > .09){
                        hold = 640ms;
                    }else if (distance > .04){
                        hold = 320ms;
                    }else{
                        hold = 160ms;
                    }
                }else if (std::abs(match.dy) > 1.0 / 54){
                    y = match.dy > 0 ? -1 : 1;
                    const double distance = std::abs(match.dy);
                    if (distance > .20){
                        hold = 800ms;
                    }else if (distance > .12){
                        hold = 640ms;
                    }else if (distance > .06){
                        hold = 320ms;
                    }else{
                        hold = 200ms;
                    }
                }
            }
            previous_dx = match.dx;
            previous_dy = match.dy;
            // A movement invalidates the old pose. Reacquire two fresh
            // settled measurements before another movement is allowed.
            have_pose = hold == 0ms;
        }
        BlackScreenWatcher black;
        WhiteScreenWatcher white;
        ShortDialogWatcher dialogue;
        StartBattleDetector starting(stream.overlay());
        BattleMenuWatcher menu(BattleType::TRAINER);
        const int interrupted = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                if (hold > 0ms){
                    pbf_move_left_joystick(ctx, {x, y}, hold, rest);
                }else{
                    pbf_wait(ctx, rest);
                }
                ctx.wait_for_all_requests();
            },
            {{black}, {white}, {dialogue}, {starting}, {menu}});
        if (interrupted >= 2){
            stream.log("[Cycling Road] Encounter during bridge approach; alignment movement cancelled.");
            return true;
        }
        if (interrupted >= 0){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Cycling Road: Unexpected transition during bridge alignment.",
                stream);
        }
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        "Cycling Road: Bridge lane alignment did not converge within 30 seconds.",
        stream);
}


static void segment03_cycle_road_battles(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, bool encounter_pending){
    size_t completed = 0;
    auto watch = [&](bool initial){
        context.wait_for_all_requests();
        ShortDialogWatcher dialogue;
        StartBattleDetector starting(stream.overlay());
        BattleMenuWatcher menu(BattleType::TRAINER);
        return run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                if (initial){
                    pbf_move_left_joystick(ctx, {0, -1}, 3300ms, 0ms);
                }
                // From the first encounter onward the controller stays
                // neutral. The bicycle rolls naturally into the next trainer.
                pbf_wait(ctx, completed >= 3 ? 7000ms : 45000ms);
                ctx.wait_for_all_requests();
            },
            {{dialogue}, {starting}, {menu}});
    };
    while (completed < 4){
        const int result = encounter_pending ? 0 : watch(completed == 0);
        encounter_pending = false;
        if (result < 0){
            if (completed >= 3){
                stream.log("[Cycling Road] Three battles complete; 7s without another encounter. Continuing south.");
                return;
            }
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Cycling Road: Expected at least three battles before the south gate.",
                stream);
        }
        fight_trainer_using_moves(stream, context, stats, "Cycling Road encounter " + std::to_string(completed + 1), false);
        ++completed;
        stream.log("[Cycling Road] Completed battles: " + std::to_string(completed) + "/4.");
    }
    stream.log("[Cycling Road] Four battles complete; skipping the quiet wait.");
}

static void segment03_doorway(VideoStream& stream,
    ProControllerContext& context,
    bool north,
    std::chrono::milliseconds duration,
    const std::string& label,
    double horizontal = 0){
    context.wait_for_all_requests();
    // Horizontal HQ stairs must cancel at fade-out, before loading can
    // swallow or carry movement onto the next floor. Vertical door callers
    // retain their existing transition behavior.
    if (horizontal != 0){
        BlackScreenWatcher entering;
        if (run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                    pbf_move_left_joystick(ctx, {horizontal, 0}, duration, 3000ms);
                },
            {{entering}}) != 0){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label + ": Stair transition did not start.",
                stream);
        }
        BlackScreenWatcher visible(COLOR_RED,
            {0.1, 0.1, 0.8, 0.8},
            100,
            10,
            BlackScreenWatcher::FinderType::GONE,
            500ms);
        if (wait_until(stream, context, 30s, {{visible}}) != 0){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                label + ": Stair transition did not finish.",
                stream);
        }
        pbf_wait(context, 500ms);
        context.wait_for_all_requests();
        return;
    }
    BlackScreenOverWatcher transition(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int result = run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {0, north ? 1.0 : -1.0}, duration, 3000ms);
        },
        {{transition}});
    if (result != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            label + ": Doorway transition not confirmed.",
            stream);
    }
    segment03_finish_transition(stream, context, label);
}

void eterna_gym_to_jupiter(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1060ms, 225ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 4232ms, 269ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 287ms, 1777ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 367ms, 237ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1140ms, 2621ms);
    if (!heal_pokemon(stream, context, "Eterna before Galactic HQ", &stats)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Eterna Center exit was not confirmed.",
            stream);
    }
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1895ms, 130ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 710ms, 921ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 129ms, 354ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 813ms, 565ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 618ms, 483ms);
    context.wait_for_all_requests();
    // Align with the sign and Center emblem before trying Cut. Never search blindly or
    // keep pushing north against the hedge when the tree is east of us.
    size_t hq_cut_confirmations = 0, hq_cut_missing = 0;
    for (size_t correction = 0; correction < 20; correction++){
        const auto frame = stream.video().snapshot();
        const auto position = frame ? match_galactic_hq_cut(*frame.frame) : OverworldMatchResult{};
        for (const auto& anchor : position.landmarks){
            stream.log("[Galactic HQ Cut] " + anchor.name + " score=" + std::to_string(anchor.score) +
                ", margin=" + std::to_string(anchor.margin) + ", offset=(" + std::to_string(anchor.dx) + "," +
                std::to_string(anchor.dy) + ")");
        }
        if (!position.located){
            hq_cut_confirmations = 0;
            if (++hq_cut_missing > 6){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Galactic HQ: Front bush sign/Center alignment not confirmed.",
                    stream);
            }
            // Let a passing shadow clear; never search by moving blindly.
            pbf_wait(context, 500ms);
            context.wait_for_all_requests();
            continue;
        }
        hq_cut_missing = 0;
        if (std::abs(position.dx) > .20){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Galactic HQ: Front bush alignment offset exceeds bounds.",
                stream);
        }
        if (position.at_target){
            if (++hq_cut_confirmations >= 3){
                break;
            }
        }else{
            hq_cut_confirmations = 0;
            pbf_move_left_joystick(context,
                {position.dx > 0 ? 1.0 : -1.0, 0},
                std::abs(position.dx) > 1.0 / 24 ? 80ms : 40ms,
                0ms);
        }
        pbf_wait(context, 250ms);
        context.wait_for_all_requests();
    }
    if (hq_cut_confirmations < 3){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Galactic HQ: Front bush column did not converge.",
            stream);
    }
    // Some attempts reach the bush before the interaction accepts input.
    // Confirm its dialogue while stationary, then finish the Cut animation.
    ShortDialogWatcher hq_cut_dialogue;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                for (size_t press = 0; press < 30; press++){
                    pbf_move_left_joystick(ctx, {0, 1}, 80ms, 100ms);
                    pbf_press_button(ctx, BUTTON_A, 150ms, 500ms);
                }
            },
        {{hq_cut_dialogue}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Galactic HQ: Front bush interaction did not produce dialogue.",
            stream);
    }
    clear_dialog(stream, context, BUTTON_A, 6000ms);
    context.wait_for_all_requests();
    // The Center exit can leave the player one tile west of the HQ doorway. Confirm entry before
    // starting the interior route.
    BlackScreenWatcher hq_entering;
    const int hq_entry = run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {0, 1}, 1507ms, 193ms);
            ctx.wait_for_all_requests();
            for (size_t correction = 0; correction < 4; correction++){
                stream.log("[Galactic HQ entry] No transition; correcting east toward doorway, attempt " +
                    std::to_string(correction + 1) + "/4.");
                pbf_move_left_joystick(ctx, {1, 0}, 80ms, 100ms);
                pbf_move_left_joystick(ctx, {0, 1}, 350ms, 250ms);
                ctx.wait_for_all_requests();
            }
            pbf_wait(ctx, 1500ms);
        },
        {{hq_entering}});
    if (hq_entry != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Galactic HQ: Doorway transition not detected after bounded east correction.",
            stream);
    }
    stream.log("[Galactic HQ entry] Doorway transition detected; waiting for interior.");
    BlackScreenWatcher hq_visible(COLOR_RED,
        {0.1, 0.1, 0.8, 0.8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    if (wait_until(stream, context, 30s, {{hq_visible}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Galactic HQ: Entry transition did not finish.",
            stream);
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    // galacticHQ.txt starts at the ground-floor entrance mat.
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1247ms, 237ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1246ms, 2989ms);
    pbf_press_button(context, BUTTON_A, 150ms, 250ms);
    fight_trainer_using_moves(stream, context, stats, "Galactic HQ entrance grunts", false);
    // With one usable Pokemon these grunts battle consecutively. The next
    // intro can outlast the ordinary dialogue quiet period. Keep movement
    // stopped and reuse the battle handler only if its command menu appears.
    // A party that fought both together needs no second battle handler.
    BattleMenuWatcher hq_next_grunt(BattleType::TRAINER);
    const int hq_next_battle = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            pbf_mash_button(ctx, BUTTON_B, 30000ms);
        },
        {{hq_next_grunt}});
    if (hq_next_battle == 0){
        stream.log("[Galactic HQ] Second entrance grunt detected before stair movement.");
        fight_trainer_using_moves(stream, context, stats, "Galactic HQ second entrance grunt", false);
    }
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 580ms, 220ms);
    segment03_doorway(stream, context, true, 1077ms, "Galactic HQ stairs 1F to 2F", -1.0);
    pbf_wait(context, 1145ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 758ms, 487ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1633ms, 254ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 925ms, 188ms);
    segment03_doorway(stream, context, true, 1145ms, "Galactic HQ stairs 2F to 3F", -1.0);
    pbf_wait(context, 1455ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 410ms, 679ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 744ms, 2084ms);
    pbf_press_button(context, BUTTON_A, 150ms, 250ms);
    fight_trainer_using_moves(stream, context, stats, "Galactic HQ upper-floor grunt", false);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2119ms, 903ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 263ms, 984ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 342ms, 1806ms);
    pbf_press_button(context, BUTTON_A, 150ms, 250ms);
    fight_trainer_using_moves(stream, context, stats, "Galactic HQ Scientist", false);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 425ms, 161ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 723ms, 1639ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 494ms, 678ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 409ms, 858ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 389ms, 1189ms);
    // Align directly above Jupiter, then turn south to interact without
    // trying to enter the narrow gap between her and the rescued NPC.
    align_to_overworld_landmarks(stream, context, match_galactic_hq_jupiter, "Galactic HQ Jupiter", false, true);
    // Use an explicit Down press: the 16ms stick pulse can leave the
    // previous right-facing direction active and interact with Clefairy.
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 250ms);
    context.wait_for_all_requests();
    // Advance the introduction before entering the battle handler's
    // B-based waiting loop. A single press may only open the first text.
    BattleMenuWatcher jupiter_menu(BattleType::TRAINER);
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                for (size_t press = 0; press < 30; press++){
                    pbf_press_button(ctx, BUTTON_A, 150ms, 500ms);
                }
                pbf_wait(ctx, 15000ms);
            },
        {{jupiter_menu}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Galactic HQ: Jupiter interaction did not reach the battle menu.",
            stream);
    }
    fight_trainer_using_moves(stream, context, stats, "Commander Jupiter", true);
    // Jupiter's departure and the rescued family have chained dialogue.
    clear_dialog(stream, context, BUTTON_A, 6000ms);
    stream.log("[Galactic HQ] Rescue dialogue cleared; following recorded exit.", COLOR_GREEN);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 691ms, 575ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 330ms, 2575ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 523ms, 239ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 717ms, 1721ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 306ms, 294ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1041ms, 307ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 431ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, 1.000000}, 11ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 999ms, 573ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 343ms, 175ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 999ms, 456ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 277ms, 122ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 654ms, 1092ms);
    context.wait_for_all_requests();
    BlackScreenOverWatcher hq_leaving(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, -1}, 2350ms, 0ms);
                pbf_move_left_joystick(ctx, {-1, -1}, 2744ms, 3000ms);
            },
        {{hq_leaving}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Galactic HQ: Exit transition not confirmed.",
            stream);
    }
    segment03_finish_transition(stream, context, "Galactic HQ exit");
    // Clamp against the uncut front bush; checkpoint 03.02 starts here.
    pbf_move_left_joystick(context, {0, -1}, 350ms, 500ms);
    context.wait_for_all_requests();
    stream.log("[Galactic HQ] Outside, facing the front Cut bush.", COLOR_GREEN);
}
static void eterna_mart_escape_ropes(VideoStream& stream, ProControllerContext& context, Language language){
    stream.log("[Eterna Mart] Buying exactly 3 Escape Ropes after the Jupiter heal.");
    // Complete the eighteen-step south leg from the Center exit.
    // The caller has already cleared the doorway with three steps.
    DpadState dpad;
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 15);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4);
    segment03_doorway(stream, context, true, 2000ms, "Eterna Mart entrance");
    // Standard Mart layout: walk north from the entrance to the clerk.
    pbf_move_left_joystick(context, {0, 1}, 885ms, 300ms);
    if (!buy_items(stream, context, {{ShopItem::EscapeRope, 3}}, language, DPAD_UP)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Eterna Mart: Could not buy 3 Escape Ropes.",
            stream);
    }
    segment03_doorway(stream, context, false, 5000ms, "Eterna Mart exit");
}

static void segment03_enter_cycle_shop(VideoStream& stream, ProControllerContext& context){
    // Start outside the Mart facing south; pass west of it to the shop's front lane.
    DpadState dpad;
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 4);
    repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 8);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);
    segment03_doorway(stream, context, true, 2000ms, "Eterna bicycle shop entrance");
    segment03_confirm_location(stream, context, match_eterna_cycle_shop_interior, "Eterna bicycle shop interior");
    stream.log("[Eterna bicycle shop] Interior confirmed after the route from the Mart.", COLOR_GREEN);
}

void jupiter_to_coronet(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, Language language){
    // Continue from 03.01 outside HQ, directly in front of the Cut bush.
    use_HM(stream, context, "Galactic HQ exit Cut");
    clear_dialog(stream, context, BUTTON_A, 6000ms);
    DpadState dpad;
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);
    repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 5);
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 9);
    repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 5);
    segment03_doorway(stream, context, true, 2000ms, "Eterna Center entrance after Jupiter");
    if (!heal_pokemon(stream, context, "Eterna after Jupiter", &stats)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Eterna Center exit not confirmed.",
            stream);
    }
    segment03_finish_transition(stream, context, "Eterna Center heal exit");
    // The exit fixes the column and faces south; clear the doorway before turning.
    pbf_wait(context, 500ms);
    dpad.last_dir = DPAD_DOWN;
    repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 3);
    context.wait_for_all_requests();
    stream.log("[Eterna Center departure to Mart] Cleared doorway with three south D-pad steps.");
    eterna_mart_escape_ropes(stream, context, language);
    segment03_enter_cycle_shop(stream, context);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 505ms, 405ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 568ms, 403ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 642ms, 14ms);
    // The approach reaches the keeper even if the exact counter pose differs. Start talking
    // before waiting for the reward. Cancel Up immediately on dialogue; use only A afterward.
    context.wait_for_all_requests();
    ShortDialogWatcher keeper_dialogue;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                for (size_t press = 0; press < 20; press++){
                    pbf_move_left_joystick(ctx, {0, 1}, 80ms, 100ms);
                    pbf_press_button(ctx, BUTTON_A, 150ms, 500ms);
                }
            },
        {{keeper_dialogue}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Eterna bicycle shop: Keeper interaction did not produce dialogue.",
            stream);
    }
    // The reward scene returns us outside already mounted. Wait for that
    // positive destination instead of replaying a fixed dialogue burst.
    segment03_confirm_location(stream,
        context,
        match_eterna_cycle_shop_reward,
        "Eterna bicycle reward complete",
        180,
        true,
        true,
        match_eterna_bike_riding_card);
    segment03_register_bike(stream, context, language);
    segment03_cycling_move(stream, context, stats, -1, 0, 733ms, 316ms, "Cycling Road approach");
    segment03_cycling_move(stream, context, stats, 0, -1, 3280ms, 3000ms, "Cycling Road north gate entrance", true);
    segment03_cycling_move(stream, context, stats, 0, -1, 1789ms, 3000ms, "Cycling Road north gate exit", true);
    // Fade completion can precede restored player control. Settle and
    // clear the steps before acquiring the bridge, not the cliff wall.
    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
    OverworldWatcher north_gate_overworld;
    if (wait_until(stream, context, 10s, {{north_gate_overworld}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Cycling Road: Overworld not visible after north gate exit settle.",
            stream);
    }
    BlackScreenWatcher north_gate_black;
    WhiteScreenWatcher north_gate_white;
    ShortDialogWatcher north_gate_dialogue;
    StartBattleDetector north_gate_starting(stream.overlay());
    BattleMenuWatcher north_gate_menu(BattleType::TRAINER);
    const int clearance = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {0, -1}, 1200ms, 250ms);
            ctx.wait_for_all_requests();
        },
        {{north_gate_black}, {north_gate_white}, {north_gate_dialogue}, {north_gate_starting}, {north_gate_menu}});
    if (clearance == 0 || clearance == 1){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Cycling Road: Unexpected transition while clearing north gate steps.",
            stream);
    }
    const bool cycling_encounter = clearance >= 2 || segment03_align_cycle_start(stream, context);
    segment03_cycle_road_battles(stream, context, stats, cycling_encounter);
    // Any approach movement may already enter the south gate. Preserve
    // that transition instead of requesting two more doorway crossings.
    bool south_gate_entered =
        segment03_cycling_move(stream, context, stats, -1, 0, 129ms, 1499ms, "Cycling Road south gate approach");
    if (!south_gate_entered){
        south_gate_entered =
            segment03_cycling_move(stream, context, stats, -1, 0, 361ms, 3345ms, "Cycling Road south gate approach");
    }
    if (!south_gate_entered){
        south_gate_entered =
            segment03_cycling_move(stream, context, stats, 0, -1, 472ms, 2574ms, "Cycling Road south gate approach");
    }
    if (!south_gate_entered){
        segment03_cycling_move(stream, context, stats, 0, -1, 1977ms, 1544ms, "Cycling Road south gate entrance", true);
    }
    segment03_confirm_location(stream,
        context,
        match_cycling_road_south_gate_interior,
        "Cycling Road south gate interior");
    segment03_cycling_move(stream, context, stats, 0, -1, 1977ms, 1544ms, "Cycling Road south gate exit", true);
    OverworldWatcher south_gate_overworld;
    if (wait_until(stream, context, 10s, {{south_gate_overworld}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Cycling Road: Overworld not visible after south gate exit.",
            stream);
    }
    // Stay southbound until the scripted Dawn/Lucas dialogue actually
    // appears. Cancel movement at the dialogue instead of spending inputs
    // against the cutscene, and never begin the eastward trainer route early.
    bool dawn_dialogue_seen = false;
    for (size_t attempt = 0; attempt < 4 && !dawn_dialogue_seen; attempt++){
        ShortDialogWatcher dawn_dialogue;
        // A dialogue already open after the transition needs no movement.
        if (wait_until(stream, context, 500ms, {{dawn_dialogue}}) == 0){
            dawn_dialogue_seen = true;
            break;
        }
        BlackScreenWatcher dawn_black;
        WhiteScreenWatcher dawn_white;
        StartBattleDetector dawn_starting(stream.overlay());
        BattleMenuWatcher dawn_menu(BattleType::TRAINER);
        const Milliseconds south = attempt == 0 ? 1546ms : 3000ms;
        stream.log("[Route 207 Dawn] South approach attempt=" + std::to_string(attempt + 1));
        const int result = run_until<ProControllerContext>(stream,
            context,
            [south](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, -1}, south, 500ms);
                ctx.wait_for_all_requests();
            },
            {{dawn_dialogue}, {dawn_black}, {dawn_white}, {dawn_starting}, {dawn_menu}});
        if (result == 0){
            dawn_dialogue_seen = true;
        }else if (result >= 0){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 207 Dawn: Unexpected transition or battle before dialogue.",
                stream);
        }
    }
    if (!dawn_dialogue_seen){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 207 Dawn: Dialogue not encountered after southward retries.",
            stream);
    }
    stream.log("[Route 207 Dawn] Dialogue detected; south movement cancelled.", COLOR_GREEN);
    bool dawn_dialogue_cleared = false;
    for (size_t pass = 0; pass < 6 && !dawn_dialogue_cleared; pass++){
        // A advances the conversation and accepts its default choice.
        // Reuse the existing dialogue helper, then guard against a brief
        // gap between text boxes being mistaken for the end of the scene.
        mash_until_dialogue_ends(stream, context, BUTTON_A, 60s);
        context.wait_for_all_requests();
        ShortDialogWatcher resumed_dialogue;
        if (wait_until(stream, context, 2000ms, {{resumed_dialogue}}) == 0){
            continue;
        }
        const auto frame = stream.video().snapshot();
        OverworldDetector overworld;
        dawn_dialogue_cleared = frame && overworld.detect(*frame.frame);
    }
    if (!dawn_dialogue_cleared){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 207 Dawn: Dialogue did not remain cleared.",
            stream);
    }
    pbf_wait(context, 6000ms);
    stream.log("[Route 207 Dawn] Dialogue cleared; continuing to Camper Anthony.", COLOR_GREEN);
    // Start after Dawn. Encounter watchers and the move-2 battle handler manage battles and
    // menus.
    segment03_expected_encounter(stream, context, stats, "Route 207 Camper Anthony", [](ProControllerContext& ctx){
        pbf_move_left_joystick(ctx, {0, -1}, 1013ms, 1667ms);
        pbf_move_left_joystick(ctx, {1, 0}, 3309ms, 1011ms);
    });
    // The approach triggers Lauren without further steering.
    segment03_expected_encounter(stream, context, stats, "Route 207 Picnicker Lauren", [](ProControllerContext&){
    });
    // Justin can be dodged. After the approach, wait three seconds for an encounter; otherwise
    // continue toward Coronet.
    segment03_expected_encounter(
        stream,
        context, stats,
        "Route 207 Hiker Justin",
        [](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {1, 0}, 994ms, 508ms);
            pbf_move_left_joystick(ctx, {0, 1}, 534ms, 457ms);
            pbf_move_left_joystick(ctx, {1, 0}, 2076ms, 149ms);
        },
        3000ms,
        false);
    // Continue after Justin, then use the watched down/right doorway search from Oreburgh Gate
    // for the final entry.
    bool coronet_entered = segment03_cycling_move(stream, context, stats, 1, 0, 942ms, 443ms, "Mount Coronet approach");
    if (!coronet_entered){
        coronet_entered = segment03_cycling_move(stream, context, stats, 0, -1, 776ms, 119ms, "Mount Coronet approach");
    }
    if (!coronet_entered){
        coronet_entered = segment03_cycling_move(stream, context, stats, 1, 0, 627ms, 147ms, "Mount Coronet approach");
    }
    if (!coronet_entered){
        coronet_entered = segment03_cycling_move(stream, context, stats, 0, 1, 725ms, 1008ms, "Mount Coronet approach");
    }
    if (!coronet_entered){
        enter_cave_with_down_right_search(stream, context, "Mount Coronet west entrance");
        segment03_finish_transition(stream, context, "Mount Coronet west entrance");
        // finish_transition includes 500ms; allow two seconds in total.
        pbf_wait(context, 1500ms);
        context.wait_for_all_requests();
    }
    OverworldWatcher coronet_overworld;
    if (wait_until(stream, context, 10s, {{coronet_overworld}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Mount Coronet: Interior not ready after west entrance.",
            stream);
    }
    context.wait_for_all_requests();
}
void coronet1(VideoStream& stream, ProControllerContext& context, Language language){
    activate_repel(stream, context, language, 5);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 2190ms, 104ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 852ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, 1.000000}, 37ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 102ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, 1.000000}, 78ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 983ms, 0ms);
    wait_for_dialog(stream, context, "Mount Coronet Cyrus introduction", 15s);
    // Allow Cyrus to walk away after the last text box before resuming the route.
    clear_dialog(stream, context, ClearDialogMode::STOP_OVERWORLD, 60, BUTTON_A, 8000ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 999ms, 268ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 375ms, 227ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 741ms, 46ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 743ms, 68ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 265ms, 255ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 890ms, 88ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 462ms, 271ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 647ms, 20ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1047ms, 56ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 776ms, 2038ms);
    context.wait_for_all_requests();
}
void coronet_to_heartholme(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, Language language){
    // The expiry routine starts west. Step east first to leave room for
    // its west/east pairs, then clear the old Repel before route movement.
    context.wait_for_all_requests();
    pbf_wait(context, 250ms);
    pbf_move_left_joystick(context, {1, 0}, 1000ms, 250ms);
    context.wait_for_all_requests();
    // Leave Repel off during the potentially long Hiker sight-line sweeps.
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);

    // Either Hiker may interrupt the crossing. Resume toward the east
    // wall after each fight. The wiggles are one encounter-triggering
    // attempt: once started, do not restart or finish them after an
    // interruption. Keep the resumed east approach watched for a later Hiker.
    bool bridge_approach_clear = false, bridge_wiggle_started = false;
    size_t bridge_battles = 0, approach_dialogues = 0;
    for (size_t pass = 0; pass < 6 && !bridge_approach_clear; pass++){
        ShortDialogWatcher dialogue;
        StartBattleDetector starting(stream.overlay());
        BattleMenuWatcher menu(BattleType::TRAINER);
        const int engaged = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {1, 0}, 4828ms, 104ms);
                ctx.wait_for_all_requests();
                pbf_wait(ctx, 3000ms);
                ctx.wait_for_all_requests();
                if (!bridge_wiggle_started){
                    // Mark before the first input: an encounter during
                    // this set fulfills its purpose and consumes the set.
                    bridge_wiggle_started = true;
                    for (size_t pair = 0; pair < 8; pair++){
                        pbf_move_left_joystick(ctx, {0, 1}, 500ms, 150ms);
                        pbf_move_left_joystick(ctx, {0, -1}, 500ms, 150ms);
                        ctx.wait_for_all_requests();
                    }
                }else{
                    stream.log(
                        "[Route 208 bridge] Wiggle set already used; continuing watched east approach without repeating it.");
                }
                pbf_wait(ctx, 2500ms);
            },
            {{dialogue}, {starting}, {menu}});
        if (engaged < 0){
            bridge_approach_clear = true;
            break;
        }
        if (engaged == 0){
            // A short box can be a Repel prompt. Advance it with A (the
            // default renewal choice) while watching for actual combat.
            // Never send a nonbattle choice arrow to the battle engine.
            StartBattleDetector dialogue_starting(stream.overlay());
            BattleMenuWatcher dialogue_menu(BattleType::TRAINER);
            bool dialogue_cleared = false;
            const int battle_started = run_until<ProControllerContext>(stream,
                context,
                [&](ProControllerContext& ctx){
                    mash_until_dialogue_ends(stream, ctx, BUTTON_A, 20s);
                    ctx.wait_for_all_requests();
                    pbf_wait(ctx, 3000ms);
                    ctx.wait_for_all_requests();
                    const auto frame = stream.video().snapshot();
                    OverworldDetector overworld;
                    ShortDialogDetector short_dialogue;
                    dialogue_cleared = frame && overworld.detect(*frame.frame) && !short_dialogue.detect(*frame.frame);
                },
                {{dialogue_starting}, {dialogue_menu}});
            if (battle_started < 0){
                if (!dialogue_cleared || ++approach_dialogues > 2){
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Route 208 bridge: Approach dialogue did not clear within bounded recovery.",
                        stream);
                }
                stream.log("[Route 208 bridge] Nonbattle dialogue cleared; resuming watched crossing.");
                continue;
            }
        }
        fight_trainer_using_moves(stream, context, stats, "Route 208 Hiker", false);
        if (++bridge_battles > 2){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 208 bridge: Unexpected additional Hiker battle.",
                stream);
        }
        pbf_wait(context, 500ms);
        context.wait_for_all_requests();
    }
    if (!bridge_approach_clear){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 208 bridge: Watched east approach did not clear.",
            stream);
    }
    stream.log("[Route 208 bridge] Full east approach clear after " + std::to_string(bridge_battles) +
        " battle(s); continuing toward the stairs.",
        COLOR_GREEN);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 421ms, 178ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1436ms, 81ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1197ms, 298ms);
    // Fixed eastward travel can stop beyond the stair column. Center above
    // the stairs using the native left-rock pose before moving south.
    bool rock_acquisition_timed_out = false;
    align_to_overworld_landmarks(stream,
        context,
        match_route208_stair_top,
        "Route 208 stair-top rocks",
        false,
        true,
        false,
        nullptr,
        nullptr,
        0,
        0,
        0,
        &rock_acquisition_timed_out);
    if (!activate_repel(stream, context, language, 5)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 208 stair top: Repel activation failed.",
            stream);
    }
    context.wait_for_all_requests();
    if (rock_acquisition_timed_out){
        stream.log("[Route 208 stair fallback] Using east 2000ms, southwest 1500ms, then south trial 1650ms.");
        // Watch the entire recovery phase and cancel remaining inputs if a transition or battle
        // starts before the gate approach.
        BlackScreenWatcher fallback_black;
        WhiteScreenWatcher fallback_white;
        BattleMenuWatcher fallback_battle(BattleType::TRAINER);
        const int fallback_interrupted = run_until<ProControllerContext>(stream,
            context,
            [&](ProControllerContext& ctx){
                // East: clamp against the cliff edge.
                pbf_move_left_joystick(ctx, {1, 0}, 2000ms, 0ms);
                ctx.wait_for_all_requests();
                // Southwest: drag toward the stair column.
                pbf_move_left_joystick(ctx, {-1, -1}, 1500ms, 0ms);
                ctx.wait_for_all_requests();
                // Move south for 1650ms. Recovery may start farther down the stairs.
                pbf_move_left_joystick(ctx, {0, -1}, 1650ms, 643ms);
                ctx.wait_for_all_requests();
                segment03_align_lower_stair_lane(stream, ctx);
            },
            {{fallback_black}, {fallback_white}, {fallback_battle}});
        if (fallback_interrupted >= 0){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 208 stair fallback: Unexpected transition or battle before west gate approach.",
                stream);
        }
    }else{
        // Pass above the honey tree, between the southern trainer and Hannah. Validate the 1825ms
        // descent endpoint.
        pbf_move_left_joystick(context, {0.000000, -1.000000}, 1825ms, 643ms);
    }
    // Entry may occur during an earlier approach leg. Once a fade is
    // caught, skip the remaining outdoor approach rather than replaying it
    // inside the gatehouse. The existing watcher cancels at black/white,
    // waits for GONE500ms, then settles for2s before the next input.
    bool hearthome_gate_entered =
        segment03_cycling_move(stream, context, stats, 1, 0, 3500ms, 443ms, "Hearthome west gate approach");
    if (!hearthome_gate_entered){
        hearthome_gate_entered =
            segment03_cycling_move(stream, context, stats, 0, 1, 250ms, 405ms, "Hearthome west gate doorway alignment");
    }
    if (!hearthome_gate_entered){
        hearthome_gate_entered =
            segment03_cycling_move(stream, context, stats, 1, 0, 600ms, 1617ms, "Hearthome west gate entrance", true);
    }
    if (!hearthome_gate_entered){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome west gate: Interior transition not confirmed.",
            stream);
    }
    if (!segment03_cycling_move(stream, context, stats, 1, 0, 1800ms, 1588ms, "Hearthome west gate exit", true)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome west gate: Exterior transition not confirmed.",
            stream);
    }
    OverworldWatcher hearthome_outside;
    if (wait_until(stream, context, 10s, {{hearthome_outside}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome west gate: Overworld not ready after exit.",
            stream);
    }
    if (segment03_cycling_move(stream, context, stats, 1, 0, 2200ms, 6189ms, "Hearthome Buneary approach")){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Buneary approach: Unexpected extra transition.",
            stream);
    }
    // Start the scene deadline after approach requests and neutral waits finish.
    context.wait_for_all_requests();
    ShortDialogWatcher buneary_dialogue;
    if (wait_until(stream, context, 15s, {{buneary_dialogue}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Buneary: Dialogue did not start.",
            stream);
    }
    bool buneary_cleared = false;
    for (size_t pass = 0; pass < 6 && !buneary_cleared; pass++){
        mash_until_dialogue_ends(stream, context, BUTTON_A, 60s);
        context.wait_for_all_requests();
        ShortDialogWatcher resumed_dialogue;
        if (wait_until(stream, context, 2000ms, {{resumed_dialogue}}) == 0){
            continue;
        }
        // Let the NPC finish walking away before any expiration movement.
        pbf_wait(context, 6000ms);
        context.wait_for_all_requests();
        const auto frame = stream.video().snapshot();
        OverworldDetector overworld;
        ShortDialogDetector dialogue;
        buneary_cleared = frame && overworld.detect(*frame.frame) && !dialogue.detect(*frame.frame);
    }
    if (!buneary_cleared){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Buneary: Scene did not clear.",
            stream);
    }
    stream.log("[Hearthome Buneary] Scene cleared; expiring Repel without renewal before Center travel.", COLOR_GREEN);
    pbf_move_left_joystick(context, {1, 0}, 500ms, 0ms);
    context.wait_for_all_requests();
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);
    // Use the whole-approach watcher from the Eterna bicycle shop. Entry during travel or
    // alignment cancels remaining outdoor inputs; confirm the room visually afterward.
    bool center_entered = false;
    BlackScreenWatcher center_transition;
    const int center_entry = run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
            pbf_move_left_joystick(ctx, {1.000000, -0.000000}, 3350ms, 437ms);
            // Clear the northbound lamp column before crossing the plaza.
            pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 250ms, 0ms);
            pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 3000ms, 1582ms);
            pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 1150ms, 334ms);
            // Repel expired before this travel; no blind response inputs remain.
            pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 2250ms, 304ms);
            pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 300ms, 174ms);
            pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 500ms, 0ms);
            pbf_move_left_joystick(ctx, {-1.000000, 1.000000}, 100ms, 0ms);
            pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 1800ms, 242ms);
            pbf_move_left_joystick(ctx, {-1.000000, -0.000000}, 500ms, 0ms);
            pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 250ms, 59ms);
            align_to_overworld_landmarks(stream,
                ctx,
                match_hearthome_center_entrance,
                "Hearthome Center exterior alignment",
                false,
                true,
                false,
                match_hearthome_center_interior,
                &center_entered);
            if (!center_entered){
                pbf_move_left_joystick(ctx, {0, 1}, 4000ms, 3000ms);
            }
        },
        {{center_transition}});
    if (center_entry != 0 && !center_entered){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Center: Entry transition not detected.",
            stream);
    }
    segment03_finish_transition(stream, context, "Hearthome Center entrance");
    pbf_wait(context, 1500ms);
    context.wait_for_all_requests();
    segment03_confirm_location(stream, context, match_hearthome_center_interior, "Hearthome Center interior");
    if (!heal_pokemon(stream, context, "Hearthome", &stats)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Center exit not confirmed.",
            stream);
    }
    // Let the exit scene and camera settle before spending the Hall travel
    // budget. The old east hold began while the exit was still fading in.
    segment03_finish_transition(stream, context, "Hearthome Center heal exit");
    pbf_wait(context, 1500ms);
    context.wait_for_all_requests();
    OverworldWatcher center_exit_ready;
    if (wait_until(stream, context, 10s, {{center_exit_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Center: Exterior not ready after healing.",
            stream);
    }
    pbf_move_left_joystick(context, {1, 0}, 2092ms, 185ms);
    // Bring the Hall into view without the old 1004ms north overshoot.
    pbf_move_left_joystick(context, {0, 1}, 600ms, 500ms);
    context.wait_for_all_requests();
    align_to_overworld_landmarks(stream,
        context,
        match_hearthome_contest_fantina,
        "Hearthome Contest Hall Fantina",
        false,
        true);
    // Repeat the existing short Up/A interaction within the 15s window.
    // Dialogue cancels the remaining inputs before any text cleanup.
    ShortDialogWatcher fantina_dialogue;
    BlackScreenWatcher fantina_black;
    WhiteScreenWatcher fantina_white;
    const int fantina_started = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            constexpr auto pulse = 16ms + 100ms + 194ms + 277ms;
            Milliseconds elapsed = 0ms;
            for (; elapsed + pulse <= 15s; elapsed += pulse){
                pbf_move_left_joystick(ctx, {0, 1}, 16ms, 100ms);
                pbf_press_button(ctx, BUTTON_A, 194ms, 277ms);
                ctx.wait_for_all_requests();
            }
            pbf_wait(ctx, 15s - elapsed);
        },
        {{fantina_dialogue}, {fantina_black}, {fantina_white}});
    if (fantina_started > 0){
        // Fantina's opening camera cut fades to black before the text box.
        // The watcher has cancelled Up/A; wait without further movement.
        stream.log("[Hearthome Fantina] Opening fade detected; waiting for dialogue.");
        segment03_finish_transition(stream, context, "Hearthome Fantina opening");
        ShortDialogWatcher opening_dialogue;
        if (wait_until(stream, context, 15s, {{opening_dialogue}}) != 0){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Hearthome Fantina: Opening fade was not followed by dialogue.",
                stream);
        }
    }else if (fantina_started != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Contest Hall: Fantina dialogue did not start.",
            stream);
    }
    clear_dialog(stream, context, BUTTON_B);
    pbf_wait(context, 2000ms);
    clear_dialog(stream, context, BUTTON_B);
    // Keep Hall entry separate from Fantina's outdoor conversation.
    segment03_cycling_move(stream, context, stats, 0, 1, 1179ms, 4268ms, "Hearthome Contest Hall entrance", true);
    pbf_press_button(context, BUTTON_A, 181ms, 219ms);
    context.wait_for_all_requests();
    ShortDialogWatcher contest_dialogue;
    if (wait_until(stream, context, 15s, {{contest_dialogue}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Contest Hall: Interior dialogue did not start.",
            stream);
    }
    // Use Buneary's resumed-text and scene-clear checks, with B to clear.
    bool contest_cleared = false;
    for (size_t pass = 0; pass < 6 && !contest_cleared; pass++){
        mash_until_dialogue_ends(stream, context, BUTTON_B, 60s);
        context.wait_for_all_requests();
        ShortDialogWatcher resumed_dialogue;
        if (wait_until(stream, context, 2000ms, {{resumed_dialogue}}) == 0){
            continue;
        }
        pbf_wait(context, 6000ms);
        context.wait_for_all_requests();
        const auto frame = stream.video().snapshot();
        OverworldDetector overworld;
        ShortDialogDetector dialogue;
        contest_cleared = frame && overworld.detect(*frame.frame) && !dialogue.detect(*frame.frame);
    }
    if (!contest_cleared){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Contest Hall: Interior scene did not clear.",
            stream);
    }
    // Own the exit before handing off the checkpoint. The transition watcher
    // cancels southward movement at the fade, leaving the fixed outdoor spawn.
    if (!segment03_cycling_move(stream, context, stats, 0, -1, 903ms, 1455ms, "Hearthome Contest Hall exit", true)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Contest Hall: Exit transition not confirmed.",
            stream);
    }
    OverworldWatcher hall_outside;
    if (wait_until(stream, context, 10s, {{hall_outside}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Contest Hall: Overworld not ready after exit.",
            stream);
    }
    // The approach landmark view is not the post-exit doorway view.
    // The watched south exit plus restored overworld completes this boundary.
    stream.log("[Hearthome Contest Hall] Checkpoint complete: outside doorway, facing south.");
    // End 03.04 / start 03.05: just outside the Hall, south-facing, Repel off.
}

void heartholme_to_lost_tower(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, Language language){
    OverworldWatcher hall_start;
    if (wait_until(stream, context, 10s, {{hall_start}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome Contest Hall checkpoint start: Overworld not ready.",
            stream);
    }
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1294ms, 132ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 3255ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 2666ms, 62ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2068ms, 456ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1065ms, 0ms);
    fight_trainer_using_moves(stream, context, stats, "Barry in Hearthome", true);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1088ms, 95ms);
    context.wait_for_all_requests();
    // Cancel at each gate fade, then allow a defined settling wait.
    BlackScreenWatcher east_gate_entry;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {1, 0}, 3446ms, 983ms);
            },
        {{east_gate_entry}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome east gate: Entrance transition did not start.",
            stream);
    }
    BlackScreenWatcher east_gate_entry_finished(COLOR_RED,
        {.1, .1, .8, .8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    if (wait_until(stream, context, 30s, {{east_gate_entry_finished}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome east gate: Entrance transition did not finish.",
            stream);
    }
    pbf_wait(context, 983ms);
    context.wait_for_all_requests();

    BlackScreenWatcher east_gate_exit;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {1, 0}, 2095ms, 1660ms);
            },
        {{east_gate_exit}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome east gate: Exit transition did not start.",
            stream);
    }
    BlackScreenWatcher east_gate_exit_finished(COLOR_RED,
        {.1, .1, .8, .8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    if (wait_until(stream, context, 30s, {{east_gate_exit_finished}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Hearthome east gate: Exit transition did not finish.",
            stream);
    }
    pbf_wait(context, 1660ms);
    context.wait_for_all_requests();
    stream.log("[Hearthome east gate] Entry and exit transitions finished.", COLOR_GREEN);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1128ms, 7266ms);
    // Use the Bag helper to handle menu navigation.
    if (!activate_repel(stream, context, language, 8, 0)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 209: Repel activation failed.",
            stream);
    }
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1412ms, 77ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1878ms, 100ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1246ms, 522ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 3110ms, 322ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1245ms, 89ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 3634ms, 966ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2378ms, 310ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 612ms, 1001ms);
    context.wait_for_all_requests();
    // Road alignment uses the sign frame and rock pair.
    align_to_overworld_landmarks(stream,
        context,
        match_route209_repel_point,
        "Route 209 Repel point before expiry",
        false,
        true);
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
    align_to_overworld_landmarks(stream,
        context,
        match_route209_repel_point,
        "Route 209 Repel point after renewal",
        false,
        true);

    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1345ms, 309ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 789ms, 456ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1100ms, 122ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1040ms, 93ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1157ms, 488ms);
    // Repel was renewed at the marked road pose; no delayed prompt inputs.
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 835ms, 549ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1451ms, 901ms);
    context.wait_for_all_requests();
    // Normalize the checkpoint boundary with a complete enter/exit cycle.
    // Cancel movement at each fade so it cannot continue onto the next map.
    BlackScreenWatcher tower_entry;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, 1}, 1311ms, 3000ms);
            },
        {{tower_entry}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower handoff: Entry transition did not start.",
            stream);
    }
    BlackScreenWatcher tower_entry_finished(COLOR_RED,
        {.1, .1, .8, .8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    if (wait_until(stream, context, 30s, {{tower_entry_finished}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower handoff: Entry transition did not finish.",
            stream);
    }
    OverworldWatcher tower_inside;
    if (wait_until(stream, context, 10s, {{tower_inside}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower handoff: Indoor overworld not ready.",
            stream);
    }

    BlackScreenWatcher tower_exit;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0, -1}, 1311ms, 3000ms);
            },
        {{tower_exit}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower handoff: Exit transition did not start.",
            stream);
    }
    BlackScreenWatcher tower_exit_finished(COLOR_RED,
        {.1, .1, .8, .8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    if (wait_until(stream, context, 30s, {{tower_exit_finished}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower handoff: Exit transition did not finish.",
            stream);
    }
    OverworldWatcher tower_outside;
    if (wait_until(stream, context, 10s, {{tower_outside}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower handoff: Outdoor overworld not ready.",
            stream);
    }
    context.wait_for_all_requests();
    stream.log("[Lost Tower handoff] Outside doorway, facing south; checkpoint complete.", COLOR_GREEN);
}
void lost_tower(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, Language language){
    // Checkpoint 03.06 starts outside the doorway, facing south, with Repel active.
    // Turn north, then enter using the same single-step timing as Segment 02.
    context.wait_for_all_requests();
    BlackScreenWatcher entering_tower;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                for (int i = 0; i < 2; i++){
                    pbf_press_dpad(ctx, DPAD_UP, 80ms, 300ms);
                }
                pbf_wait(ctx, 2389ms);
            },
        {{entering_tower}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower entry: Transition did not start.",
            stream);
    }
    BlackScreenWatcher tower_entry_finished(COLOR_RED,
        {.1, .1, .8, .8},
        100,
        10,
        BlackScreenWatcher::FinderType::GONE,
        500ms);
    if (wait_until(stream, context, 30s, {{tower_entry_finished}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower entry: Transition did not finish.",
            stream);
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    OverworldWatcher tower_entry_ready;
    if (wait_until(stream, context, 10s, {{tower_entry_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower entry: Overworld not ready.",
            stream);
    }

    // Use counted D-pad steps as in the Eterna Gym. Counts include one facing press whenever
    // direction changes. Keep the same direction across a dialogue stop without another facing
    // press.
    struct Move {
        DpadPosition dir;
        int count;
    };
    const Move to_floor2[] = {
        {DPAD_UP, 4},
        {DPAD_LEFT, 4},
        {DPAD_UP, 6},
        {DPAD_LEFT, 2},
        {DPAD_UP, 3},
        {DPAD_RIGHT, 2},
    };
    BlackScreenWatcher stairs_to_floor2;
    if (run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
                for (const auto& move : to_floor2){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(ctx, move.dir, 80ms, 300ms);
                    }
                }
                // Keep watching if the last step starts a delayed stair fade.
                pbf_wait(ctx, 3000ms);
            },
        {{stairs_to_floor2}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 2: Stair transition did not start.",
            stream);
    }
    segment03_finish_transition(stream, context, "Lost Tower floor 2");
    OverworldWatcher floor2_ready;
    if (wait_until(stream, context, 10s, {{floor2_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 2: Overworld not ready after stairs.",
            stream);
    }

    const Move to_floor3[] = {
        {DPAD_DOWN, 12},
        {DPAD_LEFT, 6},
        {DPAD_UP, 2},
        // Includes the extra left step to clear the gravestone column.
        {DPAD_LEFT, 8},
        {DPAD_UP, 8},
        {DPAD_RIGHT, 2},
        {DPAD_UP, 4},
        {DPAD_RIGHT, 4},
    };
    BlackScreenWatcher stairs_to_floor3;
    if (run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
                for (const auto& move : to_floor3){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(ctx, move.dir, 80ms, 300ms);
                    }
                }
                // Keep watching if the last step starts a delayed stair fade.
                pbf_wait(ctx, 3000ms);
            },
        {{stairs_to_floor3}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 3: Stair transition did not start.",
            stream);
    }
    segment03_finish_transition(stream, context, "Lost Tower floor 3");
    OverworldWatcher floor3_ready;
    if (wait_until(stream, context, 10s, {{floor3_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 3: Overworld not ready after stairs.",
            stream);
    }

    const Move to_repel[] = {
        {DPAD_DOWN, 6},
    };
    bool repel_renewed = false;
    ShortDialogDetector pending_repel_dialogue(COLOR_YELLOW);
    for (const auto& move : to_repel){
        for (int i = 0; i < move.count; i++){
            // Never queue another step while an expiry prompt is already visible.
            context.wait_for_all_requests();
            const auto frame = stream.video().snapshot();
            if (!frame){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Lost Tower Repel: No frame before the next down step.",
                    stream);
            }
            if (pending_repel_dialogue.detect(*frame.frame)){
                stream.log("[Lost Tower Repel] Pending renewal before down press " + std::to_string(i + 1) +
                    "; clearing before movement.");
                clear_dialog(stream, context, BUTTON_A);
                repel_renewed = true;
            }

            // Watch only one press at a time. Its 80ms hold finishes before the
            // fresh dialogue watcher's 250ms confirmation can cancel the rest.
            // The triggering step counts; all subsequent steps remain unqueued.
            ShortDialogWatcher step_repel_dialogue(COLOR_YELLOW);
            const int expiry = run_until<ProControllerContext>(stream,
                context,
                [&](ProControllerContext& ctx){
                    pbf_press_dpad(ctx, move.dir, 80ms, 300ms);
                    ctx.wait_for_all_requests();
                    pbf_wait(ctx, 500ms);
                },
                {{step_repel_dialogue}});
            if (expiry == 0){
                stream.log("[Lost Tower Repel] Renewal after down press " + std::to_string(i + 1) + "/" +
                    std::to_string(move.count) + "; clearing before the remaining steps.");
                clear_dialog(stream, context, BUTTON_A);
                repel_renewed = true;
            }
            context.wait_for_all_requests();
        }
    }
    // Preserve the bounded final observation if expiry arrives after the last step.
    if (!repel_renewed){
        ShortDialogWatcher late_repel_dialogue(COLOR_YELLOW);
        if (wait_until(stream, context, 3s, {{late_repel_dialogue}}) == 0){
            stream.log("[Lost Tower Repel] Late renewal dialogue detected; accepting renewal.");
            clear_dialog(stream, context, BUTTON_A);
        }else{
            stream.log("[Lost Tower Repel] No renewal dialogue at the recorded stop; continuing.");
        }
    }

    const Move to_biker[] = {
        {DPAD_DOWN, 1},
        {DPAD_RIGHT, 2},
        {DPAD_DOWN, 3},
        {DPAD_LEFT, 2},
        {DPAD_DOWN, 6},
        // Clear the gravestone column before beginning the watched up step.
        {DPAD_LEFT, 5},
        {DPAD_UP, 2},
    };
    for (const auto& move : to_biker){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }
    context.wait_for_all_requests();
    // The final up movement is one facing press and one movement press.
    // Stop at a trainer prompt before any later path segment is queued.
    constexpr int BIKER_UP_PRESSES = 2;
    int biker_up_completed = 0;
    // A trainer's walk-up can lock movement before the dialogue box appears.
    ShortDialogWatcher corner_biker_dialogue(COLOR_YELLOW);
    bool biker_challenged = wait_until(stream, context, 5s, {{corner_biker_dialogue}}) == 0;
    ShortDialogDetector pending_biker_dialogue(COLOR_YELLOW);
    while (!biker_challenged && biker_up_completed < BIKER_UP_PRESSES){
        context.wait_for_all_requests();
        const auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Lost Tower biker: No frame before the next up press.",
                stream);
        }
        if (pending_biker_dialogue.detect(*frame.frame)){
            biker_challenged = true;
            break;
        }

        // A fresh 250ms dialogue watch cannot cancel the 80ms hold itself.
        // Count that press, then preserve any subsequent unqueued presses.
        ShortDialogWatcher approaching_biker(COLOR_YELLOW);
        const int challenge = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                pbf_press_dpad(ctx, DPAD_UP, 80ms, 300ms);
                ctx.wait_for_all_requests();
                // Observe the walk-up before another up press can be queued.
                pbf_wait(ctx, 5000ms);
            },
            {{approaching_biker}});
        biker_up_completed++;
        if (challenge == 0){
            biker_challenged = true;
            break;
        }
        context.wait_for_all_requests();
    }
    if (biker_challenged){
        stream.log("[Lost Tower biker] Dialogue detected after " + std::to_string(biker_up_completed) +
            " up presses; handling the trainer battle.");
        fight_trainer_using_moves(stream, context, stats, "Lost Tower biker", false);
        clear_dialog(stream, context, BUTTON_B);

        // The trainer conversation turns the player toward the biker.
        // Restore north if its facing press was already used before the battle.
        if (biker_up_completed > 0){
            pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
            context.wait_for_all_requests();
        }
        for (; biker_up_completed < BIKER_UP_PRESSES; biker_up_completed++){
            pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
        }
        context.wait_for_all_requests();
        stream.log("[Lost Tower biker] North-facing approach complete; resuming the recorded path.");
    }else{
        stream.log("[Lost Tower biker] No dialogue; continuing the recorded path.");
    }

    const Move to_floor4[] = {
        {DPAD_UP, 1},
        {DPAD_LEFT, 2},
        {DPAD_UP, 4},
        {DPAD_LEFT, 6},
        {DPAD_UP, 4},
        {DPAD_RIGHT, 3},
        {DPAD_UP, 4},
        {DPAD_RIGHT, 3},
    };
    BlackScreenWatcher stairs_to_floor4;
    if (run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
                for (const auto& move : to_floor4){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(ctx, move.dir, 80ms, 300ms);
                    }
                }
                // Keep watching if the last step starts a delayed stair fade.
                pbf_wait(ctx, 3000ms);
            },
        {{stairs_to_floor4}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 4: Stair transition did not start.",
            stream);
    }
    segment03_finish_transition(stream, context, "Lost Tower floor 4");
    OverworldWatcher floor4_ready;
    if (wait_until(stream, context, 10s, {{floor4_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 4: Overworld not ready after stairs.",
            stream);
    }

    const Move to_floor5[] = {
        {DPAD_DOWN, 6},
        {DPAD_LEFT, 8},
        {DPAD_UP, 3},
        {DPAD_LEFT, 2},
        {DPAD_UP, 4},
        {DPAD_RIGHT, 3},
    };
    BlackScreenWatcher stairs_to_floor5;
    if (run_until<ProControllerContext>(stream,
        context,
        [&](ProControllerContext& ctx){
                for (const auto& move : to_floor5){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(ctx, move.dir, 80ms, 300ms);
                    }
                }
                // Keep watching if the last step starts a delayed stair fade.
                pbf_wait(ctx, 3000ms);
            },
        {{stairs_to_floor5}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 5: Stair transition did not start.",
            stream);
    }
    segment03_finish_transition(stream, context, "Lost Tower floor 5");
    OverworldWatcher floor5_ready;
    if (wait_until(stream, context, 10s, {{floor5_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower floor 5: Overworld not ready after stairs.",
            stream);
    }

    const Move to_rewards[] = {
        {DPAD_DOWN, 4},
        {DPAD_LEFT, 5},
        {DPAD_DOWN, 3},
    };
    for (const auto& move : to_rewards){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
        }
    }
    context.wait_for_all_requests();
    // Use the Segment 01 conversation pattern: start once, then clear with B.
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 80ms, 100ms);
    context.wait_for_all_requests();
    wait_for_dialog(stream, context, "Lost Tower reward: First box", 30s);
    clear_dialog(stream, context, BUTTON_B, 4000ms);
    stream.log("[Lost Tower reward] Dialogue absent for four seconds; opening the Bag.");
    if (!use_escape_rope(stream, context, language, 8)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower: Escape Rope exit was not confirmed.",
            stream);
    }
    // The exterior is visible before the Rope arrival camera finishes settling.
    // Keep the following south/left movement out of that arrival animation.
    pbf_wait(context, 5000ms);
    context.wait_for_all_requests();
    OverworldWatcher rope_exit_ready;
    if (wait_until(stream, context, 10s, {{rope_exit_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Lost Tower: Overworld not ready after Escape Rope settling.",
            stream);
    }
    context.wait_for_all_requests();
    stream.log("[Lost Tower] Reward dialogue cleared; Escape Rope exit settled.", COLOR_GREEN);
}

void lost_tower_to_veilstone_gym(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats, Language language){
    // Tower-to-Gym path; shared helpers handle prompts and battles.
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 336ms, 590ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 327ms, 590ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 745ms, 379ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 77ms, 17ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1167ms, 1882ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 512ms, 171ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 500ms, 1767ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1701ms, 0ms);
    pbf_press_button(context, BUTTON_PLUS, 181ms, 219ms);
    // Stay north/south on the open road beside Lost Tower.
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, true, true);
    pbf_wait(context, 1767ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 5000ms, 1767ms);
    pbf_press_button(context, BUTTON_PLUS, 181ms, 219ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 3400ms, 133ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1379ms, 365ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 3179ms, 76ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 3172ms, 28ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 4210ms, 421ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2327ms, 71ms);
    // End coverage after the Route 210 grass, then realign east against the trees.
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, false, true);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2327ms, 71ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 1028ms, 120ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2480ms, 773ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 182ms, 567ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2514ms, 86ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 180ms, 308ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 137ms, 2277ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 500ms, 510ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1300ms, 335ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 756ms, 335ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2176ms, 24ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 826ms, 561ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 957ms, 621ms);
    context.wait_for_all_requests();
    ShortDialogWatcher calvin_dialogue;
    StartBattleDetector calvin_start(stream.overlay());
    BattleMenuWatcher calvin_menu(BattleType::TRAINER);
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {1.000000, 0.000000}, 794ms, 15000ms);
            },
        {{calvin_dialogue}, {calvin_start}, {calvin_menu}}) < 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 215 Calvin: Trainer encounter did not start.",
            stream);
    }
    fight_trainer_using_moves(stream, context, stats, "Route 215 Ruin Maniac Calvin", false);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1582ms, 406ms);
    // The item occupies lane 2. Search only lanes 4 and 3 (numbered from the top).
    // Every D-pad press is observed separately: facing changes must not become
    // an unverified extra step north. Fixed scenery measures eastward progress.
    ShortDialogDetector jogger_dialogue;
    BattleMenuDetector jogger_battle(BattleType::TRAINER, COLOR_GREEN);
    BlackScreenDetector jogger_transition;
    constexpr double lane_pitch = 70.0 / 1080;
    constexpr double lane_tolerance = 1.0 / 54;
    size_t missing_pose = 0, stale_frames = 0, stalled = 0, lane_changes = 0, lane_presses = 0, arrival_frames = 0,
        jogger_battles = 0;
    int changing_to = 0;
    bool east_pending = false, last_coarse = false, corner_ready = false;
    double last_x = 0;
    Milliseconds bypass_movement = 0ms;
    WallClock fresh_after = current_time(), last_frame_time = WallClock::min();
    auto bypass_started = std::chrono::steady_clock::now();
    for (size_t observation = 0; observation < 120; observation++){
        context.throw_if_cancelled();
        context.wait_for_all_requests();
        if (std::chrono::steady_clock::now() - bypass_started > 90s){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 215 jogger: Lane search timed out.",
                stream);
        }
        const auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 215 jogger: No video frame.",
                stream);
        }
        if (frame.timestamp <= fresh_after || frame.timestamp <= last_frame_time){
            if (++stale_frames >= 8){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 215 jogger: Video is not advancing after movement.",
                    stream);
            }
            pbf_wait(context, 500ms);
            continue;
        }
        last_frame_time = frame.timestamp;
        stale_frames = 0;
        if (jogger_dialogue.detect(*frame.frame) || jogger_battle.detect(*frame.frame)){
            if (++jogger_battles > 1){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 215 jogger: Repeated unexpected encounter.",
                    stream);
            }
            // The jogger can interrupt with a trainer introduction. Complete it with the
            // move-list battle handler, then measure the lane again; the trainer may have
            // approached the player.
            fight_trainer_using_moves(stream, context, stats, "Route 215 jogger", false);
            context.wait_for_all_requests();
            fresh_after = current_time();
            last_frame_time = WallClock::min();
            missing_pose = 0;
            stale_frames = 0;
            stalled = 0;
            arrival_frames = 0;
            changing_to = 0;
            lane_presses = 0;
            east_pending = false;
            bypass_started = std::chrono::steady_clock::now();
            continue;
        }
        if (jogger_transition.detect(*frame.frame)){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 215 jogger: Unexpected transition.",
                stream);
        }
        const auto pose = match_route215_jogger_corner(*frame.frame);
        for (const auto& anchor : pose.landmarks){
            stream.log("[Route 215 jogger] " + anchor.name + " score=" + std::to_string(anchor.score) +
                ", margin=" + std::to_string(anchor.margin) + ", accepted=" + std::to_string(anchor.accepted) +
                ", offset=(" + std::to_string(anchor.dx) + "," + std::to_string(anchor.dy) + ")");
        }
        const int lane = pose.located && std::abs(pose.dy) <= lane_tolerance   ? 4
            : pose.located && std::abs(pose.dy - lane_pitch) <= lane_tolerance ? 3
            : 0;
        stream.log("[Route 215 jogger] lane=" + std::to_string(lane) + ", coarse=" + std::to_string(pose.coarse) +
            ", offset=(" + std::to_string(pose.dx) + "," + std::to_string(pose.dy) + ")");
        if (!lane){
            // Wait out rain/clouds or a settling step; never search vertically
            // from a missing or ambiguous pose, including after a blocked step.
            arrival_frames = 0;
            if (++missing_pose >= 8){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 215 jogger: Cannot verify lane 3 or 4.",
                    stream);
            }
            pbf_wait(context, 500ms);
            continue;
        }
        missing_pose = 0;
        if (east_pending){
            stalled = pose.coarse == last_coarse && last_x - pose.dx < 1.0 / 256 ? stalled + 1 : 0;
            east_pending = false;
        }
        if (changing_to && lane == changing_to){
            changing_to = 0;
            lane_presses = 0;
            stalled = 0;
        }
        const bool near_corner = !pose.coarse && std::abs(pose.dx) <= 1.0 / 128;
        if (near_corner && lane == 4 && !changing_to){
            if (++arrival_frames < 3){
                pbf_wait(context, 250ms);
                continue;
            }
            // Settle east against the fence in lane 4 before going south.
            pbf_move_left_joystick(context, {1.0, 0.0}, 1000ms, 0ms);
            context.wait_for_all_requests();
            corner_ready = true;
            break;
        }
        arrival_frames = 0;
        if (!changing_to && ((near_corner && lane == 3) || stalled >= 2)){
            if (++lane_changes > 6){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 215 jogger: Both safe lanes remain blocked.",
                    stream);
            }
            changing_to = lane == 4 ? 3 : 4;
            lane_presses = 0;
            stalled = 0;
            pbf_wait(context, 750ms);
            continue;
        }
        const auto movement_limit = 8000ms;
        if (changing_to){
            if (lane_presses >= 2){
                // A blocked lane change leaves us in the observed lane.
                // Wait for the jogger, then try east again without assuming a step.
                changing_to = 0;
                lane_presses = 0;
                pbf_wait(context, 750ms);
                continue;
            }
            if (bypass_movement + 80ms > movement_limit){
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 215 jogger: Movement budget exhausted.",
                    stream);
            }
            pbf_press_dpad(context, changing_to == 3 ? DPAD_UP : DPAD_DOWN, 80ms, 300ms);
            context.wait_for_all_requests();
            fresh_after = current_time();
            bypass_movement += 80ms;
            ++lane_presses;
            continue;
        }
        if (!pose.coarse && pose.dx < -1.0 / 32){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 215 jogger: Passed the expected east corner.",
                stream);
        }
        const auto duration = !pose.coarse && std::abs(pose.dx) <= 1.0 / 24 ? 40ms : 160ms;
        if (bypass_movement + duration > movement_limit){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 215 jogger: Movement budget exhausted.",
                stream);
        }
        last_x = pose.dx;
        last_coarse = pose.coarse;
        // Only a small westward correction is allowed at the verified corner.
        pbf_move_left_joystick(context, {!pose.coarse && pose.dx < 0 ? -1.0 : 1.0, 0.0}, duration, 400ms);
        context.wait_for_all_requests();
        fresh_after = current_time();
        bypass_movement += duration;
        east_pending = true;
    }
    if (!corner_ready){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 215 jogger: East corner not confirmed.",
            stream);
    }
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1682ms, 501ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1843ms, 55ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 2431ms, 723ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 2075ms, 243ms);
    pbf_move_left_joystick(context, {0.000000, 1.000000}, 500ms, 75ms);
    if (!activate_repel(stream, context, language, 8)){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 215 Derek: Could not activate Repel before the grass crossing.",
            stream);
    }
    pbf_wait(context, 1767ms);
    // Align against the eastern wall before Derek's south approach.
    pbf_move_left_joystick(context, {1.0, 0.0}, 3000ms, 43ms);
    context.wait_for_all_requests();
    ShortDialogWatcher derek_dialogue;
    StartBattleDetector derek_start(stream.overlay());
    BattleMenuWatcher derek_menu(BattleType::TRAINER);
    const int derek_encounter = run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
            // Stop before the grass; Derek can turn away and miss the player.
            pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 850ms, 15000ms);
        },
        {{derek_dialogue}, {derek_start}, {derek_menu}});
    if (derek_encounter >= 0){
        fight_trainer_using_moves(stream, context, stats, "Route 215 Black Belt Derek", false);
    }else{
        context.wait_for_all_requests();
        const auto frame = stream.video().snapshot();
        OverworldDetector overworld;
        BlackScreenDetector black;
        WhiteScreenDetector white;
        if (!frame || black.detect(*frame.frame) || white.detect(*frame.frame) ||
            !overworld.detect(*frame.frame)){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Route 215 Derek: No encounter and overworld not confirmed.",
                stream);
        }
        stream.log("[Route 215 Black Belt Derek] No encounter; continuing past optional trainer.", COLOR_GREEN);
    }
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1929ms, 5ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2195ms, 88ms);
    // Renew below the stairs before taking the grass-covered climb.
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
    stream.log("[Route 215 Derek stairs] East wall contact, northeast climb, then north to the trees.");
    pbf_move_left_joystick(context, {1.0, 0.0}, 1500ms, 300ms);
    pbf_move_left_joystick(context, {1.0, 0.75}, 5000ms, 300ms);
    pbf_move_left_joystick(context, {0.0, 1.0}, 2500ms, 300ms);
    // Renew east/west along the tree line, then restore both axes at its corner.
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
    stream.log("[Route 215 Derek stairs] Northwest tree corner, then four down presses.");
    pbf_move_left_joystick(context, {-1.0, 1.0}, 2500ms, 300ms);
    DpadState derek_dpad;
    repeat_dpad(context, derek_dpad, DPAD_DOWN, 80ms, 300ms, 4, false);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 1855ms, 171ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1385ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2478ms, 355ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 580ms, 392ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 978ms, 201ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1159ms, 122ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 535ms, 234ms);
    // West/east follows the clear strip south of the grass before the Ace Trainers.
    force_repel_expiry(stream, context, RepelRenewalChoice::YES, false, true);
    pbf_wait(context, 1767ms);
    // Resume from the road/fence pose after renewal. A fixed leftover west input compounds the
    // variable expiry offset.
    align_to_overworld_landmarks(stream,
        context,
        match_route215_ace_renewal,
        "Route 215 before Ace Trainers after renewal",
        false,
        true);
    context.wait_for_all_requests();
    ShortDialogWatcher dennis_dialogue;
    StartBattleDetector dennis_start(stream.overlay());
    BattleMenuWatcher dennis_menu(BattleType::TRAINER);
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0.000000, -1.000000}, 1323ms, 106ms);
                pbf_move_left_joystick(ctx, {1.000000, 0.000000}, 834ms, 206ms);
                pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 221ms, 516ms);
                pbf_move_left_joystick(ctx, {1.000000, 0.000000}, 667ms, 15000ms);
            },
        {{dennis_dialogue}, {dennis_start}, {dennis_menu}}) < 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Route 215 Dennis: Trainer encounter did not start.",
            stream);
    }
    // Dennis and Maya are separate consecutive battles.
    fight_trainer_using_moves(stream, context, stats, "Route 215 Ace Trainer Dennis", false);
    fight_trainer_using_moves(stream, context, stats, "Route 215 Ace Trainer Maya", false);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 10884ms, 48ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 2314ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -1.000000}, 26ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -0.000000}, 2251ms, 0ms);
    pbf_move_left_joystick(context, {1.000000, -1.000000}, 27ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 1382ms, 29ms);
    pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1035ms, 0ms);
    pbf_move_left_joystick(context, {-1.000000, -1.000000}, 75ms, 0ms);
    pbf_move_left_joystick(context, {0.000000, -1.000000}, 2190ms, 0ms);
    // Veilstone needs no Repel. Clear the remaining expiry without renewal
    // on the paved north/south strip before normalizing the city approach.
    force_repel_expiry(stream, context, RepelRenewalChoice::NO, true);
    align_to_overworld_landmarks(stream,
        context,
        match_veilstone_city_passage,
        "Veilstone passage after expiry",
        false,
        true,
        true);
    // Follow the street approach, then align with the western stair lane before climbing. Do not
    // use the NPC pair as a wall stop.
    pbf_move_left_joystick(context, {0.0, -1.0}, 1342ms, 0ms);
    pbf_move_left_joystick(context, {-1.0, 0.0}, 1404ms, 0ms);
    pbf_move_left_joystick(context, {0.0, -1.0}, 368ms, 0ms);
    pbf_move_left_joystick(context, {-1.0, -1.0}, 90ms, 0ms);
    pbf_move_left_joystick(context, {-1.0, 0.0}, 1458ms, 0ms);
    // Use the first 200ms of the north leg to expose the western stair base, then correct its
    // lane before continuing the ascent.
    pbf_move_left_joystick(context, {0.0, 1.0}, 200ms, 0ms);
    context.wait_for_all_requests();
    align_to_overworld_landmarks(stream,
        context,
        match_veilstone_dawn_stairs,
        "Veilstone western stair approach",
        false,
        true);
    stream.log("[Veilstone approach] Following the western stairs toward Dawn.");
    ShortDialogWatcher dawn_dialogue;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0.0, 1.0}, 3023ms, 0ms);
                // Dawn's turn, walk and camera cut precede the first box.
                // Keep watching through the cutscene; cancel as soon as it opens.
                pbf_press_button(ctx, BUTTON_A, 80ms, 15100ms);
            },
        {{dawn_dialogue}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Veilstone Dawn: Dialogue did not start from the west stair approach.",
            stream);
    }
    clear_dialog(stream, context, BUTTON_B, 4000ms);

    // Cancel movement at the first gym entry fade.
    context.wait_for_all_requests();
    BlackScreenWatcher gym_entry;
    if (run_until<ProControllerContext>(stream,
        context,
        [](ProControllerContext& ctx){
                pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 1954ms, 223ms);
                pbf_move_left_joystick(ctx, {1.000000, -0.000000}, 110ms, 190ms);
                pbf_move_left_joystick(ctx, {0.000000, 1.000000}, 910ms, 3000ms);
            },
        {{gym_entry}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Veilstone Gym: Entrance transition did not start.",
            stream);
    }
    segment03_finish_transition(stream, context, "Veilstone Gym entrance");
    // The first visit pans across the Gym before returning to the entrance.
    pbf_wait(context, 5000ms);
    context.wait_for_all_requests();
    OverworldWatcher gym_ready;
    if (wait_until(stream, context, 10s, {{gym_ready}}) != 0){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Veilstone Gym: Overworld not ready after the entrance scene.",
            stream);
    }
    context.wait_for_all_requests();
}

static void gym_Puzzle_03(VideoStream& stream, ProControllerContext& context, AutoStoryStats& stats){

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

    for (const auto& move : sequence1){
        for (int i = 0; i < move.count; i++){
            pbf_press_dpad(context, move.dir, 80ms, 300ms);
            if (move.delay.count() > 0){
                pbf_wait(context, move.delay);
            }
        }
    }

    //Button presses to initiate Trainer battle 01 (Black Belt Rafael).

    pbf_press_button(context, BUTTON_A, 150ms, 300ms);
    fight_trainer_using_moves(stream, context, stats, "Black Belt Rafael", false);

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

    pbf_press_button(context, BUTTON_A, 150ms, 300ms);
    fight_trainer_using_moves(stream, context, stats, "Black Belt Jeffrey", false);

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

    pbf_press_button(context, BUTTON_A, 150ms, 300ms);
    fight_trainer_using_moves(stream, context, stats, "Black Belt Darren", false);

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

    pbf_press_button(context, BUTTON_A, 150ms, 300ms);
    fight_trainer_using_moves(stream, context, stats, "Black Belt Colby", false);

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

    pbf_press_button(context, BUTTON_A, 150ms, 300ms);
    fight_trainer_using_moves(stream, context, stats, "Maylene", true);

    // Clear badge and reward dialogue before handing off inside the Gym.
    // Match Gardenia's quiet period so gaps between rewards do not end cleanup.
    stream.log("[Veilstone Gym puzzle] Shared battle cleanup returned; clearing remaining Maylene rewards with B.");
    clear_dialog(stream, context, BUTTON_B, 6000ms);
    stream.log("[Veilstone Gym puzzle] Maylene post-battle dialogue cleared; remaining here for Segment 04.");
}

std::string AutoStory_Segment_03::name() const {
    return "03: Second Badge to Third Badge";
}
std::string AutoStory_Segment_03::start_text() const {
    return AutoStory_Checkpoint_0301().start_text();
}
std::string AutoStory_Segment_03::end_text() const {
    return AutoStory_Checkpoint_0307().end_text();
}

void AutoStory_Segment_03::run_segment(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
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
    AutoStory_Checkpoint_0307().run_checkpoint(env, context, options, stats);
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);
}


std::string AutoStory_Checkpoint_0301::name() const {
    return "03.01 - Eterna Gym to Jupiter";
}
std::string AutoStory_Checkpoint_0301::start_text() const {
    return "Outside Eterna Gym after defeating Gardenia and acquiring the Forest Badge, facing south with reward dialogue finished.";
}
std::string AutoStory_Checkpoint_0301::end_text() const {
    return "Outside Eterna Galactic HQ after defeating Jupiter and completing the rescue dialogue, facing south against the uncut front bush.";
}

void AutoStory_Checkpoint_0301::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    Checkpoint_0301(env, context, options, stats);
}


void Checkpoint_0301(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            eterna_gym_to_jupiter(env.console, context, stats);
        },

        true);
}
std::string AutoStory_Checkpoint_0302::name() const {
    return "03.02 - Jupiter to Mount Coronet";
}
std::string AutoStory_Checkpoint_0302::start_text() const {
    return "Outside Eterna Galactic HQ after defeating Jupiter and completing the rescue dialogue, facing south against the uncut front bush.";
}
std::string AutoStory_Checkpoint_0302::end_text() const {
    return "Inside Mount Coronet at the west entrance after defeating Hiker Justin, facing east.";
}

void AutoStory_Checkpoint_0302::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    Checkpoint_0302(env, context, options, stats);
}


void Checkpoint_0302(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Set daylight once for this outdoor locator strip, including the
            // Center, Mart return, bicycle shop, and north Route 206 gate.
            context.wait_for_all_requests();
            env.console.log("[Eterna locator setup] Setting 14:00; preserving the current date.");
            DateTime locator_time;
            locator_time.hour = 14;
            locator_time.minute = 0;
            locator_time.second = 0;
            set_time_fast(env, context, locator_time, true);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
            jupiter_to_coronet(env.console, context, stats, options.language);
        },

        true);
}
std::string AutoStory_Checkpoint_0303::name() const {
    return "03.03 - Mount Coronet";
}
std::string AutoStory_Checkpoint_0303::start_text() const {
    return "Inside Mount Coronet at the west entrance after defeating Hiker Justin, facing east.";
}
std::string AutoStory_Checkpoint_0303::end_text() const {
    return "Inside Mount Coronet near the east exit, after the first Cyrus meeting.";
}

void AutoStory_Checkpoint_0303::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    Checkpoint_0303(env, context, options, stats);
}


void Checkpoint_0303(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            coronet1(env.console, context, options.language);
        },

        true);
}
std::string AutoStory_Checkpoint_0304::name() const {
    return "03.04 - Coronet to Hearthome";
}
std::string AutoStory_Checkpoint_0304::start_text() const {
    return "Inside Mount Coronet near the east exit, after the first Cyrus meeting.";
}
std::string AutoStory_Checkpoint_0304::end_text() const {
    return "Just outside the Hearthome Contest Hall doorway, facing south with Repel inactive, after the Keira/Johanna introduction.";
}

void AutoStory_Checkpoint_0304::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    Checkpoint_0304(env, context, options, stats);
}


void Checkpoint_0304(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            coronet_to_heartholme(env.console, context, stats, options.language);
        },

        true);
}
std::string AutoStory_Checkpoint_0305::name() const {
    return "03.05 - Hearthome to Lost Tower";
}
std::string AutoStory_Checkpoint_0305::start_text() const {
    return "Just outside the Hearthome Contest Hall doorway, facing south with Repel inactive, after the Keira/Johanna introduction.";
}
std::string AutoStory_Checkpoint_0305::end_text() const {
    return "Just outside the Lost Tower doorway on Route 209 after entering and exiting, facing south with Repel active.";
}

void AutoStory_Checkpoint_0305::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    Checkpoint_0305(env, context, options, stats);
}


void Checkpoint_0305(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Restore night lighting on every attempt, including after a reset.
            context.wait_for_all_requests();
            env.console.log("[Route 209 locator setup] Setting 22:00; preserving the current date.");
            DateTime locator_time;
            locator_time.hour = 22;
            locator_time.minute = 0;
            locator_time.second = 0;
            set_time_fast(env, context, locator_time, true);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
            heartholme_to_lost_tower(env.console, context, stats, options.language);
            // Stop here. Checkpoint 03.06 saves outside and owns the Tower visit.
        },

        true);
}

std::string AutoStory_Checkpoint_0306::name() const {
    return "03.06 - Lost Tower to Veilstone";
}
std::string AutoStory_Checkpoint_0306::start_text() const {
    return "Just outside the Lost Tower doorway on Route 209 after entering and exiting, facing south with Repel active.";
}
std::string AutoStory_Checkpoint_0306::end_text() const {
    return "Inside Veilstone Gym at the entrance, facing north.";
}

void AutoStory_Checkpoint_0306::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    Checkpoint_0306(env, context, options, stats);
}


void Checkpoint_0306(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            // Restore night before the route on every attempt, including resets.
            context.wait_for_all_requests();
            env.console.log("[Route 215 locator setup] Setting 22:00; preserving the current date.");
            DateTime locator_time;
            locator_time.hour = 22;
            locator_time.minute = 0;
            locator_time.second = 0;
            set_time_fast(env, context, locator_time, true);
            pbf_wait(context, 5000ms);
            context.wait_for_all_requests();
            // Save/retry begins outside the Tower, at the end of 03.05.
            lost_tower(env.console, context, stats, options.language);
            lost_tower_to_veilstone_gym(env.console, context, stats, options.language);
        },
        true);
}

std::string AutoStory_Checkpoint_0307::name() const {
    return "03.07 - Veilstone Gym (start inside at entrance)";
}
std::string AutoStory_Checkpoint_0307::start_text() const {
    return "Start inside Veilstone Gym at the entrance, facing north.";
}
std::string AutoStory_Checkpoint_0307::end_text() const {
    return "Inside Veilstone Gym in front of Maylene, facing north, after acquiring the third badge and clearing her reward dialogue.";
}

void AutoStory_Checkpoint_0307::run_checkpoint(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats) const {
    Checkpoint_0307(env, context, options, stats);
}


void Checkpoint_0307(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats){
    checkpoint_reattempt_loop(
        env,
        context,
        options,
        stats,
        [&](size_t /*attempt*/){
            //#. Veilstone Gym navigation and battle
            gym_Puzzle_03(env.console, context, stats);
        },
        true);
}

}
}
}
