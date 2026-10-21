/*  BDSP AutoStory - Segment 02
 *  From after acquiring the first badge until after acquiring the second badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include <algorithm>
#include <functional>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "../Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "../Utils/PokemonBDSP_OverworldMatchers.h"
#include "../Utils/PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "../Utils/PokemonBDSP_Autostory_Segment_01_tools.h"
#include "PokemonBDSP_AutoStory_Segment_02.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
    namespace NintendoSwitch{
        namespace PokemonBDSP{
            static void align_to_oreburgh_mart(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                pbf_wait(context, 700ms);
                context.wait_for_all_requests();
                size_t confirmations = 0, missing = 0;
                for (size_t attempt = 0; attempt < 40; attempt++) {
                    const VideoSnapshot frame = stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Mart alignment: No video frame.", stream);
                    const OverworldMatchResult result = match_oreburgh_mart(*frame.frame);
                    VideoOverlaySet overlays(stream.overlay());
                    for (const auto& landmark : result.landmarks) {
                        overlays.add(COLOR_CYAN, landmark.search);
                        if (landmark.accepted) overlays.add(COLOR_GREEN, landmark.found);
                        stream.log("[Mart alignment] score=" + std::to_string(landmark.score) +
                            ", screen-relative offset=(" + std::to_string(landmark.dx) + "," + std::to_string(landmark.dy) + ")");
                    }
                    if (!result.located) {
                        confirmations = 0;
                        if (++missing >= 8) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Mart alignment: Emblem match missing or ambiguous.", stream);
                        // A passing shadow must not cause an immediate checkpoint reset.
                        // Stay still until a trusted match returns; never steer by a rejected candidate.
                        pbf_wait(context, 250ms);
                        context.wait_for_all_requests();
                        continue;
                    }
                    missing = 0;
                    if (result.at_target) {
                        if (++confirmations >= 3) { stream.log("[Mart alignment] Door position confirmed."); return; }
                        pbf_wait(context, 500ms);
                    } else {
                        confirmations = 0;
                        const double dx = result.dx, dy = result.dy;
                        const bool horizontal = std::abs(dx) >= std::abs(dy)*9.0/16;
                        const double error = horizontal ? dx : dy*9.0/16;
                        const auto duration = std::abs(error) > 1.0/24 ? 80ms : std::abs(error) > 0.015625 ? 40ms : 16ms;
                        pbf_move_left_joystick(context,
                            {horizontal ? (dx > 0 ? 1.0 : -1.0) : 0.0,
                             horizontal ? 0.0 : (dy < 0 ? 1.0 : -1.0)}, duration, 0ms);
                        pbf_wait(context, 700ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Mart alignment: Could not reach the doorway reference position.", stream);
            }

            static void enter_oreburgh_gate_from_city(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                stream.log("[Oreburgh Gate entry] Moving up, then searching left/down for the entrance.");
                // Observe before the first move so entry during the upward
                // alignment or either search direction cannot be missed.
                BlackScreenWatcher black;
                const int result = run_until<ProControllerContext>(stream, context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx, {0.0, 1.0}, 800ms, 100ms);
                        for (size_t attempt = 0; attempt < 30; attempt++) {
                            pbf_move_left_joystick(ctx, {-1.0, 0.0}, 200ms, 80ms);
                            pbf_move_left_joystick(ctx, {0.0, -1.0}, 120ms, 80ms);
                            ctx.wait_for_all_requests();
                        }
                        // Keep watching if the last pulse starts a delayed fade.
                        pbf_wait(ctx, 1500ms);
                    }, {{black}});
                if (result != 0) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Oreburgh Gate entry: No black transition during left/down search.", stream);
                stream.log("[Oreburgh Gate entry] Black transition detected; movement stopped.");
                BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8},
                    100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
                if (wait_until(stream, context, 15s, {{visible}}) != 0)
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Oreburgh Gate entry: Transition did not finish.", stream);
                pbf_wait(context, 700ms);
                context.wait_for_all_requests();
                stream.log("[Oreburgh Gate entry] Cave visible; continuing the route.", COLOR_GREEN);
            }

            enum class RepelRenewalChoice { YES, NO };

            // Call at a safe walking point, away from trainer/NPC prompts.
            static void force_repel_expiry(VideoStream& stream, ProControllerContext& context,
                RepelRenewalChoice choice, bool north_south = false) {
                context.wait_for_all_requests();
                stream.log(std::string("[Repel expiry] Wiggling ") +
                    (north_south ? "north/south" : "west/east") + " until expiry; renewal response=" +
                    (choice == RepelRenewalChoice::YES ? "Yes." : "No."));
                const Button response = choice == RepelRenewalChoice::YES ? BUTTON_A : BUTTON_B;
                ShortDialogWatcher expired(COLOR_CYAN);
                // 108 fresh Max Repels, two calibration runs: mean 41.244682s,
                // sample SD 3.668059s. Round mean + SD up to the next millisecond.
                // Match the calibration's 500ms movement / 100ms rest pulses.
                const auto movement_deadline = std::chrono::steady_clock::now() + 44913ms;
                const int result = run_until<ProControllerContext>(stream, context,
                    [north_south, movement_deadline](ProControllerContext& ctx) {
                        while (std::chrono::steady_clock::now() + 1200ms <= movement_deadline) {
                            // Complete steps before reversing so the Repel counter advances.
                            if (north_south) {
                                pbf_move_left_joystick(ctx, {0.0, 1.0}, 500ms, 100ms);
                                pbf_move_left_joystick(ctx, {0.0, -1.0}, 500ms, 100ms);
                            } else {
                                pbf_move_left_joystick(ctx, {-1.0, 0.0}, 500ms, 100ms);
                                pbf_move_left_joystick(ctx, {1.0, 0.0}, 500ms, 100ms);
                            }
                            ctx.wait_for_all_requests();
                        }
                        // Finish the observation window without an unpaired move.
                        const auto remaining = std::chrono::duration_cast<Milliseconds>(
                            movement_deadline - std::chrono::steady_clock::now());
                        if (remaining > 0ms) pbf_wait(ctx, remaining);
                    }, {{expired}});
                if (result != 0) {
                    stream.log("[Repel expiry] Calibrated 44.913s limit reached without an expiry prompt; checking clear overworld before continuing.", COLOR_ORANGE);
                }
                // Renewal has a gap before the final used-Repel message. A single
                // overworld frame is insufficient: require three continuous clear seconds.
                context.wait_for_all_requests();
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BlackScreenDetector black;
                OverworldDetector overworld;
                VideoOverlaySet overlays(stream.overlay());
                dialogue.make_overlays(overlays);
                black.make_overlays(overlays);
                overworld.make_overlays(overlays);
                const auto deadline = std::chrono::steady_clock::now() + 30s;
                auto quiet_since = std::chrono::steady_clock::now();
                bool quiet = false;
                while (std::chrono::steady_clock::now() < deadline) {
                    const auto frame = stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Repel renewal: No video frame during cleanup.", stream);
                    const auto now = std::chrono::steady_clock::now();
                    if (dialogue.detect(*frame.frame)) {
                        quiet = false;
                        pbf_mash_button(context, response, 100ms);
                    } else if (black.detect(*frame.frame) || !overworld.detect(*frame.frame)) {
                        quiet = false;
                        pbf_wait(context, 100ms);
                    } else {
                        if (!quiet) { quiet = true; quiet_since = now; }
                        if (now - quiet_since >= 3s) {
                            context.wait_for_all_requests();
                            if (result == 0) {
                                stream.log(std::string("[Repel expiry] Renewal ") +
                                    (choice == RepelRenewalChoice::YES ? "accepted" : "declined") +
                                    "; three continuous seconds of overworld confirmed.");
                            } else {
                                stream.log("[Repel expiry] No expiry observed within the calibrated limit; three continuous seconds of clear overworld confirmed. Continuing the route.");
                            }
                            return;
                        }
                        // Do not send confirmation/cancel buttons while the overworld is clear.
                        pbf_wait(context, 100ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Repel renewal: Three clear seconds not confirmed after renewal.", stream);
            }

            static void wait_for_trainer_battle_menu(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BattleMenuDetector menu(BattleType::TRAINER, COLOR_GREEN);
                VideoOverlaySet overlays(stream.overlay());
                dialogue.make_overlays(overlays);
                menu.make_overlays(overlays);
                const auto deadline = std::chrono::steady_clock::now() + 120s;
                size_t confirmed = 0;
                bool previous_dialogue = false;
                while (std::chrono::steady_clock::now() < deadline) {
                    const auto frame = stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Trainer approach: No video frame.", stream);
                    const bool dialog = dialogue.detect(*frame.frame);
                    if (dialog && !previous_dialogue) stream.log("[Trainer approach] Clearing dialogue with B.");
                    previous_dialogue = dialog;
                    if (menu.detect(*frame.frame)) {
                        if (++confirmed >= 3) {
                            stream.log("[Trainer approach] Battle menu confirmed; starting combat.");
                            return;
                        }
                        pbf_wait(context, 100ms);
                    } else {
                        confirmed = 0;
                        pbf_mash_button(context, BUTTON_B, 100ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Trainer approach: Battle menu did not appear after dialogue cleanup.", stream);
            }

            static void exit_oreburgh_gate_to_route203(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                stream.log("[Oreburgh Gate exit] Moving west until the white transition starts.");
                WhiteScreenWatcher white;
                const int result = run_until<ProControllerContext>(stream, context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx, {-1.0, 0.0}, 6000ms, 0ms);
                        pbf_wait(ctx, 1000ms);
                    }, {{white}});
                if (result != 0) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Oreburgh Gate exit: White transition did not start.", stream);
                WhiteScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8},
                    500, 10, WhiteScreenWatcher::FinderType::GONE, 500ms);
                if (wait_until(stream, context, 15s, {{visible}}) != 0)
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Oreburgh Gate exit: White transition did not finish.", stream);
                pbf_wait(context, 700ms);
                context.wait_for_all_requests();
            }

            static void approach_galactic_dialogue(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BattleMenuDetector battle(BattleType::TRAINER, COLOR_GREEN);
                const auto frame = stream.video().snapshot();
                if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Galactic approach: No video frame.", stream);
                // Alignment may already have handed off after the story started.
                if (dialogue.detect(*frame.frame) || battle.detect(*frame.frame)) return;
                stream.log("[Galactic approach] Plaza position confirmed; sidestepping left around the sign, then moving up until story dialogue.");
                ShortDialogWatcher started(COLOR_YELLOW);
                BattleMenuWatcher menu(BattleType::TRAINER, COLOR_GREEN);
                const int result = run_until<ProControllerContext>(stream, context,
                    [](ProControllerContext& ctx) {
                        // The reference position faces the sign, which blocks a
                        // straight upward approach and consumes A as a sign read.
                        pbf_move_left_joystick(ctx, {-1.0, 0.0}, 200ms, 150ms);
                        ctx.wait_for_all_requests();
                        for (size_t pulse = 0; pulse < 20; pulse++) {
                            pbf_move_left_joystick(ctx, {0.0, 1.0}, 100ms, 100ms);
                            ctx.wait_for_all_requests();
                        }
                        // If an NPC blocks the final approach, initiate interaction.
                        pbf_press_button(ctx, BUTTON_A, 80ms, 1000ms);
                        pbf_wait(ctx, 1500ms);
                    }, {{started}, {menu}});
                if (result < 0) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Galactic approach: Story dialogue/battle did not start during the bounded upward approach.", stream);
                stream.log("[Galactic approach] Story dialogue/battle detected; movement stopped.");
            }

            // Dialogue indicates the trainer interaction started; combat still waits
            // for a confirmed battle menu before invoking the battle handler.
            static bool watch_route204_trainer_event(VideoStream& stream, ProControllerContext& context,
                const std::function<void(ProControllerContext&)>& action, const std::string& label) {
                context.wait_for_all_requests();
                ShortDialogWatcher dialogue(COLOR_YELLOW);
                BattleMenuWatcher menu(BattleType::TRAINER, COLOR_GREEN);
                const int result = run_until<ProControllerContext>(stream, context,
                    [&action](ProControllerContext& ctx) { action(ctx); }, {{dialogue}, {menu}});
                const bool detected = result >= 0;
                stream.log("[" + label + "] Trainer dialogue/battle detected=" + std::to_string(detected));
                return detected;
            }

            static bool approach_route204_female_trainer(VideoStream& stream, ProControllerContext& context) {
                return watch_route204_trainer_event(stream, context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx, {0.0, 1.0}, 8000ms, 0ms);
                        // Replaces the 2374ms settle plus the unbounded-looking
                        // generic dialogue wait with one explicit detection window.
                        pbf_wait(ctx, 2500ms);
                    }, "Route 204 female trainer approach");
            }

            static void recover_route204_female_trainer(VideoStream& stream, ProControllerContext& context) {
                stream.log("[Route 204 female trainer] Missed initial trigger; crossing her sight line down/up.");
                const bool recovered = watch_route204_trainer_event(stream, context,
                    [](ProControllerContext& ctx) {
                        for (size_t pair = 0; pair < 8; pair++) {
                            // We have passed her: go back down first, then up.
                            pbf_move_left_joystick(ctx, {0.0, -1.0}, 500ms, 150ms);
                            pbf_move_left_joystick(ctx, {0.0, 1.0}, 500ms, 150ms);
                            ctx.wait_for_all_requests();
                        }
                        pbf_wait(ctx, 2500ms);
                    }, "Route 204 female trainer recovery");
                if (!recovered) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 204 female trainer: No dialogue/battle after bounded down/up recovery.", stream);
            }

            static void recover_route204_youngster(VideoStream& stream, ProControllerContext& context) {
                // The recording ends above the youngster after passing left.
                // Cross his row down/up instead of resetting after one miss.
                const bool recovered = watch_route204_trainer_event(stream, context,
                    [](ProControllerContext& ctx) {
                        for (size_t pair = 0; pair < 6; pair++) {
                            pbf_move_left_joystick(ctx, {0, -1}, 500ms, 200ms);
                            // Body collision can leave us adjacent without a sightline trigger.
                            pbf_press_button(ctx, BUTTON_A, 100ms, 250ms);
                            pbf_move_left_joystick(ctx, {0, 1}, 500ms, 200ms);
                            pbf_press_button(ctx, BUTTON_A, 100ms, 250ms);
                            ctx.wait_for_all_requests();
                        }
                        pbf_wait(ctx, 2500ms);
                    }, "Route 204 youngster sightline recovery");
                if (!recovered) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 204 youngster: No trigger after bounded down/up recovery.", stream);
            }

            static void floaroma_house_transition(VideoStream& stream, ProControllerContext& context, double y) {
                context.wait_for_all_requests();
                BlackScreenWatcher black;
                const int result=run_until<ProControllerContext>(stream,context,
                    [y](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx,{0,y},2500ms,0ms);
                        pbf_wait(ctx,1000ms);
                    },{{black}});
                if (result!=0) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Floaroma house: Door transition not detected.",stream);
                context.wait_for_all_requests();
                BlackScreenDetector black_frame;
                const auto deadline=std::chrono::steady_clock::now()+15s;
                auto clear_since=std::chrono::steady_clock::now();
                while (std::chrono::steady_clock::now()<deadline) {
                    const auto frame=stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Floaroma house: No video frame after transition.",stream);
                    const auto now=std::chrono::steady_clock::now();
                    if (black_frame.detect(*frame.frame)) clear_since=now;
                    else if (now-clear_since>=500ms) {
                        pbf_wait(context,700ms);
                        context.wait_for_all_requests();
                        return;
                    }
                    pbf_wait(context,100ms);
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Floaroma house: Door transition did not finish.",stream);
            }

            static void establish_floaroma_house_exit(VideoStream& stream, ProControllerContext& context) {
                stream.log("[Floaroma checkpoint] Blind northward house entry, then immediate exit.");
                floaroma_house_transition(stream,context,1);
                floaroma_house_transition(stream,context,-1);
                stream.log("[Floaroma checkpoint] House entry/exit completed; ready for checkpoint save.",COLOR_GREEN);
            }

            static void recover_teighler_trigger(VideoStream& stream, ProControllerContext& context) {
                // The first pass ended east of the corner. Return left, then
                // cross her sightline along the same bottom ledge for five seconds.
                const bool recovered=watch_route204_trainer_event(stream,context,
                    [](ProControllerContext& ctx) {
                        // The old 500ms sweeps stopped beneath/west of Teighler.
                        // Cross the full bottom ledge in both directions, still
                        // limiting the total recovery motion to five seconds.
                        for (size_t pair=0; pair<3; pair++) {
                            const Milliseconds sweep = pair<2 ? 1000ms : 500ms;
                            pbf_move_left_joystick(ctx,{-1,0},sweep,0ms);
                            pbf_move_left_joystick(ctx,{1,0},sweep,0ms);
                            ctx.wait_for_all_requests();
                        }
                        pbf_wait(ctx,2500ms);
                    },"Teighler five-second ledge recovery");
                if (!recovered) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Teighler: No battle after five-second left/right ledge recovery.",stream);
            }

            // True means the next trainer triggered before the position was reached.
            // False means the sign position was confirmed. Both outcomes are explicit.
            // Route 204 arrival accepts a small standing area, rather than one
            // exact pose. Preserve strong observations through passing shadows
            // only while stationary; every movement invalidates this history.
            static bool align_to_route204_floaroma_turn(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BattleMenuDetector battle_menu(BattleType::TRAINER, COLOR_GREEN);
                size_t confirmed=0, missing=0, blocked_left=0;
                double previous_dx=1;
                bool previously_left=false;
                stream.log("[Route 204 north turn] Locate left to the sign; correct height after horizontal alignment.");
                for (size_t attempt=0; attempt<60; attempt++) {
                    const auto frame=stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Route 204 north turn: No video frame.", stream);
                    if (dialogue.detect(*frame.frame) || battle_menu.detect(*frame.frame)) return true;
                    const auto result=match_route204_floaroma_turn(*frame.frame);
                    if (result.at_target) {
                        if (++confirmed>=3) {
                            stream.log("[Route 204 north turn] Sign position confirmed; ready to continue west.", COLOR_GREEN);
                            return false;
                        }
                        pbf_wait(context,150ms);
                        context.wait_for_all_requests();
                        continue;
                    }
                    confirmed=0;
                    double x=-1, y=0;
                    Milliseconds duration=100ms;
                    if (result.located) {
                        missing=0;
                        stream.log("[Route 204 north turn] Sign offset=("+std::to_string(result.dx)+","+std::to_string(result.dy)+")");
                        const bool horizontal=std::abs(result.dx)>1.0/50;
                        if (previously_left && std::abs(result.dx-previous_dx)<1.0/384) blocked_left++;
                        else blocked_left=0;
                        // Primarily locate left. If a fence/NPC blocks progress,
                        // correct height briefly before resuming measured travel.
                        const bool correct_height=!horizontal ||
                            (blocked_left>=3 && std::abs(result.dy)>1.0/54);
                        x=correct_height ? 0.0 : (result.dx<0 ? -1.0 : 1.0);
                        y=correct_height ? (result.dy>0 ? -1.0 : 1.0) : 0.0;
                        const double error=correct_height ? std::abs(result.dy) : std::abs(result.dx);
                        duration=error<.08 ? 40ms : 100ms;
                        previous_dx=result.dx;
                        previously_left=x<0;
                        if (correct_height) blocked_left=0;
                    } else {
                        previously_left=false;
                        blocked_left=0;
                        if (++missing>12) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Route 204 north turn: Sign unavailable during bounded leftward acquisition.", stream);
                    }
                    if (watch_route204_trainer_event(stream,context,
                        [x,y,duration](ProControllerContext& ctx) {
                            pbf_move_left_joystick(ctx,{x,y},duration,0ms);
                            pbf_wait(ctx,250ms);
                        }, "Route 204 north sign alignment")) return true;
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 204 north turn: Sign position did not converge.", stream);
            }

            static void align_to_windworks_sign(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                size_t confirmed=0, missing=0;
                bool previously_located=false;
                stream.log("[Windworks approach] Locate beside the sign before the blind wall/NPC approach.");
                for (size_t attempt=0; attempt<80; attempt++) {
                    const auto frame=stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Windworks approach: No video frame.",stream);
                    const auto result=match_windworks_sign_approach(*frame.frame);
                    if (result.at_target) {
                        if (++confirmed>=3) return;
                        pbf_wait(context,150ms);
                    } else {
                        confirmed=0;
                        double x=0,y=1;
                        Milliseconds duration=100ms;
                        if (result.located) {
                            missing=0;
                            previously_located=true;
                            // Correct large height errors first, including coming
                            // south from the wall, then clear the grass horizontally.
                            const bool vertical=result.dy>.08 || std::abs(result.dx)<=1.0/50;
                            const double error=vertical?std::abs(result.dy):std::abs(result.dx);
                            x=vertical?0:(result.dx<0?-1:1);
                            y=vertical?(result.dy>0?-1:1):0;
                            duration=error<.08?40ms:100ms;
                            stream.log("[Windworks approach] Sign offset=("+std::to_string(result.dx)+","+std::to_string(result.dy)+")");
                        } else if (previously_located && ++missing<=12) {
                            pbf_wait(context,250ms);
                            context.wait_for_all_requests();
                            continue;
                        } else if (previously_located || ++missing>12) {
                            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                "Windworks approach: Sign unavailable during bounded northward acquisition.",stream);
                        }
                        pbf_move_left_joystick(context,{x,y},duration,0ms);
                        pbf_wait(context,250ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Windworks approach: Sign position did not converge.",stream);
            }

            struct Route204ArrivalTracker {
                using Clock = std::chrono::steady_clock;
                static constexpr double TOLERANCE_X = 1.0/32;
                static constexpr double TOLERANCE_Y = 1.0/36;
                struct Anchor {
                    bool seen = false;
                    double dx = 0, dy = 0;
                    Clock::time_point timestamp{};
                };
                Anchor anchors[2];
                bool holding = false;
                Clock::time_point hold_started{};
                void reset() {
                    anchors[0].seen = anchors[1].seen = false;
                    holding = false;
                }
                bool observe(const OverworldMatchResult& result, Clock::time_point now) {
                    if (result.landmarks.size() != 2) { reset(); return false; }
                    for (const auto& landmark : result.landmarks) {
                        if (landmark.accepted &&
                            (std::abs(landmark.dx)>TOLERANCE_X || std::abs(landmark.dy)>TOLERANCE_Y)) {
                            reset();
                            return false;
                        }
                    }
                    for (size_t i = 0; i < 2; i++) {
                        const auto& landmark = result.landmarks[i];
                        if (!landmark.accepted) continue;
                        anchors[i] = {true, landmark.dx, landmark.dy, now};
                        if (!holding) { holding = true; hold_started = now; }
                    }
                    for (auto& anchor : anchors) {
                        if (anchor.seen && now-anchor.timestamp>10s) anchor.seen = false;
                    }
                    return anchors[0].seen && anchors[1].seen &&
                        std::abs(anchors[0].dx-anchors[1].dx)<=1.0/64 &&
                        std::abs(anchors[0].dy-anchors[1].dy)<=1.0/54;
                }
                bool wait_for_shadow(Clock::time_point now) const {
                    // Bound the stationary wait even if one anchor keeps matching.
                    return holding && now-hold_started<10s;
                }
            };

            static bool align_to_route204_sign(VideoStream& stream, ProControllerContext& context, bool northbound = false) {
                context.wait_for_all_requests();
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BattleMenuDetector menu(BattleType::TRAINER, COLOR_GREEN);
                BlackScreenDetector black;
                WhiteScreenDetector white;
                size_t confirmations = 0, missing = 0, blocked_east = 0, detours = 0;
                double previous_dx = 1;
                bool previous_east = false;
                Route204ArrivalTracker arrival;
                for (size_t attempt = 0; attempt < 80; attempt++) {
                    const auto frame = stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Route 204 sign: No video frame.", stream);
                    if (dialogue.detect(*frame.frame) || menu.detect(*frame.frame)) {
                        stream.log("[Route 204 sign] Next trainer triggered; stopping alignment.");
                        return true;
                    }
                    double x = 0, y = 0;
                    bool detour = false, measured = false;
                    double measured_dx = 0;
                    Milliseconds duration = 250ms;
                    VideoOverlaySet overlays(stream.overlay());
                    dialogue.make_overlays(overlays);
                    menu.make_overlays(overlays);
                    if (black.detect(*frame.frame) || white.detect(*frame.frame)) {
                        confirmations = 0;
                        arrival.reset();
                    } else {
                        const auto result = northbound ? match_route204_north_approach(*frame.frame) : match_route204_sign(*frame.frame);
                        measured=result.located;measured_dx=result.dx;
                        for (const auto& landmark : result.landmarks) {
                            overlays.add(COLOR_CYAN, landmark.search);
                            if (landmark.accepted) overlays.add(COLOR_GREEN, landmark.found);
                            stream.log("[Route 204 sign] " + landmark.name + " score=" + std::to_string(landmark.score) +
                                ", margin=" + std::to_string(landmark.margin) + ", accepted=" + std::to_string(landmark.accepted) +
                                ", offset=(" + std::to_string(landmark.dx) + "," + std::to_string(landmark.dy) + ")");
                        }
                        const auto now = Route204ArrivalTracker::Clock::now();
                        // The broad pre-battle cache tolerance can accept the sign
                        // while still inside the fence's collision column.
                        bool clear_column=true;
                        if (northbound) for (const auto& landmark:result.landmarks)
                            if (landmark.accepted && std::abs(landmark.dx)>1.0/128) clear_column=false;
                        if (!clear_column) arrival.reset();
                        const bool arrived = clear_column && arrival.observe(result, now);
                        if (arrived) {
                            if (++confirmations >= 3) {
                                stream.log(northbound ? "[Route 204 sign] Left-of-sign northbound position confirmed." : "[Route 204 sign] Position confirmed; ready for the next trainer approach.");
                                return false;
                            }
                        } else if (arrival.wait_for_shadow(now)) {
                            // One strong nearby anchor is enough to hold still,
                            // but both are still required to confirm arrival.
                            confirmations = 0;
                            missing = 0;
                            duration = 250ms;
                            stream.log("[Route 204 sign] Holding near target while waiting for shadowed landmark.");
                        } else if (result.located) {
                            confirmations = 0;
                            missing = 0;
                            stream.log(std::string("[Route 204 sign] ") + (result.coarse ? "Sign-guided acquisition." : "Both landmarks located."));
                            // At the wrong height, leftward movement can hit the
                            // grass boundary. Acquire the sign's height first.
                            const bool horizontal = (!result.coarse || std::abs(result.dy)<=1.0/54) &&
                                std::abs(result.dx)>=std::abs(result.dy)*9.0/16;
                            const double error = horizontal ? result.dx : result.dy*9.0/16;
                            x = horizontal ? (result.dx>0 ? 1.0 : -1.0) : 0.0;
                            y = horizontal ? 0.0 : (result.dy<0 ? 1.0 : -1.0);
                            duration = std::abs(error)>1.0/24 ? 80ms : std::abs(error)>1.0/64 ? 40ms : 16ms;
                            if (result.coarse && std::abs(result.dx)<=1.0/128 && std::abs(result.dy)<=1.0/72) {
                                // Resume fine acquisition after the bounded shadow wait.
                                // The bounded shadow wait has ended. Make a small
                                // correction instead of holding indefinitely.
                                duration = 40ms;
                            }
                        } else {
                            confirmations = 0;
                            if (++missing > 24) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                "Route 204 sign: Landmarks missing during bounded acquisition.", stream);
                            // The sign is north of the defeated female trainer.
                            // Search that height first, then return below it.
                            if (northbound) {
                                // Tyler leaves us west of the sign. Search east
                                // along the road instead of climbing into grass.
                                x=1.0;y=0.0;
                            } else y = missing<=8 ? 1.0 : -1.0;
                            duration = 100ms;
                        }
                    }
                    if (northbound && measured && x>0 && previous_east && std::abs(previous_dx-measured_dx)<1.0/384) blocked_east++;
                    else blocked_east=0;
                    previous_east=northbound && measured && x>0;
                    // Reuse this snapshot; no movement has been sent since it.
                    if (previous_east) previous_dx=measured_dx;
                    if (blocked_east>=3) {
                        if (++detours>2) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Route 204 north approach: Tyler obstruction did not clear.",stream);
                        stream.log("[Route 204 sign] Eastward movement blocked; passing below defeated Tyler.");
                        detour=true;blocked_east=0;previous_east=false;
                    }
                    if (x != 0 || y != 0 || detour) arrival.reset();
                    // Observe throughout each correction and its settle period,
                    // so an early next-trainer trigger cancels remaining movement.
                    if (watch_route204_trainer_event(stream, context,
                        [x,y,duration,detour](ProControllerContext& ctx) {
                            if (detour) {
                                pbf_move_left_joystick(ctx,{0,-1},180ms,0ms);
                                pbf_move_left_joystick(ctx,{1,0},400ms,0ms);
                                pbf_move_left_joystick(ctx,{0,1},180ms,0ms);
                                pbf_wait(ctx,500ms);
                            } else if (x!=0 || y!=0) {
                                pbf_move_left_joystick(ctx, {x,y}, duration, 0ms);
                                pbf_wait(ctx, 500ms);
                            } else pbf_wait(ctx, duration);
                        }, "Route 204 sign alignment")) return true;
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 204 sign: Position could not be confirmed.", stream);
            }

            static void enter_ravaged_path_from_route204(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                // The cave's brown corridor and black map void distinguish an
                // indoor return from a battle fade or an outdoor scene. All ROIs
                // are normalized and visible without labels in the overlays.
                const ImageFloatBox floor_left{0.42, 0.35, 0.08, 0.10};
                const ImageFloatBox floor_right{0.52, 0.35, 0.08, 0.10};
                const ImageFloatBox map_void{0.15, 0.85, 0.10, 0.08};
                auto cave_visible = [&](const ImageRGB32& frame) {
                    for (const auto& box : {floor_left, floor_right}) {
                        const auto stats = image_stats(extract_box_reference(frame, box));
                        if (stats.average.r < stats.average.g + 12 ||
                            stats.average.g < stats.average.b + 12) return false;
                    }
                    const auto empty = image_stats(extract_box_reference(frame, map_void));
                    return empty.average.r < 20 && empty.average.g < 20 &&
                        empty.average.b < 20 && empty.stddev.sum() < 30;
                };
                auto handle_trainer = [&] {
                    stream.log("[Ravaged Path approach] Lass Samantha triggered; completing battle before continuing.");
                    wait_for_trainer_battle_menu(stream, context);
                    Barry_1_fight(env, stream, context);
                    clear_post_battle_dialogue(stream, context);
                };
                Route204ArrivalTracker arrival;
                size_t position_confirmations = 0;
                auto missing_since = std::chrono::steady_clock::now();
                bool initial_approach = true;
                size_t acquisition_pulses = 0;
                size_t blocked_north=0;
                double previous_north_dy=1;
                bool previous_coarse_north=false;
                for (size_t attempt = 0; attempt < 120; attempt++) {
                    ShortDialogWatcher dialogue(COLOR_YELLOW);
                    BattleMenuWatcher battle(BattleType::TRAINER, COLOR_GREEN);
                    BlackScreenWatcher entrance;
                    VideoOverlaySet overlays(stream.overlay());
                    overlays.add(COLOR_CYAN, floor_left);
                    overlays.add(COLOR_CYAN, floor_right);
                    overlays.add(COLOR_CYAN, map_void);
                    const auto before = stream.video().snapshot();
                    if (!before) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Ravaged Path entrance: No video frame.", stream);
                    ShortDialogDetector current_dialogue;
                    BattleMenuDetector current_battle(BattleType::TRAINER);
                    if (current_dialogue.detect(*before.frame) || current_battle.detect(*before.frame)) {
                        handle_trainer();
                        arrival.reset();
                        position_confirmations = 0;
                        missing_since = std::chrono::steady_clock::now();
                        continue;
                    }
                    double x = 0, y = 0;
                    Milliseconds duration = 250ms;
                    if (initial_approach) {
                        // Retain the northbound approach from Tyler. Subsequent
                        // movement is landmark-guided, without the old right sweep.
                        initial_approach = false;
                        y = 1;
                        duration = 2400ms;
                    } else {
                        auto result = match_ravaged_path_south_entrance(*before.frame);
                        for (const auto& landmark : result.landmarks) {
                            overlays.add(COLOR_CYAN, landmark.search);
                            if (landmark.accepted) overlays.add(COLOR_GREEN, landmark.found);
                            stream.log("[Ravaged Path locator] " + landmark.name +
                                " score=" + std::to_string(landmark.score) +
                                ", margin=" + std::to_string(landmark.margin) +
                                ", accepted=" + std::to_string(landmark.accepted) +
                                ", offset=(" + std::to_string(landmark.dx) + "," + std::to_string(landmark.dy) + ")");
                        }
                        const auto now = std::chrono::steady_clock::now();
                        auto fixed = result;
                        fixed.landmarks.resize(2);
                        // Cave entry needs tighter horizontal alignment than the
                        // standing area beside the Route 204 sign.
                        bool horizontal_ready = true;
                        for (const auto& anchor : fixed.landmarks)
                            if (anchor.accepted && std::abs(anchor.dx)>1.0/64) horizontal_ready = false;
                        if (!horizontal_ready) arrival.reset();
                        const bool arrived = horizontal_ready && arrival.observe(fixed, now);
                        if (arrived) {
                            if (++position_confirmations >= 3) {
                                stream.log("[Ravaged Path locator] Entrance position confirmed; moving north into cave.", COLOR_GREEN);
                                y = 1;
                                duration = 1200ms;
                            }
                        } else if (arrival.wait_for_shadow(now)) {
                            position_confirmations = 0;
                            stream.log("[Ravaged Path locator] Holding still for shadowed landmark.");
                        } else if (result.located) {
                            missing_since = now;
                            position_confirmations = 0;
                            // Escape the upper-right alcove vertically before
                            // moving left. Near the doorway, correct horizontal first.
                            if (std::abs(result.dy)>1.0/36) {
                                y = result.dy>0 ? -1.0 : 1.0;
                            } else if (std::abs(result.dx)>1.0/64) {
                                x = result.dx>0 ? 1.0 : -1.0;
                            }
                            duration = 80ms;
                            if (result.coarse && y>0 && previous_coarse_north &&
                                std::abs(result.dy-previous_north_dy)<1.0/384) blocked_north++;
                            else blocked_north=0;
                            previous_north_dy=result.dy;
                            previous_coarse_north=result.coarse && y>0;
                            if (blocked_north>=4) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                "Ravaged Path locator: Northward coarse acquisition blocked; refusing repeated movement.",stream);
                            if (result.coarse && x==0 && y==0) {
                                // After the stationary shadow wait, correct a
                                // small height error that may still change the
                                // sign's perspective enough to prevent matching.
                                if (std::abs(result.dy)<=1.0/72)
                                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                        "Ravaged Path locator: Near entrance, but sign remained shadowed; refusing blind correction.", stream);
                                y = result.dy<0 ? 1.0 : -1.0;
                                duration = 40ms;
                            }
                            if (y<0) {
                                // Fixed entrance geometry predicts the lass's
                                // feet when she is shadowed/off-screen. A visible
                                // lass may tighten this limit, never relax it.
                                double boundary = 930.0/1080 + result.dy;
                                if (result.landmarks.size()>2 && result.landmarks[2].accepted)
                                    boundary = std::min(boundary,
                                        result.landmarks[2].found.y + result.landmarks[2].found.height);
                                // Leave >12% screen height above her before any
                                // short down pulse. No blind southward search.
                                if (boundary - 0.55 <= 0.12) {
                                    y = 0;
                                    stream.log("[Ravaged Path locator] Downward correction blocked by lass boundary.", COLOR_RED);
                                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                        "Ravaged Path locator: Southern safety boundary reached.", stream);
                                }
                            }
                        } else {
                            position_confirmations = 0;
                            if (now-missing_since>10s) {
                                // Failed recording: still beside the Route 204
                                // sign, blocked below the northbound approach.
                                // Only this recognizable route authorizes search.
                                const auto route = match_route204_sign(*before.frame);
                                const bool known_route = !route.landmarks.empty() &&
                                    route.landmarks[0].accepted && route.landmarks[0].score>=.94 &&
                                    route.landmarks[0].margin>=.30;
                                if (known_route && acquisition_pulses<8) {
                                    const bool sidestep = acquisition_pulses++ % 2 == 0;
                                    x = sidestep ? -1.0 : 0.0;
                                    y = sidestep ? 0.0 : 1.0;
                                    duration = sidestep ? 150ms : 500ms;
                                    stream.log("[Ravaged Path locator] Recognized Route 204 sign; bounded left/north acquisition.");
                                } else OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                    "Ravaged Path locator: Landmarks unavailable after bounded acquisition.", stream);
                            }
                        }
                    }
                    if (x!=0 || y!=0) {
                        arrival.reset();
                        position_confirmations = 0;
                        missing_since = std::chrono::steady_clock::now();
                    }
                    const int ret = run_until<ProControllerContext>(stream, context,
                        [x,y,duration](ProControllerContext& ctx) {
                            if (x!=0 || y!=0) {
                                pbf_move_left_joystick(ctx, {x,y}, duration, 0ms);
                                pbf_wait(ctx, 500ms);
                            } else pbf_wait(ctx, duration);
                        }, {{entrance}, {dialogue}, {battle}});
                    context.wait_for_all_requests();
                    if (ret == 1 || ret == 2) {
                        handle_trainer();
                        arrival.reset();
                        position_confirmations = 0;
                        missing_since = std::chrono::steady_clock::now();
                        continue;
                    }
                    if (ret != 0) continue;
                    // A trainer transition can also go black. Require a stable
                    // cave scene, or route the detected battle through combat.
                    const auto deadline = std::chrono::steady_clock::now() + 15s;
                    size_t confirmed = 0;
                    bool trainer_seen = false;
                    while (std::chrono::steady_clock::now() < deadline) {
                        const auto frame = stream.video().snapshot();
                        if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Ravaged Path entrance: No frame after transition.", stream);
                        if (current_dialogue.detect(*frame.frame) || current_battle.detect(*frame.frame)) {
                            trainer_seen = true;
                            break;
                        }
                        if (cave_visible(*frame.frame)) {
                            if (++confirmed >= 5) {
                                stream.log("[Ravaged Path entrance] Transition and indoor corridor confirmed.", COLOR_GREEN);
                                pbf_wait(context, 700ms);
                                context.wait_for_all_requests();
                                return;
                            }
                        } else confirmed = 0;
                        pbf_wait(context, 100ms);
                        context.wait_for_all_requests();
                    }
                    if (trainer_seen) handle_trainer();
                    arrival.reset();
                    position_confirmations = 0;
                    missing_since = std::chrono::steady_clock::now();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Ravaged Path entrance: Could not confirm entry after bounded approach.", stream);
            }

            static void exit_ravaged_path_to_route204(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                stream.log("[Ravaged Path north exit] Right wall, lower wall, then left/down search.");
                // Observe the entire approach: reaching the exit during either
                // wall movement must cancel the remaining queued commands.
                WhiteScreenWatcher white;
                const int result=run_until<ProControllerContext>(stream,context,
                    [](ProControllerContext& ctx) {
                        // Deliberate overtravel establishes the walls despite
                        // variation in the position after Rock Smash.
                        pbf_move_left_joystick(ctx,{1,0},3000ms,100ms);
                        pbf_move_left_joystick(ctx,{0,-1},1800ms,100ms);
                        for (size_t attempt=0; attempt<30; attempt++) {
                            pbf_move_left_joystick(ctx,{-1,0},160ms,80ms);
                            pbf_move_left_joystick(ctx,{0,-1},120ms,80ms);
                            ctx.wait_for_all_requests();
                        }
                        pbf_wait(ctx,1500ms);
                    },{{white}});
                if (result!=0) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Ravaged Path north exit: No white transition during left/down search.",stream);
                context.wait_for_all_requests();
                stream.log("[Ravaged Path north exit] White transition detected; movement stopped.");
                WhiteScreenWatcher visible(COLOR_RED,{0.1,0.1,0.8,0.8},500,10,
                    WhiteScreenWatcher::FinderType::GONE,500ms);
                if (wait_until(stream,context,15s,{{visible}})!=0)
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Ravaged Path north exit: White transition did not finish.",stream);
                pbf_wait(context,700ms);
                context.wait_for_all_requests();
            }

            void roark_to_floaroma(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 550ms, 0ms);
                pbf_wait(context, 1046ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2000ms, 0ms);
                pbf_wait(context, 288ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 250ms, 0ms);
                pbf_wait(context, 1009ms);
                align_to_oreburgh_mart(stream, context);
                pbf_wait(context, 1009ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1128ms, 0ms);
                pbf_wait(context, 2924ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 885ms, 0ms);
                buy_items(stream, context, { {ShopItem::Repel, BUY_MAX} });
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1193ms, 0ms);
                pbf_wait(context, 2226ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2544ms, 0ms);
                pbf_wait(context, 753ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 464ms, 0ms);
                pbf_wait(context, 519ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1467ms, 0ms);
                pbf_wait(context, 2816ms);
                // Barry conversation
                wait_for_dialogue(stream, context, "Barry conversation");
                context.wait_for_all_requests();

                ShortDialogDetector dialogue(COLOR_YELLOW);
                const auto started = std::chrono::steady_clock::now();
                auto absent_since = started;

                while (true) {
                    const auto frame = stream.video().snapshot();
                    if (!frame) {
                        OperationFailedExceptionWithScreenshot::fire(
                            ErrorReportMode::SEND_ERROR_REPORT,
                            "Barry conversation: No video frame.", stream
                        );
                    }

                    const auto now = std::chrono::steady_clock::now();
                    if (now - started >= 180s) {
                        OperationFailedExceptionWithScreenshot::fire(
                            ErrorReportMode::SEND_ERROR_REPORT,
                            "Barry conversation: Dialogue did not finish.", stream
                        );
                    }

                    if (dialogue.detect(*frame.frame)) {
                        absent_since = now;
                    }
                    else if (now - absent_since >= 2s) {
                        break;
                    }

                    // Keep mashing through pauses between dialogue boxes.
                    pbf_mash_button(context, BUTTON_B, 100ms);
                    context.wait_for_all_requests();
                }

                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 282ms, 0ms);
                pbf_wait(context, 483ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 218ms, 0ms);
                pbf_wait(context, 482ms);
                enter_oreburgh_gate_from_city(stream, context);
                activate_repel(stream, context, 5);
                exit_oreburgh_gate_to_route203(stream, context);
                // Align north, then go only far enough west to jump the ledge.
                // Avoid descending the upper stairs and climbing straight back up.
                pbf_move_left_joystick(context, {0.0, 1.0}, 600ms, 0ms);
                pbf_wait(context, 400ms);
                pbf_move_left_joystick(context, {-1.0, 0.0}, 2100ms, 0ms);
                pbf_wait(context, 300ms);
                pbf_move_left_joystick(context, {0.0, -1.0}, 1700ms, 0ms);
                pbf_wait(context, 500ms);
                // Cross the lower stairs at the offset needed for the next north leg.
                // Combine the old left/right correction and the two upward moves.
                pbf_move_left_joystick(context, {-1.0, 0.0}, 850ms, 0ms);
                pbf_wait(context, 150ms);
                pbf_move_left_joystick(context, {0.0, 1.0}, 1800ms, 0ms);
                pbf_wait(context, 150ms);
                pbf_move_left_joystick(context, {-1.0, 0.0}, 1400ms, 0ms);
                pbf_wait(context, 100ms);
                pbf_move_left_joystick(context, {0.0, -1.0}, 1000ms, 0ms);
                pbf_wait(context, 100ms);
                pbf_move_left_joystick(context, {-1.0, 0.0}, 1600ms, 0ms);
                pbf_wait(context, 600ms);
                // Exhaust the active Repel and decline renewal before the safe
                // Jubilife approach, story battle, and Pokemon Center sequence.
                force_repel_expiry(stream, context, RepelRenewalChoice::NO);
                // One clock setup covers this whole new locator strip: street,
                // Galactic plaza approach, and the post-battle return. Existing
                // locators elsewhere keep their current time-independent behavior.
                stream.log("[Jubilife locator setup] Setting 13:00 once for the street/plaza locator strip; preserving the date.");
                DateTime locator_time;
                locator_time.hour = 13;
                locator_time.minute = 0;
                locator_time.second = 0;
                set_time_fast(env, context, locator_time, true);
                pbf_wait(context, 5000ms);
                context.wait_for_all_requests();
                // Stop east of the TV-station overshoot seen in the recording.
                // Acquire/correct the street lane before moving north; the locator
                // now supplies vertical travel instead of the blind 2579ms up move.
                pbf_move_left_joystick(context, {-1.0, 0.0}, 3700ms, 0ms);
                pbf_wait(context, 850ms);
                align_to_overworld_landmarks(stream, context, match_jubilife_street_approach,
                    "Jubilife street alignment", false, true);
                // Stop the upward approach if the story dialogue starts early.
                ShortDialogWatcher plaza_story(COLOR_YELLOW);
                run_until<ProControllerContext>(stream, context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx, {0.0, 1.0}, 2600ms, 0ms);
                        pbf_wait(ctx, 250ms);
                    }, {{plaza_story}});
                align_to_overworld_landmarks(stream, context, match_jubilife_plaza_approach, "Jubilife plaza alignment", true);
                approach_galactic_dialogue(stream, context);
                wait_for_trainer_battle_menu(stream, context);
                Barry_1_fight(env, stream, context);
                clear_post_battle_dialogue(stream, context);
                pbf_wait(context, 2000ms);
                clear_post_battle_dialogue(stream, context);
                // Rowan and the Poketch president leave the player north of the
                // reference. Bring the plaza landmarks back into view first.
                stream.log("[Galactic return] Dialogue cleared; short downward return before reacquiring the plaza.");
                pbf_move_left_joystick(context, {0.0, -1.0}, 600ms, 0ms);
                pbf_wait(context, 700ms);
                context.wait_for_all_requests();
                align_to_overworld_landmarks(stream, context, match_jubilife_plaza_approach,
                    "Jubilife post-battle plaza alignment");
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 5141ms, 0ms);
                pbf_wait(context, 749ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 868ms, 0ms);
                pbf_wait(context, 1073ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 431ms, 0ms);
                pbf_wait(context, 3099ms);
                // Stop outside: the old additional 1028ms up move sometimes
                // entered the building before exterior alignment even started.
                // Repel is inactive here; no expiry search is needed before the Center.
                align_and_enter_jubilife_center(stream, context);
                if (!heal_pokemon(stream, context, "Jubilife"))
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Jubilife Center: Healing/exit did not complete.", stream);

                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1440ms, 0ms);
                pbf_wait(context, 2259ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 765ms, 0ms);
                pbf_wait(context, 439ms);
                const bool missed_trainer = !approach_route204_female_trainer(stream, context);
                stream.log("[Route 204 female trainer] missed_trainer=" + std::to_string(missed_trainer));
                if (missed_trainer) recover_route204_female_trainer(stream, context);
                wait_for_trainer_battle_menu(stream, context);
                Barry_1_fight(env, stream, context);
                clear_post_battle_dialogue(stream, context);
                // Resume encounter protection after the Center and the following battle.
                if (!activate_repel(stream, context, 6)) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Post-Center battle: Could not activate a new Repel from the Bag.", stream);
                }
                context.wait_for_all_requests();
                const bool next_trainer_started = align_to_route204_sign(stream, context);
                if (!next_trainer_started) {
                    const bool triggered = watch_route204_trainer_event(stream, context,
                        [](ProControllerContext& ctx) {
                            pbf_move_left_joystick(ctx, {-1.0, 0.0}, 1200ms, 0ms);
                            pbf_wait(ctx, 2500ms);
                        }, "Route 204 next trainer approach");
                    if (!triggered) recover_route204_youngster(stream, context);
                }
                // An early locator interruption still has to complete combat;
                // it must never fall through into overworld movement mid-battle.
                wait_for_trainer_battle_menu(stream, context);
                Barry_1_fight(env, stream, context);
                clear_post_battle_dialogue(stream, context);
                // Keep the recently activated Repel through Ravaged Path.
                // Reacquire west of the sign after Tyler before moving north;
                // do not spend its remaining steps on an early expiry search.
                bool northbound_aligned=false;
                for (size_t recovery=0; recovery<3; recovery++) {
                    if (!align_to_route204_sign(stream,context,true)) {northbound_aligned=true;break;}
                    wait_for_trainer_battle_menu(stream,context);
                    Barry_1_fight(env,stream,context);
                    clear_post_battle_dialogue(stream,context);
                }
                if (!northbound_aligned) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 204 north approach: Trainer interruptions exhausted.",stream);
                enter_ravaged_path_from_route204(env, stream, context);
                //cave navigation

                // Ravaged Path -> Floaroma -> Valley Windworks -> Mars.
                // First-pass route from ravagedpath.txt and the 2026-10-13
                // 19-48-08 recording. Timings need live route validation.
                auto move = [&](double x, double y, Milliseconds duration, Milliseconds settle = 250ms) {
                    pbf_move_left_joystick(context, {x, y}, duration, 0ms);
                    pbf_wait(context, settle);
                    context.wait_for_all_requests();
                };
                auto battle = [&](const std::string& label) {
                    stream.log("[Segment 02 battle] " + label);
                    wait_for_trainer_battle_menu(stream, context);
                    Barry_1_fight(env, stream, context);
                    clear_post_battle_dialogue(stream, context);
                };
                // Break the blocking rock using the existing HM confirmation helper.
                move(0, 1, 1100ms, 900ms);
                move(1, 0, 600ms);
                use_HM(stream, context, "Rock Smash in Ravaged Path");
                context.wait_for_all_requests();
                clear_post_battle_dialogue(stream, context);
                // Failed frames stop against the remaining center rocks. Pass
                // above their collision row before the eastward exit approach.
                move(0,1,200ms);
                exit_ravaged_path_to_route204(stream,context);
                move(0, -1, 350ms, 500ms);

                // Resolve the current Repel before searching for Teighler so
                // its expiry prompt cannot interrupt trainer acquisition.
                // Renew here to protect the remaining route to Floaroma.
                force_repel_expiry(stream, context, RepelRenewalChoice::YES);

                // Route 204 north: go around the cliff to Aroma Lady Teighler.
                move(-1, 0, 1300ms);
                move(0, 1, 1300ms, 650ms);
                // Timed approach: get onto the alcove's left edge, descend
                // to its bottom ledge, then cross right through her sightline.
                // Watch the entire approach so an early trigger stops movement.
                if (!watch_route204_trainer_event(stream, context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx, {1, 0}, 600ms, 100ms);
                        pbf_move_left_joystick(ctx, {0, -1}, 600ms, 100ms);
                        pbf_move_left_joystick(ctx, {1, 0}, 1000ms, 0ms);
                        pbf_wait(ctx, 3000ms);
                    }, "Aroma Lady Teighler timed ledge approach")) {
                    recover_teighler_trigger(stream, context);
                }
                stream.log("[Segment 02 battle] Aroma Lady Teighler: Flamethrower (move 2).");
                wait_for_trainer_battle_menu(stream,context);
                fight_trainer_using_move_slot(stream,context,2,"Aroma Lady Teighler (Flamethrower)");
                clear_post_battle_dialogue(stream,context);

                // Omit the Awakening. The grass north of Teighler can trigger
                // the Bug Catcher: finish that battle before any Center locator.
                // Small watched steps preserve the remaining route if he interrupts.
                size_t northbound_battles = 0;
                ShortDialogDetector northbound_dialogue(COLOR_YELLOW);
                BattleMenuDetector northbound_menu(BattleType::TRAINER, COLOR_GREEN);
                auto northbound_move = [&](double x, double y, Milliseconds remaining, bool stop_after_battle = false) {
                    while (remaining > 0ms) {
                        const Milliseconds step = remaining > 200ms ? 200ms : remaining;
                        // A dialogue can appear at the end of the previous
                        // short step. Check it directly before restarting the
                        // watcher's 250ms confirmation window or sending input.
                        const auto frame = stream.video().snapshot();
                        if (!frame) OperationFailedExceptionWithScreenshot::fire(
                            ErrorReportMode::SEND_ERROR_REPORT,
                            "Route 204 northbound: No video frame.", stream);
                        const bool interrupted = northbound_dialogue.detect(*frame.frame) ||
                            northbound_menu.detect(*frame.frame) ||
                            watch_route204_trainer_event(stream, context,
                                [x,y,step](ProControllerContext& ctx) {
                                    pbf_move_left_joystick(ctx, {x,y}, step, 0ms);
                                }, "Route 204 northbound movement");
                        if (interrupted) {
                            if (++northbound_battles > 2) OperationFailedExceptionWithScreenshot::fire(
                                ErrorReportMode::SEND_ERROR_REPORT,
                                "Route 204 northbound: Too many trainer interruptions.", stream);
                            battle("Route 204 north Bug Catcher");
                            // The sign locator establishes the next turn after this
                            // battle; do not finish an obsolete northward timer.
                            if (stop_after_battle) return;
                            // Replay only this possibly interrupted short step,
                            // never the complete pre-battle travel duration.
                        } else remaining -= step;
                    }
                };
                northbound_move(1, 0, 2000ms); // Reach the right wall.
                northbound_move(0, 1, 450ms);  // Clear the corner above it.
                northbound_move(1, 0, 1200ms); // Move into the northbound lane.
                // Restore the north/west turns from the successful manual
                // recording. The former 3200ms up stopped at the Bug Catcher,
                // well short of town, and omitted the flower-bed detour.
                northbound_move(0, 1, 2200ms, true);
                // Locate left to the sign after the Bug Catcher, or proceed
                // directly if skipped. Do not seek out a missed trainer.
                for (size_t recovery=0; align_to_route204_floaroma_turn(stream,context); recovery++) {
                    if (recovery>=2) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Route 204 north turn: Too many trainer interruptions.", stream);
                    battle("Route 204 north Bug Catcher");
                }
                northbound_move(-1, 0, 1100ms);
                northbound_move(0, 1, 150ms);
                northbound_move(-1, 0, 1200ms);
                northbound_move(-1, 0, 800ms);
                // Continue through Floaroma's southern fence opening before
                // turning east. The shorter approach stopped against the fence.
                stream.log("[Floaroma approach] Moving farther north before Center approach.");
                northbound_move(0, 1, 2700ms);
                // Expire the Repel in the town approach lane before moving
                // toward the Center. Alignment corrects any partial final step.
                force_repel_expiry(stream, context, RepelRenewalChoice::NO, true);
                establish_floaroma_house_exit(stream,context);
                context.wait_for_all_requests();
                stream.log("[Segment 02] Floaroma house exit established; ready for town checkpoint.", COLOR_GREEN);
            }

            static void floaroma_to_mars(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {
                auto move = [&](double x, double y, Milliseconds duration, Milliseconds settle = 250ms) {
                    pbf_move_left_joystick(context, {x, y}, duration, 0ms);
                    pbf_wait(context, settle);
                    context.wait_for_all_requests();
                };
                auto transition = [&](double x, double y, Milliseconds duration, const std::string& label) {
                    context.wait_for_all_requests();
                    stream.log("[Route transition] " + label);
                    BlackScreenWatcher black;
                    WhiteScreenWatcher white;
                    const int ret = run_until<ProControllerContext>(stream, context,
                        [x, y, duration](ProControllerContext& ctx) {
                            pbf_move_left_joystick(ctx, {x, y}, duration, 0ms);
                            pbf_wait(ctx, 1500ms);
                            // Bounded doorway correction if the recorded move stops short.
                            for (size_t pulse = 0; pulse < 10; pulse++) {
                                pbf_move_left_joystick(ctx, {x, y}, 100ms, 100ms);
                            }
                        }, {{black}, {white}});
                    if (ret < 0) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        label + ": Entrance/exit transition not detected.", stream);
                    context.wait_for_all_requests();
                    BlackScreenDetector black_frame;
                    WhiteScreenDetector white_frame;
                    // Accept either fade type, then require a stable return before moving.
                    const auto deadline = std::chrono::steady_clock::now() + 15s;
                    auto clear_since = std::chrono::steady_clock::now();
                    while (std::chrono::steady_clock::now() < deadline) {
                        const auto frame = stream.video().snapshot();
                        if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            label + ": No video frame during transition.", stream);
                        const auto now = std::chrono::steady_clock::now();
                        if (black_frame.detect(*frame.frame) || white_frame.detect(*frame.frame)) clear_since = now;
                        else if (now - clear_since >= 500ms) {
                            pbf_wait(context, 700ms);
                            context.wait_for_all_requests();
                            return;
                        }
                        pbf_wait(context, 100ms);
                        context.wait_for_all_requests();
                    }
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        label + ": Transition did not finish.", stream);
                };
                auto battle = [&](const std::string& label) {
                    stream.log("[Segment 02 battle] " + label);
                    wait_for_trainer_battle_menu(stream, context);
                    Barry_1_fight(env, stream, context);
                    clear_post_battle_dialogue(stream, context);
                };
                auto talk = [&](const std::string& label, Button response = BUTTON_B) {
                    stream.log("[Segment 02 dialogue] " + label);
                    pbf_press_button(context, BUTTON_A, 150ms, 500ms);
                    context.wait_for_all_requests();
                    wait_for_dialogue(stream, context, label, 5000ms);
                    clear_post_battle_dialogue(stream, context, response);
                };

                // The house exit fixes the start pose. Use the median of the
                // twenty successful recorded approaches: 24 south taps, then
                // nine east taps. Retain the recorded tap cadence; replacing
                // it with one continuous hold changes acceleration/step timing.
                stream.log("[Floaroma Center] Blind south/east approach from the house exit.");
                for (size_t step=0;step<24;step++) move(0,-1,80ms,250ms);
                for (size_t step=0;step<9;step++) move(1,0,80ms,500ms);
                transition(0, 1, 950ms, "Floaroma Pokemon Center entrance");
                // heal_pokemon includes a watched exit; do not exit a second time.
                if (!heal_pokemon(stream, context, "Floaroma"))
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Floaroma Center: Healing/exit did not complete.", stream);
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
                // Clear the Center doorway to the west before turning north,
                // so we cannot walk back through the door. Repel is already off.
                move(-1, 0, 1500ms);

                // Meet the girl on Route 205, then return for the Works Key.
                move(0, 1, 1000ms);
                // Include the westward clearance and the former 650 ms east leg.
                move(1, 0, 6550ms, 900ms);
                talk("Route 205 girl requesting help for her father");
                move(-1, 0, 4500ms, 500ms);
                // Repel is already inactive; omit the recorded renewal A presses.
                move(-1, 0, 3000ms);
                move(0, 1, 2300ms);
                // The westward overtravel establishes the tree wall. The
                // recording then stops west of the Meadow gap: move right
                // into that opening instead of pushing farther left.
                move(1, 0, 300ms);
                transition(0, 1, 1000ms, "Floaroma Meadow entrance");
                move(0, 1, 1100ms);
                pbf_press_button(context, BUTTON_A, 150ms, 500ms);
                stream.log("[Segment 02 battle] Floaroma Meadow first Galactic grunt.");
                wait_for_trainer_battle_menu(stream, context);
                Barry_1_fight(env, stream, context);
                // The second grunt starts another battle automatically. Do not
                // wait for quiet overworld or send movement between the fights.
                battle("Floaroma Meadow second Galactic grunt");
                talk("Floaroma Meadow honey seller and Works Key");
                if (!clear_new_item_flags(stream,context,6)) OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,"Floaroma Meadow: Bag NEW flags did not clear.",stream);
                transition(0, -1, 1000ms, "Floaroma Meadow exit");
                move(0, -1, 2600ms);

                // No Repel is needed on the town/road route to Valley Windworks.
                move(1, 0, 4200ms);
                move(0, -1, 200ms);
                move(1, 0, 5500ms);
                move(0, -1, 500ms);
                move(1, 0, 1200ms);
                align_to_windworks_sign(stream, context);
                // From the sign-side pose, the building wall fixes north/south
                // position and the grunt blocks the subsequent eastward travel.
                move(0, 1, 2000ms);
                move(1, 0, 1500ms);
                pbf_press_button(context, BUTTON_A, 150ms, 500ms);
                battle("Valley Windworks door Galactic grunt");

                // Probe the door after the grunt retreats. Cancel all remaining
                // movement as soon as dialogue appears, preserving doorway alignment.
                bool door_dialogue=false;
                for (size_t attempt=0; attempt<20 && !door_dialogue; attempt++) {
                    ShortDialogWatcher dialogue(COLOR_YELLOW);
                    door_dialogue=run_until<ProControllerContext>(stream,context,
                        [](ProControllerContext& ctx) {
                            pbf_move_left_joystick(ctx,{0,1},100ms,0ms);
                            pbf_press_button(ctx,BUTTON_A,150ms,400ms);
                            pbf_move_left_joystick(ctx,{1,0},40ms,0ms);
                            pbf_wait(ctx,250ms);
                        },{{dialogue}})>=0;
                    context.wait_for_all_requests();
                }
                if (!door_dialogue) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Valley Windworks: Door dialogue not detected after bounded up/A/right probes.",stream);
                stream.log("[Windworks door] Dialogue detected; confirm Works Key for two seconds.");
                pbf_mash_button(context,BUTTON_A,2000ms);
                context.wait_for_all_requests();
                // Interaction reaches the door from west of its entrance lane.
                // One additional recorded nudge centers the subsequent northward entry.
                move(1, 0, 40ms);
                transition(0, 1, 1000ms, "Valley Windworks entrance");
                clear_post_battle_dialogue(stream, context); // Entrance warning from the grunt.
                move(-1, 0, 2000ms); // Combined 1210ms + 817ms, separated by only 5ms.
                bool fought_grunt=watch_route204_trainer_event(stream, context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx, {0, 1}, 1250ms, 0ms);
                        pbf_wait(ctx, 2500ms);
                    }, "Valley Windworks interior grunt approach");
                if (fought_grunt) battle("Valley Windworks interior Galactic grunt");
                auto grunt_position=[&]() {
                    OverworldMatchResult position;
                    for (size_t attempt=0;attempt<12;attempt++) {
                        context.wait_for_all_requests();
                        const auto frame=stream.video().snapshot();
                        if (frame) position=match_windworks_interior_grunt(*frame.frame);
                        if (position.located) return position;
                        pbf_wait(context,250ms);
                    }
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Windworks interior: Could not determine player position relative to the grunt.",stream);
                    return position;
                };
                auto position=grunt_position();
                if (!fought_grunt && position.dy<=.025) {
                    stream.log("[Windworks interior] Below/level with grunt; recover north and interact.");
                    for (size_t attempt=0;attempt<20 && !fought_grunt;attempt++) {
                        fought_grunt=watch_route204_trainer_event(stream,context,
                            [](ProControllerContext& ctx) {
                                pbf_move_left_joystick(ctx,{0,1},100ms,0ms);
                                pbf_press_button(ctx,BUTTON_A,150ms,500ms);
                            },"Windworks interior below-grunt recovery");
                    }
                    if (!fought_grunt) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Windworks interior: North/A recovery did not trigger the grunt battle.",stream);
                    battle("Valley Windworks interior Galactic grunt (below recovery)");
                }
                auto align_above_grunt=[&]() {
                    // Re-establish the west wall after a below-grunt interaction,
                    // then climb past his row before crossing the partition gap.
                    move(-1,0,1000ms);
                    bool above=false;
                    for (size_t attempt=0;attempt<20;attempt++) {
                        position=grunt_position();
                        if (position.dy>.025) {above=true;break;}
                        move(0,1,80ms);
                    }
                    if (!above) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Windworks interior: Could not get above the defeated grunt.",stream);
                };
                if (fought_grunt) align_above_grunt();
                else stream.log("[Windworks interior] Above grunt; try the rightward crossing for his battle.");
                const bool crossing_event=watch_route204_trainer_event(stream,context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx,{1,0},750ms,0ms);
                        pbf_wait(ctx,2500ms);
                    },"Windworks interior upper crossing");
                if (crossing_event) {
                    if (!fought_grunt) battle("Valley Windworks interior Galactic grunt (upper crossing)");
                    else clear_post_battle_dialogue(stream,context);
                    // The event cancelled travel at an unknown point. Reacquire
                    // the west wall and upper row, then cross from that known pose.
                    align_above_grunt();
                    move(1,0,750ms,450ms);
                } else if (!fought_grunt) {
                    stream.log("[Windworks interior] Rightward crossing completed without a battle; grunt missed, continuing to Mars.");
                }

                // The upper crossing above replaces the old blind north/right
                // pair. Retain the remaining turns between the machines.
                move(0, -1, 300ms, 0ms);
                move(1, 0, 900ms);
                move(0, 1, 250ms);
                move(1, 0, 550ms, 400ms);
                move(0, 1, 200ms);
                move(1, 0, 750ms, 500ms);
                pbf_press_button(context, BUTTON_A, 150ms, 500ms);
                battle("Commander Mars");
                clear_post_battle_dialogue(stream, context); // Father/daughter reunion after Mars leaves.
                stream.log("[Segment 02] Mars defeated; post-battle dialogue cleared.", COLOR_GREEN);

            }
            static void align_to_route205_marker(VideoStream& stream, ProControllerContext& context, bool forest) {
                context.wait_for_all_requests();
                const std::string label=forest?"Route 205 forest entrance":"Route 205 bridge";
                size_t confirmed=0,missing=0;
                bool acquired=false;
                for (size_t attempt=0;attempt<80;attempt++) {
                    const auto frame=stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        label+": No video frame.",stream);
                    const auto result=forest?match_route205_forest_entrance(*frame.frame):match_route205_bridge(*frame.frame);
                    if (result.at_target) {
                        acquired=true;missing=0;
                        if (++confirmed>=3) {stream.log("["+label+"] Sign position confirmed.",COLOR_GREEN);return;}
                        pbf_wait(context,150ms);
                    } else {
                        confirmed=0;
                        double x=0,y=1;
                        Milliseconds duration=80ms;
                        if (result.located) {
                            acquired=true;missing=0;
                            stream.log("["+label+"] Sign offset=("+std::to_string(result.dx)+","+std::to_string(result.dy)+")");
                            const bool horizontal=std::abs(result.dx)>1.0/50;
                            const double error=horizontal?std::abs(result.dx):std::abs(result.dy);
                            x=horizontal?(result.dx<0?-1:1):0;
                            y=horizontal?0:(result.dy>0?-1:1);
                            duration=error<.08?40ms:80ms;
                        } else if (++missing>12) {
                            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                label+": Sign unavailable during bounded acquisition.",stream);
                        } else if (acquired) {
                            // Never walk past a sign after losing a trusted match.
                            pbf_wait(context,250ms);
                            context.wait_for_all_requests();
                            continue;
                        }
                        pbf_move_left_joystick(context,{x,y},duration,0ms);
                        pbf_wait(context,250ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label+": Sign position did not converge.",stream);
            }

            static void align_to_route205_bank_tree(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                size_t confirmed=0,missing=0;
                for (size_t attempt=0;attempt<80;attempt++) {
                    const auto frame=stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Route 205 bank tree: No video frame.",stream);
                    const auto result=match_route205_bank_tree(*frame.frame);
                    if (!result.located) {
                        confirmed=0;
                        if (++missing>=12) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Route 205 bank tree: Tree and blue-hat child unavailable; holding position.",stream);
                        pbf_wait(context,250ms);
                    } else if (result.at_target) {
                        missing=0;
                        if (++confirmed>=3) {stream.log("[Route 205 bank tree] Position confirmed.",COLOR_GREEN);return;}
                        pbf_wait(context,150ms);
                    } else {
                        confirmed=0;missing=0;
                        stream.log("[Route 205 bank tree] Offset=("+std::to_string(result.dx)+","+std::to_string(result.dy)+")");
                        // Clear the north bridge lip before attempting to step west.
                        // Once at bank height, keep horizontal priority beside the tree.
                        const bool clear_bank=result.dy < -1.0/54;
                        const bool horizontal=!clear_bank && std::abs(result.dx)>1.0/96;
                        const double error=horizontal?std::abs(result.dx):std::abs(result.dy);
                        const double x=horizontal?(result.dx<0?-1:1):0;
                        const double y=horizontal?0:(result.dy>0?-1:1);
                        pbf_move_left_joystick(context,{x,y},error<.08?40ms:80ms,0ms);
                        pbf_wait(context,250ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Route 205 bank tree: Position did not converge.",stream);
            }

            static void align_route205_patrol_position(VideoStream& stream, ProControllerContext& context,
                const std::function<OverworldMatchResult(const ImageRGB32&)>& match, const std::string& label) {
                size_t confirmed=0,missing=0;
                for (size_t attempt=0;attempt<100;attempt++) {
                    context.wait_for_all_requests();
                    const auto frame=stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        label+": No video frame during patrol positioning.",stream);
                    const auto result=match(*frame.frame);
                    if (!result.located) {
                        confirmed=0;
                        if (++missing>=16) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            label+": Patrol landmarks unavailable; holding position.",stream);
                        pbf_wait(context,250ms);
                    } else if (result.at_target) {
                        missing=0;
                        if (++confirmed>=3) {stream.log("["+label+"] Patrol position confirmed.",COLOR_GREEN);return;}
                        pbf_wait(context,150ms);
                    } else {
                        confirmed=0;missing=0;
                        stream.log("["+label+"] Patrol offset=("+std::to_string(result.dx)+","+std::to_string(result.dy)+")");
                        // Correct height first so the subsequent horizontal step
                        // stays on the stair landing rather than hitting its cliff.
                        const bool vertical=std::abs(result.dy)>1.0/54;
                        const double error=vertical?std::abs(result.dy):std::abs(result.dx);
                        pbf_move_left_joystick(context,
                            {vertical?0:(result.dx<0?-1.0:1.0),vertical?(result.dy>0?-1.0:1.0):0},
                            error<.08?40ms:80ms,0ms);
                        pbf_wait(context,250ms);
                    }
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label+": Patrol position did not converge.",stream);
            }

            void mars_to_eterna_forest(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
                auto move=[&](double x,double y,Milliseconds duration,Milliseconds settle=250ms) {
                    pbf_move_left_joystick(context,{x,y},duration,0ms);
                    pbf_wait(context,settle);
                    context.wait_for_all_requests();
                };
                auto battle=[&](const std::string& label) {
                    stream.log("[Route 205 battle] "+label);
                    wait_for_trainer_battle_menu(stream,context);
                    Barry_1_fight(env,stream,context);
                    clear_post_battle_dialogue(stream,context);
                };
                auto patrol_battle=[&](const std::string& label,
                    const std::function<OverworldMatchResult(const ImageRGB32&)>& match,
                    const std::function<void(ProControllerContext&)>& approach) {
                    // One watcher covers every blind step, locator correction and
                    // stationary wait. An early encounter cancels queued movement.
                    if (!watch_route204_trainer_event(stream,context,
                        [&](ProControllerContext& ctx) {
                            approach(ctx);
                            ctx.wait_for_all_requests();
                            align_route205_patrol_position(stream,ctx,match,label);
                            pbf_wait(ctx,45000ms);
                        },label)) OperationFailedExceptionWithScreenshot::fire(
                            ErrorReportMode::SEND_ERROR_REPORT,label+": Trainer did not approach the confirmed patrol position.",stream);
                    battle(label);
                    // Either trainer can interrupt the approach. After the fight,
                    // continue the route without returning to a patrol waypoint
                    // or replaying the canceled approach movement.
                };

                // Recorded path around the Windworks machines from the exact
                // reunion position at the end of 02.02. Keep directional turns.
                move(-1,0,1335ms,847ms);
                move(0,1,880ms,209ms);
                move(-1,0,906ms,471ms);
                move(0,-1,427ms,728ms);
                move(-1,0,1213ms,936ms);
                move(-1,0,991ms,175ms);
                move(0,-1,1714ms,456ms);
                move(1,-1,6809ms,1862ms);
                move(0,-1,922ms,554ms);
                move(-1,0,813ms,1370ms);
                move(0,-1,1862ms,145ms);
                move(-1,0,3478ms,500ms);
                // Replace the manual left/right/up sign adjustments with the
                // supplied repeatable position south of the bridge.
                align_to_route205_marker(stream,context,false);
                // Stop on the north bank, before Jacob's sight line. The longer
                // hold reached his patrol and interrupted the westward crossing.
                move(0,1,800ms,500ms);
                if (!activate_repel(stream,context,6)) OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,"Route 205: Could not activate Repel before the grass.",stream);

                // Cross beside the golden tree first, then travel through the
                // grass west of Jacob. Clamp north against the plateau cliff
                // before completing the westward approach to its south stairs.
                // Keep the recorded total westward travel, split around the turn.
                move(-1,0,600ms,268ms);
                align_to_route205_bank_tree(stream,context);
                move(0,1,2500ms,250ms);
                move(-1,0,2212ms,268ms);
                // Route 204's missed-fight pattern: one continuous approach,
                // then stand still and decide before sending the next movement.
                ShortDialogDetector hiker_dialogue(COLOR_YELLOW);
                BattleDialogDetector hiker_intro(COLOR_YELLOW);
                BattleMenuDetector hiker_menu(BattleType::TRAINER,COLOR_GREEN);
                BlackScreenDetector hiker_transition;
                const auto hiker_started=[&]() {
                    context.wait_for_all_requests();
                    const auto frame=stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Daniel north pass: No frame while checking for a dodged battle.",stream);
                    return hiker_dialogue.detect(*frame.frame) || hiker_intro.detect(*frame.frame) ||
                        hiker_menu.detect(*frame.frame) || hiker_transition.detect(*frame.frame);
                };
                ShortDialogWatcher hiker_dialogue_watch(COLOR_YELLOW);
                DetectorToFinder<BattleDialogDetector> hiker_intro_watch("Daniel battle intro",250ms,COLOR_YELLOW);
                BattleMenuWatcher hiker_menu_watch(BattleType::TRAINER,COLOR_GREEN);
                BlackScreenWatcher hiker_transition_watch;
                const bool hiker_encounter=hiker_started() || run_until<ProControllerContext>(stream,context,
                    [&](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx,{0,1},1200ms,0ms);
                        ctx.wait_for_all_requests();
                        stream.log("[Daniel dodge check] North movement stopped; observing without input.");
                        // Battle intro uses a long dialogue box before the menu.
                        // Keep one uninterrupted watcher through the approach and
                        // this stationary window; no wiggle and no A press.
                        pbf_wait(ctx,10000ms);
                    },{{hiker_dialogue_watch},{hiker_intro_watch},{hiker_menu_watch},{hiker_transition_watch}})>=0 || hiker_started();
                stream.log("[Daniel dodge check] missed_trainer="+std::to_string(!hiker_encounter));
                if (hiker_encounter) {
                    battle("Hiker Daniel");
                    stream.log("[Route 205] Daniel defeated; moving north from his battle landing to Zackary.");
                } else {
                    stream.log("[Route 205] Daniel dodged; continuing north up the stairs before turning right to Zackary.",COLOR_GREEN);
                }
                // Daniel stops us at his row when he engages. A skipped pass
                // has already carried us farther north. Resume from each pose
                // with its own remaining climb, then confirm the visual waypoint.
                // Stop at the bottom of the upper landing: the previous extra
                // 300 ms north put us in Zackary's walking lane before the turn.
                const Milliseconds north_to_zackary=hiker_encounter?1000ms:400ms;
                patrol_battle("Camper Zackary",match_route205_zackary,
                    [north_to_zackary](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx,{0,1},north_to_zackary,250ms);
                        // Clamp against the right cliff at the landing's bottom
                        // corner before visual confirmation and stationary wait.
                        pbf_move_left_joystick(ctx,{1,0},900ms,250ms);
                    });
                // Restore the west side for the recorded bridge/stair turns.
                move(-1,0,1800ms,250ms);
                patrol_battle("Battle Girl Kelsey",match_route205_kelsey,
                    [](ProControllerContext& ctx) {
                        const auto step=[&](double x,double y,Milliseconds duration,Milliseconds settle) {
                            pbf_move_left_joystick(ctx,{x,y},duration,0ms);
                            pbf_wait(ctx,settle);
                        };
                        step(0,1,638ms,673ms);
                        step(1,0,476ms,508ms);
                        step(0,1,717ms,254ms);
                        step(-1,0,415ms,430ms);
                        step(0,1,2517ms,57ms);
                        step(1,0,410ms,299ms);
                        step(0,1,1343ms,500ms);
                        step(-1,0,294ms,250ms);
                    });
                stream.log("[Route 205] Kelsey defeated; taking the left/north/right/north forest approach.");
                // Original recorded turns around the ledge and northern outcrop.
                // This exit applies even if Kelsey interrupted an earlier step.
                move(-1,0,415ms,430ms);
                // Renew in the cleared lane so expiry cannot interrupt the climb.
                // Keep both recorded northward movement durations unchanged.
                force_repel_expiry(stream,context,RepelRenewalChoice::YES,true);
                move(0,1,2517ms,57ms);
                move(1,0,410ms,299ms);
                move(0,1,1343ms,500ms);
                align_to_route205_marker(stream,context,true);
                move(0,1,20ms);
                stream.log("[Route 205] Forest entrance sign confirmed; checkpoint complete.",COLOR_GREEN);
            }
            // Cheryl controls her own Pokemon. Select Flamethrower visually,
            // then A advances target selection and battle text, as in the recording.
            void eterna_forest(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
                (void)env;
                // Second recorded run, beginning at the northern Route 205 sign.
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 1466ms, 0ms);
                pbf_wait(context, 1588ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 531ms, 0ms);
                pbf_wait(context, 3398ms);
                wait_for_dialogue(stream,context,"Cheryl introduction");
                // Reuse the Galactic plaza cutscene cleanup, including a second
                // pass after the animation gap before route movement resumes.
                clear_post_battle_dialogue(stream,context);
                pbf_wait(context,2000ms);
                clear_post_battle_dialogue(stream,context);
                auto encounter=[&](const std::string& label,const std::function<void(ProControllerContext&)>& approach) {
                    if (!watch_route204_trainer_event(stream,context,approach,label))
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            label+": No trainer encounter on the recorded approach.",stream);
                    stream.log("[Eterna Forest battle] "+label);
                    wait_for_trainer_battle_menu(stream,context);
                    // Ordinary trainers return after the confirmed battle-exit transition.
                    fight_trainer_using_moves(stream,context,2,label,false);
                };
                encounter("Lass Briana and Bug Catcher Jack",[&](ProControllerContext& context) {
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 579ms, 0ms);
                pbf_wait(context, 404ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 494ms, 0ms);
                pbf_wait(context, 358ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1011ms, 0ms);
                pbf_wait(context, 205ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 1980ms, 0ms);
                pbf_wait(context, 125ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1522ms, 0ms);
                pbf_wait(context, 38ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 1295ms, 0ms);
                pbf_wait(context, 1811ms);
                    pbf_wait(context,10000ms);
                });
                encounter("Psychics Lindsey and Elijah",[&](ProControllerContext& context) {
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 707ms, 0ms);
                pbf_wait(context, 437ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1211ms, 0ms);
                pbf_wait(context, 39ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 1440ms, 0ms);
                pbf_move_left_joystick(context, {1.000000, 1.000000}, 11ms, 0ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1426ms, 0ms);
                pbf_wait(context, 54ms);
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 1788ms, 0ms);
                pbf_wait(context, 3269ms);
                    pbf_wait(context,10000ms);
                });
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 2844ms, 0ms);
                pbf_move_left_joystick(context, {1.000000, -1.000000}, 12ms, 0ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1358ms, 0ms);
                pbf_wait(context, 223ms);
                // The recording renews here before walking into the last pair.
                force_repel_expiry(stream,context,RepelRenewalChoice::YES,true);
                encounter("Bug Catchers Donald and Phillip",[&](ProControllerContext& context) {
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 2335ms, 0ms);
                pbf_wait(context, 1960ms);
                    pbf_wait(context,10000ms);
                });
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 3878ms, 0ms);
                pbf_wait(context, 33ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 6451ms, 0ms);
                pbf_wait(context, 117ms);
                pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1727ms, 0ms);
                pbf_wait(context, 207ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 1693ms, 0ms);
                pbf_wait(context, 109ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1613ms, 0ms);
                pbf_wait(context, 538ms);
                // Clear the recorded expiry prompt before Cheryl's farewell.
                clear_post_battle_dialogue(stream,context,BUTTON_A);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 928ms, 0ms);
                pbf_wait(context, 1487ms);
                wait_for_dialogue(stream,context,"Cheryl farewell");
                // Reuse the Galactic plaza cutscene cleanup, including a second
                // pass after the animation gap before route movement resumes.
                clear_post_battle_dialogue(stream,context);
                pbf_wait(context,2000ms);
                clear_post_battle_dialogue(stream,context);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1238ms, 0ms);
                pbf_wait(context, 2157ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 510ms, 0ms);
                pbf_wait(context, 544ms);
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 864ms, 0ms);
                pbf_wait(context, 289ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1694ms, 0ms);
                pbf_wait(context, 383ms);
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 643ms, 0ms);
                pbf_wait(context, 346ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 844ms, 0ms);
                pbf_wait(context, 122ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 678ms, 0ms);
                pbf_wait(context, 151ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 4143ms, 0ms);
                context.wait_for_all_requests();
                stream.log("[Eterna Forest] East exit and Route 205 bridges complete; at Eterna City approach.",COLOR_GREEN);
            }
            void eterna_forest_exit_to_gym(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
                (void)env;
                // Clear Repel on the open street before locator corrections.
                // Reuse the helper's horizontal wiggles and decline renewal.
                force_repel_expiry(stream,context,RepelRenewalChoice::NO,false);
                // Reuse landmark alignment to clear the blue post east of the
                // door before entering. Healing starts only after map transition.
                align_to_overworld_landmarks(stream,context,match_eterna_center_entrance,
                    "Eterna Center doorway",false,true);
                BlackScreenOverWatcher center_entry(COLOR_RED,{0.1,0.1,0.8,0.8});
                int entry=run_until<ProControllerContext>(stream,context,
                    [](ProControllerContext& ctx) {
                        pbf_move_left_joystick(ctx,{0,1},4000ms,0ms);
                        pbf_wait(ctx,3000ms);
                    },{{center_entry}});
                if (entry!=0) OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,"Eterna Center doorway: Entry transition not confirmed.",stream);
                context.wait_for_all_requests();
                if (!heal_pokemon(stream,context,"Eterna")) OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,"Eterna Center exit was not confirmed.",stream);
                pbf_wait(context, 500ms);
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 500ms, 0ms);
                pbf_wait(context, 433ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 580ms, 0ms);
                pbf_wait(context, 537ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 144ms, 0ms);
                pbf_wait(context, 508ms);
                bool entered_house=false;
                align_to_overworld_landmarks(stream,context,match_eterna_underground_house,
                    "Underground Man house doorway",false,true,false,match_eterna_underground_man,&entered_house);
                BlackScreenOverWatcher house_entry(COLOR_RED,{0.1,0.1,0.8,0.8});
                if (!entered_house && run_until<ProControllerContext>(stream,context,
                    [](ProControllerContext& ctx){
                        pbf_move_left_joystick(ctx,{0,1},4000ms,0ms);
                        pbf_wait(ctx,3000ms);
                    },{{house_entry}})!=0) OperationFailedExceptionWithScreenshot::fire(
                        ErrorReportMode::SEND_ERROR_REPORT,"Underground Man house: Entry not confirmed.",stream);
                pbf_wait(context,500ms);
                // Retain the recorded northward approach from the fixed entrance.
                pbf_move_left_joystick(context,{0,1},290ms,0ms);
                align_to_overworld_landmarks(stream,context,match_eterna_underground_man,
                    "Underground Man interaction",false,true);
                pbf_move_left_joystick(context,{0,1},80ms,200ms);
                pbf_press_button(context,BUTTON_A,80ms,200ms);
                wait_for_dialogue(stream,context,"Underground Man introduction");
                // Recorded forced walk: 4.7s between house and street dialogue.
                // Keep watching for that gap plus a 500ms hardware buffer.
                clear_post_battle_dialogue(stream,context,BUTTON_A,5200ms,
                    "PokemonBDSP/Autostory/EternaUndergroundMan/ExplorerKitCard.png");
                // The first-use tutorial opens the Bag, uses the Kit, and takes
                // us underground automatically. The dialogue helper also clears
                // Roark's introduction. Do not reopen the Bag to enter again.
                const auto underground_frame=stream.video().snapshot();
                const char* underground_reference=underground_frame && underground_frame.frame->width()>=1600
                    ? "PokemonBDSP/Autostory/EternaUndergroundMan/UndergroundArrival.png"
                    : underground_frame && underground_frame.frame->width()>=1100
                        ? "PokemonBDSP/Autostory/EternaUndergroundMan/UndergroundArrivalMedium.png"
                        : "PokemonBDSP/Autostory/EternaUndergroundMan/UndergroundArrivalSmall.png";
                ImageMatchDetector underground_map(
                    std::make_shared<ImageRGB32>(RESOURCE_PATH()+underground_reference),
                    ImageFloatBox{.005,.110,.085,.130},30.0);
                if (!underground_frame || !underground_map.detect(*underground_frame.frame))
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Grand Underground tutorial: Underground arrival not confirmed.",stream);
                stream.log("[Grand Underground] Tutorial dialogue complete; returning with Y.");
                // User-requested full settling interval immediately before Y.
                pbf_wait(context,2000ms);
                pbf_press_button(context, BUTTON_Y, 129ms, 0ms);
                pbf_wait(context, 2076ms);
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 88ms, 0ms);
                pbf_wait(context, 819ms);
                pbf_press_button(context, BUTTON_A, 127ms, 0ms);
                pbf_wait(context, 1972ms);
                pbf_press_button(context, BUTTON_A, 127ms, 0ms);
                pbf_wait(context, 5330ms);
                // Keep the Y / Go up / Yes inputs above, then watch the
                // Underground Man's reward dialogue through the surface return.
                context.wait_for_all_requests();
                wait_for_dialogue(stream,context,"Underground Man return");
                clear_post_battle_dialogue(stream,context,BUTTON_A,3500ms);
                pbf_move_left_joystick(context, {0.000000, -1.000000}, 584ms, 0ms);
                pbf_wait(context, 85ms);
                pbf_move_left_joystick(context, {-1.000000, -0.000000}, 1448ms, 0ms);
                pbf_wait(context, 122ms);
                pbf_move_left_joystick(context, {0.000000, 1.000000}, 1455ms, 0ms);
                pbf_wait(context, 145ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000}, 1257ms, 0ms);
                pbf_wait(context, 956ms);
                // Reuse the plaza/Cheryl cutscene cleanup rather than a
                // recorded number of A presses during Cynthia's camera changes.
                context.wait_for_all_requests();
                wait_for_dialogue(stream,context,"Cynthia introduction");
                clear_post_battle_dialogue(stream,context,BUTTON_A,3500ms);
                // Let Cynthia's departure/camera finish before any route input,
                // then watch once more for delayed dialogue on slower hardware.
                pbf_wait(context,2000ms);
                context.wait_for_all_requests();
                clear_post_battle_dialogue(stream,context,BUTTON_A,1500ms);
                pbf_wait(context,500ms);
                stream.log("[Cynthia exit] Departure settled; moving west and south to reacquire the Center.");
                pbf_move_left_joystick(context,{-1,0},1200ms,100ms);
                pbf_move_left_joystick(context,{0,-1},1800ms,300ms);
                pbf_move_left_joystick(context, { 1,0 }, 1200ms, 100ms);
                pbf_move_left_joystick(context, { 0,-1 }, 250ms, 300ms);
                // Clear rewards before the approach can cross the gym doorway.
                context.wait_for_all_requests();
                if (!clear_new_item_flags(stream,context)) OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,"New reward flags could not be cleared before continuing.",stream);
                // Keep a loose horizontal street lane, but normalize the row:
                // the following fixed south leg requires a repeatable start.
                align_to_overworld_landmarks(stream,context,match_eterna_center_entrance,
                    "Eterna Center gym departure",false,true,true,nullptr,nullptr,.03,.02);
                // User's eternapctogym.txt with a shorter south leg; omit its final left127ms correction.
                // Start acquiring north of the old fence collision, then let
                // gym landmarks correct the final row and rightward approach.
                pbf_move_left_joystick(context,{0,-1},4300ms,0ms);
                pbf_wait(context,642ms);
                pbf_move_left_joystick(context,{1,0},1125ms,0ms);
                pbf_wait(context,939ms);
                pbf_move_left_joystick(context,{0,1},56ms,0ms);
                pbf_wait(context,467ms);
                pbf_move_left_joystick(context,{0,1},39ms,0ms);
                pbf_wait(context,1255ms);
                // Configurable only for a watched gym entry attempt. The normal
                // three-landmark exterior arrival gates remain unchanged.
                constexpr double GYM_MODERATE_ENTRY_CONFIDENCE=.88;
                constexpr size_t GYM_ENTRY_ATTEMPTS=3;
                bool gym_confirmed=false,transition_seen=false;
                for(size_t attempt=1;attempt<=GYM_ENTRY_ATTEMPTS && !gym_confirmed;attempt++){
                    stream.log("[Eterna Gym entry] Attempt "+std::to_string(attempt)+"/"+std::to_string(GYM_ENTRY_ATTEMPTS));
                    if(!transition_seen){
                        bool already_inside=false;
                        // Watch throughout alignment, not just the final north
                        // hold: a correction can cross the doorway first.
                        BlackScreenWatcher entering(COLOR_RED,{0.1,0.1,0.8,0.8});
                        const int outcome=run_until<ProControllerContext>(stream,context,
                            [&](ProControllerContext& ctx){
                                align_to_overworld_landmarks(stream,ctx,match_eterna_gym_entrance,
                                    "Eterna Gym doorway",false,true,false,match_eterna_gym_statues,&already_inside,
                                    0,0,GYM_MODERATE_ENTRY_CONFIDENCE);
                                if(!already_inside){
                                    pbf_move_left_joystick(ctx,{0,1},3000ms,0ms);
                                    pbf_wait(ctx,5000ms);
                                }
                            },{{entering}});
                        transition_seen=outcome==0 || already_inside;
                        stream.log("[Eterna Gym entry] Black transition detected="+std::to_string(outcome==0)+
                            ", interior seen during alignment="+std::to_string(already_inside));
                    }
                    if(transition_seen){
                        BlackScreenWatcher visible(COLOR_RED,{0.1,0.1,0.8,0.8},100,10,
                            BlackScreenWatcher::FinderType::GONE,500ms);
                        if(wait_until(stream,context,30s,{{visible}})!=0){
                            stream.log("[Eterna Gym entry] Transition has not finished; retrying within limit.");
                            continue;
                        }
                        stream.log("[Eterna Gym entry] Transition finished; verifying both statues.");
                    }
                    // Black alone never proves entry. Stop navigation and retry
                    // verification if a transition occurred into an unknown room.
                    size_t stable=0;
                    for(size_t check=0;check<8;check++){
                        context.wait_for_all_requests();
                        const auto frame=stream.video().snapshot();
                        if(!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Eterna Gym: No interior verification frame.",stream);
                        const auto interior=match_eterna_gym_statues(*frame.frame);
                        for(const auto& statue:interior.landmarks) stream.log("[Eterna Gym statues] "+statue.name+
                            " score="+std::to_string(statue.score)+", margin="+std::to_string(statue.margin)+
                            ", offset=("+std::to_string(statue.dx)+","+std::to_string(statue.dy)+")");
                        stable=interior.located ? stable+1 : 0;
                        stream.log("[Eterna Gym statues] Both in expected positions="+std::to_string(interior.located)+
                            ", stable frames="+std::to_string(stable)+"/3");
                        if(stable>=3){gym_confirmed=true;break;}
                        pbf_wait(context,250ms);
                    }
                    if(!gym_confirmed) stream.log("[Eterna Gym entry] Statues not verified; bounded retry.");
                }
                if(!gym_confirmed) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Eterna Gym: Both entrance statues were not verified after three attempts.",stream);
                stream.log("[Eterna City] Watched Explorer Kit/Cynthia events and verified gym entry complete.",COLOR_GREEN);
            }
            static void gym_Puzzle_02(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){

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

                // Reuse the Roark/Cynthia dialogue watcher so Gardenia's
                // introduction finishes before the doorway movement starts.
                pbf_press_button(context,BUTTON_A,150ms,300ms);
                context.wait_for_all_requests();
                wait_for_dialogue(stream,context,"Gardenia introduction");
                clear_post_battle_dialogue(stream,context,BUTTON_A,3500ms);
                stream.log("[Eterna Gym puzzle] Gardenia introduction dialogue cleared.");
                // Door transition and gym camera introduction take about 7.5s
                // in 2026-10-18 20-01-01; include 500ms for hardware variation.
                stream.log("[Eterna Gym puzzle] Entering Gardenia's doorway; waiting 8000ms for transition/cutscene (includes 500ms buffer).");
                pbf_press_dpad(context, DPAD_UP, 80ms, 8000ms);

                for (const auto& move : sequence1){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(context, move.dir, 80ms, 300ms);
                    }
                }

                //Button presses to initiate Trainer battle 01 (Lass Caroline).

                pbf_press_button(context,BUTTON_A,150ms,300ms);
                wait_for_trainer_battle_menu(stream,context);
                fight_trainer_using_moves(stream,context,2,"Lass Caroline (Flamethrower)",false);

                pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue.

                for (const auto& move : sequence2){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(context, move.dir, 80ms, 300ms);
                    }
                }

                //Button presses to initiate Trainer battle 02 (Aroma Lady Jenna).

                pbf_press_button(context,BUTTON_A,150ms,300ms);
                wait_for_trainer_battle_menu(stream,context);
                fight_trainer_using_moves(stream,context,2,"Aroma Lady Jenna (Flamethrower)",false);

                pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue.

                for (const auto& move : sequence3){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(context, move.dir, 80ms, 300ms);
                    }
                }

                //Button presses to initiate Trainer battle 03 (Aroma Lady Angela).

                pbf_press_button(context,BUTTON_A,150ms,300ms);
                wait_for_trainer_battle_menu(stream,context);
                fight_trainer_using_moves(stream,context,2,"Aroma Lady Angela (Flamethrower)",false);

                pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

                for (const auto& move : sequence4){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(context, move.dir, 80ms, 300ms);
                    }
                }

                //Button presses to initiate Trainer battle 04 (Beauty Lindsay).

                pbf_press_button(context,BUTTON_A,150ms,300ms);
                wait_for_trainer_battle_menu(stream,context);
                fight_trainer_using_moves(stream,context,2,"Beauty Lindsay (Flamethrower)",false);

                pbf_mash_button(context, BUTTON_B, 2500ms); //Battle exit dialogue

                for (const auto& move : sequence5){
                    for (int i = 0; i < move.count; i++){
                        pbf_press_dpad(context, move.dir, 80ms, 300ms);
                    }
                }

                //Button presses to initiate Gym battle.

                pbf_press_button(context,BUTTON_A,150ms,300ms);
                wait_for_trainer_battle_menu(stream,context);
                fight_trainer_using_moves(stream,context,2,"Gardenia (Flamethrower)",true);

                // The shared battle handler already advances reward dialogue
                // while confirming the overworld. Do not require the badge
                // animation again after it may have finished or be underway.
                // Continue the existing B watcher through remaining rewards;
                // its clear confirmation spans the observed ~5s badge gap.
                stream.log("[Eterna Gym puzzle] Shared battle cleanup returned; clearing remaining Gardenia rewards with B.");
                clear_post_battle_dialogue(stream,context,BUTTON_B,6000ms);
                stream.log("[Eterna Gym puzzle] Gardenia post-battle dialogue cleared; starting exit path.");

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
                AutoStory_checkpoint_0205().run_checkpoint(env, context, options, stats);
                AutoStory_checkpoint_0206().run_checkpoint(env, context, options, stats);

                context.wait_for_all_requests();
                env.console.log("End Segment " + name(), COLOR_GREEN);
            }
            std::string AutoStory_checkpoint_0201::name()       const { return "02.01 - Outside Gym 1"; }
            std::string AutoStory_checkpoint_0201::start_text() const { return "First Gym Badge acquired (Oreburgh City)."; }
            std::string AutoStory_checkpoint_0201::end_text()   const { return "Outside the central house in Floaroma Town immediately after exiting, facing south, with Repel inactive."; }

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
                checkpoint_reattempt_loop(env, context, options, stats,
                    [&](size_t /*attempt*/) {
                        roark_to_floaroma(env, env.console, context);

                    },

                    true
                );
            }
            std::string AutoStory_checkpoint_0202::name() const { return "02.02 - Floaroma Town"; }
            std::string AutoStory_checkpoint_0202::start_text() const {
                return "Outside the central house in Floaroma Town immediately after exiting, facing south, with Repel inactive.";
            }
            std::string AutoStory_checkpoint_0202::end_text() const {
                return "Standing in Valley Windworks after defeating Commander Mars.";
            }
            void AutoStory_checkpoint_0202::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                checkpoint_reattempt_loop(env, context, options, stats,
                    [&](size_t /*attempt*/) {
                        floaroma_to_mars(env, env.console, context);
                    }, true);
            }

            std::string AutoStory_checkpoint_0203::name()       const { return "02.03 - Route 205 to Eterna Forest"; }
            std::string AutoStory_checkpoint_0203::start_text() const { return "Standing in Valley Windworks after defeating Commander Mars."; }
            std::string AutoStory_checkpoint_0203::end_text()   const { return "On Route 205 outside Eterna Forest, west of the northern 205 sign, facing north with Repel active."; }

            void AutoStory_checkpoint_0203::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                checkpoint_0203(env, context, options, stats);
            }


            void checkpoint_0203(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options, stats,
                    [&](size_t /*attempt*/) {
                        mars_to_eterna_forest(env, env.console, context);

                    },

                    true
                );
            }

            std::string AutoStory_checkpoint_0204::name()       const{ return "02.04 - Eterna Forest"; }
            std::string AutoStory_checkpoint_0204::start_text() const{ return "On Route 205 outside Eterna Forest, west of the northern 205 sign, facing north with Repel active."; }
            std::string AutoStory_checkpoint_0204::end_text()   const{ return "At the Eterna City west approach after leaving Eterna Forest and crossing the Route 205 bridges, facing east."; }

            void AutoStory_checkpoint_0204::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const{
                checkpoint_0204(env, context, options, stats);
            }


            void checkpoint_0204(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ){
                checkpoint_reattempt_loop(env, context, options, stats,
                    [&](size_t /*attempt*/){
                        // TODO: implement Segment 02 gameplay logic
                        eterna_forest(env, env.console, context);

                    },

                    true
                );
            }

            std::string AutoStory_checkpoint_0205::name()       const { return "02.05 - Eterna City to Gym"; }
            std::string AutoStory_checkpoint_0205::start_text() const { return "At the Eterna City west approach after leaving Eterna Forest and crossing the Route 205 bridges, facing east."; }
            std::string AutoStory_checkpoint_0205::end_text()   const { return "Inside Eterna Gym at the entrance, facing north."; }

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
                checkpoint_reattempt_loop(env, context, options, stats,
                    [&](size_t /*attempt*/) {
                        // TODO: implement Segment 02 gameplay logic
                        eterna_forest_exit_to_gym(env, env.console, context);

                    },
                    true
                );
            }

            std::string AutoStory_checkpoint_0206::name()       const { return "02.06 - Inside Gym"; }
            std::string AutoStory_checkpoint_0206::start_text() const { return "Inside Eterna Gym at the entrance, facing north."; }
            std::string AutoStory_checkpoint_0206::end_text()   const { return "Outside Eterna Gym after defeating Gardenia and acquiring the Forest Badge."; }

            void AutoStory_checkpoint_0206::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                checkpoint_0206(env, context, options, stats);
            }


            void checkpoint_0206(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options, stats,
                    [&](size_t /*attempt*/) {

                        //#. Eterna Gym navigation and battle
                        gym_Puzzle_02(env, env.console, context);
                    },
                    true
                );
            }

        }
    }
}
