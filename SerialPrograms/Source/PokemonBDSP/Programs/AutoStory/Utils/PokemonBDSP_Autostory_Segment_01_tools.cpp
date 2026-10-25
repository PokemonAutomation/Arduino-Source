/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include <array>
#include <cmath>
#include <memory>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_Autostory_Segment_01_tools.h"
#include "PokemonBDSP_AutoStory_Segment_00_tools.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

// Extracted from the Segment 01 Oreburgh Gate approach. Both callers use
// the same bounded down/right search and cancel queued input at the black fade.
// Oreburgh preserves its original north sweep; Coronet uses only down/right.
void enter_cave_with_down_right_search(VideoStream& stream, ProControllerContext& context,
    const std::string& label, Milliseconds north_fallback) {
    context.wait_for_all_requests();
    for (size_t attempt=0;attempt<3;attempt++) {
        BlackScreenWatcher black_screen;
        const int result=run_until<ProControllerContext>(stream,context,
            [north_fallback](ProControllerContext& ctx) {
                for (size_t i=0;i<10;i++) {
                    pbf_press_button(ctx,BUTTON_DOWN,180ms,0ms);
                    pbf_press_button(ctx,BUTTON_RIGHT,180ms,0ms);
                    pbf_press_button(ctx,BUTTON_RIGHT,180ms,0ms);
                }
                if (north_fallback>0ms)
                    pbf_move_left_joystick(ctx,{0,1},north_fallback,0ms);
            },{{black_screen}});
        context.wait_for_all_requests();
        if (result==0) {
            stream.log(label+": Black screen detected.");
            return;
        }
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        label+": No black screen after 3 attempts.",stream);
}

            static bool mine_confirm_escape_via_menu(
                VideoStream& stream,
                ProControllerContext& context,
                BattleType battle_type
            ) {
                MenuWatcher game_menu;
                OverworldWatcher overworld;
                BattleMenuWatcher battle_menu(battle_type, COLOR_YELLOW);
                for (size_t attempt = 1; attempt <= 3; attempt++) {
                    // Do not accept the absence-based overworld detector before menu proof.
                    const int state = wait_until(stream, context, 300ms, {{game_menu}, {battle_menu}});
                    if (state == 1) {
                        stream.log("[Mine escape] Battle menu still present; retrying Run.");
                        return false;
                    }
                    int detected = state;
                    if (state != 0) {
                        stream.log("[Mine escape] Main menu fallback attempt " + std::to_string(attempt) + "/3.");
                        detected = run_until<ProControllerContext>(
                            stream, context,
                            [](ProControllerContext& controller) {
                                pbf_press_button(controller, BUTTON_X, 160ms, 840ms);
                                pbf_wait(controller, 3s);
                                controller.wait_for_all_requests();
                            },
                            {game_menu, battle_menu}
                        );
                        if (detected == 1) {
                            return false;
                        }
                    }
                    if (detected != 0) {
                        continue;
                    }
                    stream.log("[Mine escape] Main game menu confirmed; battle has ended.", COLOR_GREEN);
                    // Close the confirmed menu before returning control to navigation.
                    MenuDetector menu_visible;
                    for (size_t close = 0; close < 3; close++) {
                        pbf_press_button(context, BUTTON_B, 80ms, 500ms);
                        context.wait_for_all_requests();
                        if (wait_until(stream, context, 1s, {{overworld}}) == 0) {
                            return true;
                        }
                        BlackScreenWatcher visible(
                            COLOR_YELLOW, {0.1, 0.1, 0.8, 0.8}, 100, 10,
                            BlackScreenWatcher::FinderType::GONE, 500ms
                        );
                        if (wait_until(stream, context, 2s, {{visible}}) == 0) {
                            const VideoSnapshot frame = stream.video().snapshot();
                            if (frame && !menu_visible.detect(frame)) {
                                stream.log("[Mine escape] Confirmed game menu closed; returning to navigation.", COLOR_GREEN);
                                return true;
                            }
                        }
                    }
                    stream.log("[Mine escape] Main menu detected but could not confirm it closed.", COLOR_ORANGE);
                    return false;
                }
                return false;
            }

            static bool mine_select_run(
                VideoStream& stream,
                ProControllerContext& context,
                BattleType battle_type
            ) {
                BattleMenuDetector cursor(battle_type, COLOR_YELLOW);
                size_t matching_frames = 0;
                for (size_t check = 0; check < 16; check++) {
                    context.wait_for_all_requests();
                    const VideoSnapshot frame = stream.video().snapshot();
                    if (!frame) {
                        return false;
                    }
                    const BattleMenuOption selected = cursor.selected_option(frame);
                    stream.log("[Mine escape] Battle cursor row=" +
                        std::to_string(static_cast<int>(selected) + 1) + "; target Run=4.");
                    if (selected == BattleMenuOption::RUN) {
                        if (++matching_frames >= 3) {
                            stream.log("[Mine escape] Run cursor confirmed.", COLOR_GREEN);
                            return true;
                        }
                        pbf_wait(context, 100ms);
                    }
                    else {
                        matching_frames = 0;
                        if (selected == BattleMenuOption::UNKNOWN) {
                            // Do not send a direction or A without a known cursor.
                            pbf_wait(context, 150ms);
                        }
                        else {
                            const DpadPosition direction = selected == BattleMenuOption::BATTLE
                                ? DPAD_UP : DPAD_DOWN;
                            pbf_press_dpad(context, direction, 80ms, 250ms);
                        }
                    }
                }
                stream.log("[Mine escape] Could not confirm Run selection; A was not pressed.", COLOR_ORANGE);
                return false;
            }

            void escape_wild_battle(
                VideoStream& stream,
                ProControllerContext& context,
                BattleType battle_type,
                std::chrono::seconds timeout
            ) {
                EndBattleWatcher battle_end;
                bool exit_transition = false;
                const auto deadline = std::chrono::steady_clock::now() + timeout;
                while (std::chrono::steady_clock::now() < deadline) {
                    pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                    context.wait_for_all_requests();
                    BattleMenuWatcher menu_ready(battle_type, COLOR_YELLOW);
                    const int ready = wait_until(stream, context, 3s, {{battle_end}, {menu_ready}});
                    if (ready == 0) {
                        exit_transition = true;
                        break;
                    }
                    if (ready != 1) {
                        continue;
                    }
                    if (!mine_select_run(stream, context, battle_type)) {
                        continue;
                    }
                    stream.log("[Mine escape] Run input sent; watching for black-screen exit.");
                    const int result = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& controller) {
                            pbf_press_button(controller, BUTTON_A, 60ms, 0ms);
                            pbf_wait(controller, 4s);
                            controller.wait_for_all_requests();
                        },
                        {battle_end}
                    );
                    if (result == 0) {
                        exit_transition = true;
                        break;
                    }
                }
                if (!exit_transition && wait_until(stream, context, 10s, {{battle_end}}) == 0) {
                    exit_transition = true;
                }
                if (exit_transition) {
                    // EndBattleWatcher requires black followed by a visible screen.
                    stream.log("[Mine escape] Black-screen exit transition confirmed.", COLOR_GREEN);
                    pbf_wait(context, 800ms);
                    context.wait_for_all_requests();
                    return;
                }
                // Only probe X after normal escape attempts and the final transition wait.
                // Absence of battle UI by itself is not evidence of an overworld return.
                stream.log("[Mine escape] Exit transition missed; trying late main-menu fallback.");
                if (mine_confirm_escape_via_menu(stream, context, battle_type)) {
                    return;
                }
                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "escape_wild_battle: Exit transition and late main-menu fallback could not confirm escape.",
                    stream
                );
            }

            void Barry_1_fight(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
                stream.log("Starting Barry 1 battle...");

                // State machine: loop until EndBattleWatcher fires
                for (size_t c = 0; c < 20; c++) {
                    context.wait_for_all_requests();

                    BattleMenuWatcher battle_menu(BattleType::STANDARD);
                    EndBattleWatcher end_battle;
                    int ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& context) {
                            pbf_mash_button(context, BUTTON_ZL, 120000ms);
                        },
                {
                    battle_menu,
                    end_battle,
                }
                );
                    switch (ret) {
                    case 0:
                        stream.log("Battle menu detected - using first move.");
                        pbf_press_button(context, BUTTON_ZL, 160ms, 1000ms);
                        break;
                    case 1:
                        stream.log("Barry 1 battle complete.");
                        pbf_mash_button(context, BUTTON_B, 5000ms);
                        context.wait_for_all_requests();
                        return;
                    default:
                        OperationFailedExceptionWithScreenshot::fire(
                            ErrorReportMode::SEND_ERROR_REPORT,
                            "Barry_1_fight: Timed out waiting for battle menu or end.",
                            stream
                        );
                    }
                }

                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "Barry_1_fight: Battle did not end after 20 turns.",
                    stream
                );
            }

        }
    }
}
