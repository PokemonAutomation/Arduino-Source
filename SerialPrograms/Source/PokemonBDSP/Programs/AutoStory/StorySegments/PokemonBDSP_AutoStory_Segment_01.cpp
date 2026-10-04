/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <cmath>
#include <memory>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_01.h"
#include "PokemonBDSP_AutoStory_Segment_00_tools.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {
            enum class MoveDirection { UP, DOWN, LEFT, RIGHT };

            //  Left joystick vector for a direction (matches the game's joystick convention used in this file).
            static std::pair<double, double> joystick_for(MoveDirection direction) {
                switch (direction) {
                case MoveDirection::UP:    return { 0.0, 1.0 };
                case MoveDirection::DOWN:  return { 0.0, -1.0 };
                case MoveDirection::LEFT:  return { -1.0, 0.0 };
                case MoveDirection::RIGHT: return { 1.0, 0.0 };
                }
                return { 0.0, 0.0 };
            }

            //  Runs from a wild battle that is already in progress (menu visible or about to be).
            //  Throws OperationFailedException if it cannot escape within `timeout`.
            static void escape_wild_battle(
                VideoStream& stream,
                ProControllerContext& context,
                BattleType battle_type = BattleType::STANDARD,
                std::chrono::seconds timeout = 30s
            ) {
                bool escaped = false;
                auto deadline = std::chrono::steady_clock::now() + timeout;
                while (std::chrono::steady_clock::now() < deadline) {
                    pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                    context.wait_for_all_requests();

                    BattleMenuWatcher menu_ready(battle_type, COLOR_YELLOW);
                    if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                        continue;
                    }

                    pbf_move_left_joystick(context, {0, -1}, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                    context.wait_for_all_requests();

                    EndBattleWatcher battle_end;
                    BattleMenuWatcher back_to_menu(battle_type, COLOR_YELLOW);
                    if (wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}}) == 0) {
                        escaped = true;
                        break;
                    }
                }
                if (!escaped) {
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "escape_wild_battle: Failed to run from battle.",
                        stream
                    );
                }
                OverworldWatcher overworld;
                wait_until(stream, context, 5s, {{overworld}});
            }

            //  Watches for a short dialogue box over several attempts. Returns true as soon as one is seen.
            //  Returns false (without throwing) only if every attempt times out, so callers can skip
            //  an optional interaction. Multiple attempts guard against a single missed detection.
            static bool try_detect_dialogue(
                VideoStream& stream,
                ProControllerContext& context,
                const std::string& label,
                size_t attempts = 3,
                std::chrono::milliseconds window = 5000ms
            ) {
                for (size_t attempt = 0; attempt < attempts; attempt++) {
                    ShortDialogWatcher watcher_cyan(COLOR_CYAN);
                    ShortDialogWatcher watcher_red(COLOR_RED);
                    if (wait_until(stream, context, window, {{watcher_cyan}, {watcher_red}}) >= 0) {
                        stream.log(label + " dialogue detected.");
                        return true;
                    }
                    stream.log(
                        label + " dialogue not detected (attempt " + std::to_string(attempt + 1) + "/" + std::to_string(attempts) + ").",
                        COLOR_ORANGE
                    );
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
            static std::chrono::milliseconds walk_until_doorway(
                VideoStream& stream,
                ProControllerContext& context,
                MoveDirection direction,
                std::chrono::milliseconds travel_time,
                const DoorwayWalkOptions& options = DoorwayWalkOptions()
            ) {
                enum class NavState { WALKING, BLACK_SCREEN, IN_BATTLE, DONE };

                const auto [x, y] = joystick_for(direction);
                const std::chrono::milliseconds max_travel = travel_time + options.grace;

                std::chrono::milliseconds walked = 0ms;
                size_t battles = 0;
                NavState state = NavState::WALKING;

                while (state != NavState::DONE) {
                    switch (state) {
                    case NavState::WALKING: {
                        if (walked >= max_travel) {
                            OperationFailedException::fire(
                                ErrorReport::SEND_ERROR_REPORT,
                                "walk_until_doorway: Doorway not found within time budget.",
                                stream
                            );
                        }
                        context.wait_for_all_requests();
                        pbf_move_left_joystick(context, {x, y}, options.step, 0ms);
                        context.wait_for_all_requests();
                        walked += options.step;

                        BlackScreenWatcher black_screen;
                        WhiteScreenWatcher white_screen;
                        if (options.detect_white_screen) {
                            int ret = wait_until(stream, context, options.black_screen_window, {{black_screen}, {white_screen}});
                            if (ret == 0) {
                                state = NavState::BLACK_SCREEN;
                            } else if (ret == 1) {
                                stream.log("[AutoStory] White screen detected.", COLOR_ORANGE);
                                state = NavState::DONE;
                            }
                        } else if (wait_until(stream, context, options.black_screen_window, {{black_screen}}) == 0) {
                            state = NavState::BLACK_SCREEN;
                        }
                        break;
                    }
                    case NavState::BLACK_SCREEN: {
                        stream.log("[AutoStory] Black screen detected. Checking for battle menu.", COLOR_ORANGE);
                        BattleMenuWatcher battle_menu(options.battle_type, COLOR_YELLOW);
                        if (wait_until(stream, context, options.battle_menu_window, {{battle_menu}}) == 0) {
                            state = NavState::IN_BATTLE;
                        } else {
                            state = NavState::DONE;
                        }
                        break;
                    }
                    case NavState::IN_BATTLE: {
                        if (++battles > options.max_battles) {
                            OperationFailedException::fire(
                                ErrorReport::SEND_ERROR_REPORT,
                                "walk_until_doorway: Too many battles while walking.",
                                stream
                            );
                        }
                        stream.log("[AutoStory] Wild battle. Escaping. Walked " + std::to_string(walked.count()) + "ms of " + std::to_string(travel_time.count()) + "ms.", COLOR_ORANGE);
                        escape_wild_battle(stream, context, options.battle_type);
                        state = NavState::WALKING;
                        break;
                    }
                    case NavState::DONE:
                        break;
                    }
                }
                stream.log("[AutoStory] Doorway transition detected after " + std::to_string(walked.count()) + "ms of walking.", COLOR_GREEN);
                return walked;
            }

            static void Barry_1_fight(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
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
                        OperationFailedException::fire(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Barry_1_fight: Timed out waiting for battle menu or end.",
                            stream
                        );
                    }
                }

                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Barry_1_fight: Battle did not end after 20 turns.",
                    stream
                );
            }

            static void handle_unexpected_trainer_battle(
                SingleSwitchProgramEnvironment& env,
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log("Unexpected trainer dialogue detected.", COLOR_ORANGE);

                // Clear the white dialogue boxes before entering battle logic.
                pbf_mash_button(context, BUTTON_B, 5000ms);
                context.wait_for_all_requests();

                Barry_1_fight(env, stream, context);

                ShortDialogWatcher post_battle_dialog_cyan(COLOR_CYAN);
                ShortDialogWatcher post_battle_dialog_red(COLOR_RED);
                while (true) {
                    int dialog_ret = wait_until(
                        stream,
                        context,
                        3s,
                        {
                            {post_battle_dialog_cyan},
                            {post_battle_dialog_red},
                        }
                    );
                    if (dialog_ret < 0) {
                        break;
                    }
                    pbf_mash_button(context, BUTTON_B, 1200ms);
                    context.wait_for_all_requests();
                }

                pbf_move_left_joystick(context, {0, -1}, 1000ms, 0ms);
                walk_through_grass(stream, context, 1, 0, 10, "right through grass");
            }

            static void walk_through_grass_with_trainer_watch(
                SingleSwitchProgramEnvironment& env,
                VideoStream& stream,
                ProControllerContext& context,
                int8_t x,
                int8_t y,
                size_t steps,
                const std::string& direction_name,
                BattleType battle_type = BattleType::STANDARD
            ) {
                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(battle_type, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                for (size_t step = 0; step < steps; step++) {
                    context.wait_for_all_requests();

                    pbf_move_left_joystick(context, {(double)x, (double)y}, 200ms, 0ms);

                    context.wait_for_all_requests();

                    ShortDialogWatcher trainer_dialog_cyan(COLOR_CYAN);
                    ShortDialogWatcher trainer_dialog_red(COLOR_RED);
                    int battle_ret = wait_until(
                        stream,
                        context,
                        4500ms,
                        {
                            {trainer_dialog_cyan},
                            {trainer_dialog_red},
                            {battle_start},
                            {battle_menu},
                            {battle_dialog},
                        }
                    );

                    if (battle_ret == 0 || battle_ret == 1) {
                        handle_unexpected_trainer_battle(env, stream, context);
                        return;
                    }

                    if (battle_ret >= 2) {
                        stream.log("[AutoStory] Wild battle detected during movement.", COLOR_ORANGE);

                        if (battle_ret == 2) {
                            BattleMenuWatcher wait_for_menu(battle_type, COLOR_YELLOW);
                            wait_until(stream, context, 10s, {{wait_for_menu}});
                            context.wait_for_all_requests();
                        }

                        bool escaped = false;
                        size_t overworld_streak = 0;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {
                            pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                            context.wait_for_all_requests();

                            BattleMenuWatcher menu_ready(battle_type, COLOR_YELLOW);
                            if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                                // No battle menu for 3s. If the overworld is showing on
                                // two consecutive checks, the battle is already over.
                                OverworldWatcher overworld_check;
                                if (wait_until(stream, context, 1s, {{overworld_check}}) == 0) {
                                    overworld_streak++;
                                } else {
                                    overworld_streak = 0;
                                }
                                if (overworld_streak >= 2) {
                                    stream.log("[AutoStory] Overworld detected without battle menu. Battle already over.", COLOR_ORANGE);
                                    escaped = true;
                                    break;
                                }
                                continue;
                            }
                            overworld_streak = 0;

                            // Cursor direction can drift; try both vertical directions.
                            pbf_move_left_joystick(context, {0, 1}, 80ms, 80ms);
                            pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            BattleMenuWatcher back_to_menu(battle_type, COLOR_YELLOW);
                            int run_result = wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}});
                            if (run_result == 0) {
                                escaped = true;
                                break;
                            }

                            if (run_result == 1) {
                                pbf_move_left_joystick(context, {0, -1}, 80ms, 80ms);
                                pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                                context.wait_for_all_requests();

                                EndBattleWatcher battle_end_alt;
                                BattleMenuWatcher back_to_menu_alt(battle_type, COLOR_YELLOW);
                                int run_result_alt = wait_until(stream, context, 4s, {{battle_end_alt}, {back_to_menu_alt}});
                                if (run_result_alt == 0) {
                                    escaped = true;
                                    break;
                                }
                            }
                        }

                        if (!escaped) {
                            OperationFailedException::fire(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to run from wild battle while moving " + direction_name + ".",
                                stream
                            );
                        }

                        OverworldWatcher overworld;
                        wait_until(stream, context, 5s, {{overworld}});
                    }
                }
            }

            // ----------------------------------------------------------------
            //  Segment 1 Part 1: Jubilife City to Oreburgh Gate
            //  Navigates the player from Jubilife City to the Oreburgh Gate, including the
            //  first Barry fight.
            // ---------------------------------------------------------------------------
            static void jubilife_to_oreburgh_gate(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {
                // Wait for game to fully accept input after save/overworld return
                pbf_wait(context, 2000ms);
                context.wait_for_all_requests();

                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1465ms, 0ms);
                pbf_wait(context, 127ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 644ms, 0ms);
                pbf_wait(context, 698ms);
                leave_pokemon_center_before_barry_1(stream, context);
                pbf_wait(context, 2340ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 742ms, 0ms);
                pbf_wait(context, 348ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2694ms, 0ms);
                pbf_wait(context, 482ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 3810ms, 0ms);

                wait_for_dialogue(stream, context, "Phase 1 (Barry 1 fight dialog)");
                pbf_mash_button(context, BUTTON_B, 5000ms);
                Barry_1_fight(env, stream, context);
                pbf_mash_button(context, BUTTON_B, 5000ms);
                context.wait_for_all_requests();
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 2761ms, 0ms);
                pbf_wait(context, 1647ms);
                walk_through_grass(stream, context, 1, 0, 5, "right through grass");
                wait_for_dialogue(stream, context, "Dallas fight dialog)");
                pbf_mash_button(context, BUTTON_B, 5000ms);
				Barry_1_fight(env, stream, context);//fighting Dallas
                context.wait_for_all_requests();
                walk_through_grass(stream, context, 1, 0, 5, "right through grass");
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 600ms, 0ms);
                walk_through_grass(stream, context, 1, 0, 10, "right through grass");
                //there is a small chance that the player dodges sSebastian, so this 
                //function addresses that edge case
                if (try_detect_dialogue(stream, context, "Sebastian fight")) {
                    pbf_mash_button(context, BUTTON_B, 5000ms);
                    Barry_1_fight(env, stream, context);//fighting Sebastian
                    context.wait_for_all_requests();
                } else {
                    stream.log("Sebastian dialogue never appeared. Skipping fight.", COLOR_ORANGE);
                }
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 2000ms, 0ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 8000ms, 0ms);
                pbf_wait(context, 200ms);
                context.wait_for_all_requests();
                bool black_screen_found = false;
                for (size_t attempt = 0; attempt < 3 && !black_screen_found; attempt++) {
                    BlackScreenWatcher black_screen;
                    int ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& context) {
                            for (size_t i = 0; i < 10; i++) {
                                pbf_press_button(context, BUTTON_DOWN, 180ms, 0ms);
                                pbf_press_button(context, BUTTON_RIGHT, 180ms, 0ms);
                                pbf_press_button(context, BUTTON_RIGHT, 180ms, 0ms);
                            }
                            pbf_move_left_joystick(context, { 0, 1 }, 10000ms, 0ms);
                        },
                        { black_screen }
                    );
                    if (ret == 0) {
                        stream.log("Black screen detected.");
                        black_screen_found = true;
                    }
                }
                context.wait_for_all_requests();

                if (!black_screen_found) {
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "jubilife_to_oreburgh_gate: No black screen after 3 attempts.",
                        stream
                    );
                }
            }
            static void oreburgh_gate_nav(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {
                pbf_wait(context, 2000ms);
                pbf_move_left_joystick(context, {1.000000, -0.000000 }, 879ms, 0ms);
                wait_for_dialogue(stream, context, "HM Rock Smash Dialog");
                pbf_mash_button(context, BUTTON_B, 15000ms);
                walk_through_grass(stream, context, 1, 0, 10, "right through gate");
                //Similar to sebastian, an edge case exists where we run into too many
                //wild battles and don't reach curtis or diana. This detect dialog function solves that.
                while (!try_detect_dialogue(stream, context, "Curtis fight", 1, 3000ms)) {
                    walk_through_grass(stream, context, 1, 0, 1, "right through gate");
                }
                pbf_mash_button(context, BUTTON_B, 5000ms);
                Barry_1_fight(env, stream, context);//fighting Curtis
                walk_through_grass(stream, context, 1, 0, 10, "right through gate");
                while (!try_detect_dialogue(stream, context, "Diana fight", 1, 3000ms)) {
                    walk_through_grass(stream, context, 1, 0, 1, "right through gate");
                }
                pbf_mash_button(context, BUTTON_B, 5000ms);
                Barry_1_fight(env, stream, context);//fighting Diana
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
            static void oreburgh_gate_to_mine(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {
                pbf_wait(context, 2000ms);
                context.wait_for_all_requests();
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1056ms, 0ms);
                while (!try_detect_dialogue(stream, context, "Phase 2 (oreburgh tour)", 1, 3000ms)) {
                    pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1056ms, 0ms);
                }
                pbf_mash_button(context, BUTTON_B, 12000ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 319ms, 0ms);
                pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 332ms, 0ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 3124ms, 0ms);
                pbf_wait(context, 23ms);
                bool black_screen_found = false;
                for (size_t attempt = 0; attempt < 3 && !black_screen_found; attempt++) {
                    BlackScreenWatcher black_screen;
                    int ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& context) {
                            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 4740ms, 0ms);
                        },
                        { black_screen }
                    );
                    if (ret == 0) {
                        stream.log("Black screen detected.");
                        black_screen_found = true;
                    }
                }
                context.wait_for_all_requests();

                if (!black_screen_found) {
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "oreburgh_gate_to_mine: No black screen after 3 attempts.",
                        stream
                    );
                }

                //  Let the mine entrance transition fully finish so the next step's black screen
                //  watcher can't pick up the tail of this one.
                BlackScreenOverWatcher black_over;
                wait_until(stream, context, 6000ms, {{black_over}});
                context.wait_for_all_requests();
                pbf_wait(context, 2000ms);
                context.wait_for_all_requests();
            }
            // Aligns the player between the two rocks using template matching in fixed ROIs,
            // then faces up and presses A to talk to the NPC.
            static void align_between_rocks_and_talk(
                VideoStream& stream, ProControllerContext& context) {
                auto rock1_image = std::make_shared<const ImageRGB32>(RESOURCE_PATH() + "PokemonBDSP/rock_smash_rock1.png");
                auto rock2_image = std::make_shared<const ImageRGB32>(RESOURCE_PATH() + "PokemonBDSP/rock_smash_rock2.png");
                auto helmet_image = std::make_shared<const ImageRGB32>(RESOURCE_PATH() + "PokemonBDSP/roark_helmet.png");
                const ImageFloatBox rock1_box(0.390, 0.370, 0.075, 0.130);
                const ImageFloatBox rock2_box(0.529, 0.370, 0.075, 0.130);
                const ImageFloatBox helmet_box(0.474, 0.32, 0.035, 0.065);
                const double max_rmsd = 60;
                ImageMatchDetector rock1(rock1_image, rock1_box, max_rmsd, true);
                ImageMatchDetector rock2(rock2_image, rock2_box, max_rmsd, true);
                ImageMatchDetector helmet(helmet_image, helmet_box, max_rmsd, true);

                auto score = [&]() {
                    context.wait_for_all_requests();
                    VideoSnapshot frame = stream.video().snapshot();
                    return std::array<double, 3>{rock1.rmsd(frame), rock2.rmsd(frame), helmet.rmsd(frame)};
                };
                auto total = [](const std::array<double, 3>& s) { return s[0] + s[1] + s[2]; };

                const size_t MAX_ATTEMPTS = 12;
                int8_t dir_x = 1;
                bool aligned = false;
                auto prev = score();
                for (size_t attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
                    if (prev[0] <= max_rmsd && prev[1] <= max_rmsd && prev[2] <= max_rmsd) {
                        aligned = true;
                        break;
                    }
                    walk_through_grass(
                        stream, context, dir_x, 0, 1,
                        dir_x > 0 ? "right (rock align)" : "left (rock align)"
                    );
                    pbf_wait(context, 300ms);
                    auto cur = score();
                    if (total(cur) > total(prev)) {
                        dir_x = (int8_t)-dir_x;
                    }
                    prev = cur;
                }
                if (!aligned) {
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to align between the two rocks and Roark's helmet.",
                        stream
                    );
                }

                // Face north (up) without moving off the spot: a tap turns the character.
                pbf_move_left_joystick(context, { 0.0, 1.0 }, 100ms, 300ms);
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
                pbf_mash_button(context, BUTTON_A, 4000ms);
                pbf_wait(context, 4873ms);
            }

            // Diagnostic only: identifies the entrance start without moving the player.
            // Thresholds are initial values; use the logged scores to calibrate on live video.
            enum class MineStartPosition {
                UNKNOWN,
                LEFT,
                CENTER,
                RIGHT
            };

            static MineStartPosition detect_mine_start(
                VideoStream& stream, ProControllerContext& context
            ) {
                const std::array<const char*, 3> names = {"Left", "Center", "Right"};
                const std::array<const char*, 3> files = {
                    "mine_start_left.png", "mine_start_center.png", "mine_start_right.png"
                };
                // Wooden beam and side walls: exclude the animated player and shadow.
                const std::array<ImageFloatBox, 3> boxes = {
                    ImageFloatBox(0.24, 0.24, 0.52, 0.12),
                    ImageFloatBox(0.18, 0.37, 0.20, 0.23),
                    ImageFloatBox(0.64, 0.37, 0.18, 0.23)
                };
                std::array<std::array<std::unique_ptr<ImageMatchDetector>, 3>, 3> detectors;
                for (size_t candidate = 0; candidate < names.size(); candidate++) {
                    auto reference = std::make_shared<const ImageRGB32>(
                        RESOURCE_PATH() + "PokemonBDSP/Autostory/Oreburg_mine_nav/" + files[candidate]
                    );
                    for (size_t region = 0; region < boxes.size(); region++) {
                        detectors[candidate][region] = std::make_unique<ImageMatchDetector>(
                            reference, boxes[region], 60.0, true
                        );
                    }
                }

                context.wait_for_all_requests();
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();
                int previous = -1;
                size_t streak = 0;
                for (size_t attempt = 0; attempt < 12; attempt++) {
                    const VideoSnapshot frame = stream.video().snapshot();
                    std::array<double, 3> scores = {};
                    std::array<bool, 3> regions_ok = {true, true, true};
                    for (size_t candidate = 0; candidate < names.size(); candidate++) {
                        for (size_t region = 0; region < boxes.size(); region++) {
                            const double score = detectors[candidate][region]->rmsd(frame);
                            scores[candidate] += score / boxes.size();
                            regions_ok[candidate] = regions_ok[candidate] && score <= 60.0;
                        }
                    }
                    size_t best = 0;
                    for (size_t candidate = 1; candidate < names.size(); candidate++) {
                        if (scores[candidate] < scores[best]) best = candidate;
                    }
                    double second = 10000.0;
                    for (size_t candidate = 0; candidate < names.size(); candidate++) {
                        if (candidate != best && scores[candidate] < second) second = scores[candidate];
                    }
                    stream.log("[Mine start] Scores (lower is better): Left=" +
                        std::to_string(scores[0]) + ", Center=" + std::to_string(scores[1]) +
                        ", Right=" + std::to_string(scores[2]));

                    const int detected = regions_ok[best] && scores[best] <= 40.0 &&
                        second - scores[best] >= 8.0 ? static_cast<int>(best) : -1;
                    if (detected >= 0) {
                        streak = detected == previous ? streak + 1 : 1;
                        if (streak >= 3) {
                            stream.log(
                                std::string("[Mine start] Detected: ") + names[best],
                                COLOR_GREEN
                            );

                            switch (best) {
                            case 0: return MineStartPosition::LEFT;
                            case 1: return MineStartPosition::CENTER;
                            case 2: return MineStartPosition::RIGHT;
                            }

                            return MineStartPosition::UNKNOWN;
                        }
                    } else {
                        streak = 0;
                    }
                    previous = detected;
                    pbf_wait(context, 250ms);
                    context.wait_for_all_requests();
                }
                stream.log("[Mine start] Detected: Unsure.", COLOR_ORANGE);
                return MineStartPosition::UNKNOWN;
            }
            // Keeps the original picture-only test available.
            static void oreburgh_mine_localization_test(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                detect_mine_start(stream, context);
            }
            struct MineCenterOffset {
                bool valid;
                double offset;
                double score;
            };

            // Finds the sideways shift of stationary scenery relative to the center picture.
            // Positive offset: scenery shifted right, so the player is left of center.
            static MineCenterOffset measure_mine_center_offset(
                const ImageViewRGB32& reference,
                const ImageViewRGB32& live
            ) {
                if (!reference || !live) {
                    return { false, 0.0, 1000.0 };
                }

                // Use a fixed working size so the search works at different capture resolutions.
                const ImageRGB32 reference_scaled = reference.scale_to(960, 540);
                const ImageRGB32 live_scaled = live.scale_to(960, 540);

                // Entrance beam and both posts. Avoid the player and shadow.
                const std::array<ImageFloatBox, 3> boxes = {
                    ImageFloatBox(0.35, 0.285, 0.30, 0.07),
                    ImageFloatBox(0.355, 0.37, 0.035, 0.23),
                    ImageFloatBox(0.615, 0.37, 0.035, 0.23)
                };

                std::array<double, 193> scores = {};
                std::array<double, 193> worst_regions = {};

                int best_shift = 0;
                double best_score = 1000.0;

                // Search up to 10% of the screen width in either direction.
                for (int shift = -96; shift <= 96; shift++) {
                    double total = 0.0;
                    double worst = 0.0;

                    for (const ImageFloatBox& box : boxes) {
                        const ImageViewRGB32 reference_crop =
                            extract_box_reference(reference_scaled, box);

                        // Translate the same crop horizontally in the live image.
                        const ImageViewRGB32 live_crop =
                            extract_box_reference(live_scaled, box, shift, 0);

                        const double score =
                            ImageMatch::pixel_RMSD(reference_crop, live_crop);

                        total += score;
                        if (score > worst) {
                            worst = score;
                        }
                    }

                    const double average = total / boxes.size();
                    const size_t index = static_cast<size_t>(shift + 96);

                    scores[index] = average;
                    worst_regions[index] = worst;

                    if (average < best_score) {
                        best_score = average;
                        best_shift = shift;
                    }
                }

                // Reject another similarly good match well away from the chosen position.
                double alternative_score = 1000.0;

                for (int shift = -96; shift <= 96; shift++) {
                    if (std::abs(shift - best_shift) <= 12) {
                        continue;
                    }

                    const double score = scores[static_cast<size_t>(shift + 96)];
                    if (score < alternative_score) {
                        alternative_score = score;
                    }
                }

                const size_t best_index = static_cast<size_t>(best_shift + 96);

                // Initial limits: live testing will tell us whether these need adjustment.
                const bool valid =
                    best_score <= 20.0 &&
                    worst_regions[best_index] <= 30.0 &&
                    alternative_score - best_score >= 3.0 &&
                    std::abs(best_shift) < 96;

                return {
                    valid,
                    static_cast<double>(best_shift) / 960.0,
                    best_score
                };
            }
            // Test only: align with the entrance center, then stop.
            static bool oreburgh_mine_alignment_test(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                const ImageRGB32 reference(
                    RESOURCE_PATH() +
                    "PokemonBDSP/Autostory/Oreburg_mine_nav/mine_start_center.png"
                );

                constexpr size_t MAX_MOVEMENTS = 12;
                constexpr double CENTER_TOLERANCE = 0.008;

                size_t movements = 0;
                size_t center_streak = 0;

                context.wait_for_all_requests();
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();

                while (true) {
                    const VideoSnapshot frame = stream.video().snapshot();

                    const MineCenterOffset measurement =
                        measure_mine_center_offset(reference, frame);

                    stream.log(
                        "[Mine align] Offset=" +
                        std::to_string(measurement.offset) +
                        ", score=" + std::to_string(measurement.score) +
                        (measurement.valid ? ", trusted" : ", unsure")
                    );

                    if (!measurement.valid) {
                        stream.log(
                            "[Mine align] Entrance position unsure. Stopping.",
                            COLOR_ORANGE
                        );
                        return false;
                    }

                    if (std::abs(measurement.offset) <= CENTER_TOLERANCE) {
                        center_streak++;

                        if (center_streak >= 3) {
                            stream.log(
                                "[Mine align] Center reached. Test complete.",
                                COLOR_GREEN
                            );
                            return true;
                        }

                        pbf_wait(context, 250ms);
                        context.wait_for_all_requests();
                        continue;
                    }

                    center_streak = 0;

                    // Check the latest position before enforcing the movement limit.
                    if (movements >= MAX_MOVEMENTS) {
                        stream.log(
                            "[Mine align] Movement limit reached. Stopping.",
                            COLOR_ORANGE
                        );
                        return false;
                    }

                    const double x = measurement.offset > 0.0 ? 1.0 : -1.0;

                    // Smaller movements when close to the center.
                    const auto duration =
                        std::abs(measurement.offset) < 0.025 ? 40ms : 80ms;

                    stream.log(
                        x > 0.0
                        ? "[Mine align] Moving a little right."
                        : "[Mine align] Moving a little left."
                    );

                    BlackScreenWatcher black;
                    BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
                    BattleDialogWatcher dialog(COLOR_YELLOW);

                    const int result = run_until<ProControllerContext>(
                        stream,
                        context,
                        [x, duration](ProControllerContext& ctx) {
                            pbf_move_left_joystick(
                                ctx, { x, 0.0 }, duration, 0ms
                            );
                            pbf_wait(ctx, 500ms);
                            ctx.wait_for_all_requests();
                        },
                        { black, menu, dialog }
                    );

                    if (result >= 0) {
                        stream.log(
                            "[Mine align] Battle or transition detected. "
                            "Stopping the alignment test.",
                            COLOR_ORANGE
                        );
                        return false;
                    }

                    movements++;
                }
            }
            // Confirm a battle before using the existing escape helper.
            static bool mine_escape_and_settle(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);

                if (wait_until(stream, context, 12s, { {menu} }) != 0) {
                    stream.log(
                        "[Mine encounter] Battle menu not confirmed. Stopping.",
                        COLOR_ORANGE
                    );
                    return false;
                }

                stream.log("[Mine encounter] Battle confirmed. Escaping.");
                escape_wild_battle(stream, context);
                context.wait_for_all_requests();

                // Check the return waits instead of ignoring their results.
                BlackScreenWatcher visible(
                    COLOR_YELLOW,
                    ImageFloatBox(0.1, 0.1, 0.8, 0.8),
                    100,
                    10,
                    BlackScreenWatcher::FinderType::GONE,
                    500ms
                );

                if (wait_until(stream, context, 8s, { {visible} }) != 0) {
                    stream.log(
                        "[Mine encounter] Screen did not return. Stopping.",
                        COLOR_ORANGE
                    );
                    return false;
                }

                OverworldWatcher overworld;

                if (wait_until(stream, context, 5s, { {overworld} }) != 0) {
                    stream.log(
                        "[Mine encounter] Overworld not detected. Stopping.",
                        COLOR_ORANGE
                    );
                    return false;
                }

                pbf_wait(context, 800ms);
                context.wait_for_all_requests();

                stream.log(
                    "[Mine encounter] Escaped. Returning to the rock-walking loop.",
                    COLOR_GREEN
                );
                return true;
            }

            // Temporary test: wiggle along the corridor until a battle is detected.
            static bool mine_force_encounter_test(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log("[Mine encounter] Starting forced encounter wiggle.");

                const auto deadline =
                    std::chrono::steady_clock::now() + 60s;

                size_t pulses = 0;

                while (std::chrono::steady_clock::now() < deadline) {
                    // Alternate down and up. Do not move sideways.
                    const double y = pulses % 2 == 0 ? -1.0 : 1.0;

                    BlackScreenWatcher black;
                    BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
                    BattleDialogWatcher dialog(COLOR_YELLOW);

                    const int result = run_until<ProControllerContext>(
                        stream,
                        context,
                        [y](ProControllerContext& ctx) {
                            pbf_move_left_joystick(
                                ctx, { 0.0, y }, 120ms, 0ms
                            );
                            pbf_wait(ctx, 400ms);
                            ctx.wait_for_all_requests();
                        },
                        { black, menu, dialog }
                    );

                    if (result >= 0) {
                        stream.log(
                            "[Mine encounter] Wiggle interrupted. Checking for battle."
                        );
                        return mine_escape_and_settle(stream, context);
                    }

                    pulses++;
                }

                stream.log(
                    "[Mine encounter] No encounter within 60 seconds. "
                    "Test incomplete; stopping.",
                    COLOR_ORANGE
                );
                return false;
            }
            // Walk toward the first rock checkpoint, then stop.
            // This test stops on battles; escaping and resuming come next.
// Previous implementation retained during consolidation.
//             static bool oreburgh_mine_walk_to_first_rock(
//                 VideoStream& stream,
//                 ProControllerContext& context,
//                 bool force_encounter = false
//             ) {
//                 const ImageRGB32 reference(
//                     RESOURCE_PATH() + "PokemonBDSP/Autostory/Oreburg_mine_nav/mine_path_01.png"
//                 );
//                 const ImageRGB32 reference_scaled = reference.scale_to(960, 540);
//
//                 // Five small rocks and the large upper-right rock.
//                 // Boxes cover rock surfaces, avoiding the character and shadows.
//                 const std::array<ImageFloatBox, 6> rock_boxes = {
//                     ImageFloatBox(0.482, 0.585, 0.055, 0.055), // Central rock
//                     ImageFloatBox(0.335, 0.670, 0.050, 0.055), // Left pair: left
//                     ImageFloatBox(0.412, 0.670, 0.050, 0.055), // Left pair: right
//                     ImageFloatBox(0.570, 0.752, 0.045, 0.055), // Right pair: left
//                     ImageFloatBox(0.645, 0.752, 0.045, 0.055), // Right pair: right
//                     ImageFloatBox(0.570, 0.290, 0.080, 0.120)  // Large upper-right rock
//                 };
//                 bool forced_encounter_completed = false;
//                 size_t encounters = 0;
//
//                 constexpr size_t MAX_MOVEMENTS = 40;
//                 size_t movements = 0;
//                 size_t matching_frames = 0;
//                
//                 while (true) {
//                     context.wait_for_all_requests();
//                     const VideoSnapshot frame = stream.video().snapshot();
//
//                     if (!frame) {
//                         stream.log(
//                             "[Mine path 01] No video frame. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     const ImageRGB32 live_scaled = frame->scale_to(960, 540);
//
//                     std::array<double, 6> best_scores = {};
//                     double best_average = 1000.0;
//                     int best_shift = 0;
//                     int best_vertical_shift = 0;
//
//                     // Small horizontal and vertical search.
//                     // All six rocks must match at the SAME pair of offsets.
//                     for (int vertical = -6; vertical <= 6; vertical++) {
//                         for (int horizontal = -24; horizontal <= 24; horizontal++) {
//                             std::array<double, 6> scores = {};
//                             double total = 0.0;
//
//                             for (size_t rock = 0; rock < rock_boxes.size(); rock++) {
//                                 const ImageViewRGB32 reference_crop =
//                                     extract_box_reference(
//                                         reference_scaled, rock_boxes[rock]
//                                     );
//
//                                 const ImageViewRGB32 live_crop =
//                                     extract_box_reference(
//                                         live_scaled,
//                                         rock_boxes[rock],
//                                         horizontal,
//                                         vertical
//                                     );
//
//                                 scores[rock] =
//                                     ImageMatch::pixel_RMSD(reference_crop, live_crop);
//
//                                 total += scores[rock];
//                             }
//
//                             const double average = total / rock_boxes.size();
//
//                             if (average < best_average) {
//                                 best_average = average;
//                                 best_scores = scores;
//                                 best_shift = horizontal;
//                                 best_vertical_shift = vertical;
//                             }
//                         }
//                     }
//
//                     stream.log(
//                         "[Mine path 01] Horizontal=" + std::to_string(best_shift) +
//                         ", vertical=" + std::to_string(best_vertical_shift) +
//                         ", central=" + std::to_string(best_scores[0]) +
//                         ", left1=" + std::to_string(best_scores[1]) +
//                         ", left2=" + std::to_string(best_scores[2]) +
//                         ", right1=" + std::to_string(best_scores[3]) +
//                         ", right2=" + std::to_string(best_scores[4]) +
//                         ", large=" + std::to_string(best_scores[5])
//                     );
//
//                     // Initial limits: each rock must match, plus a good overall score.
//                     bool at_target = best_average <= 20.0;
//
//                     for (double score : best_scores) {
//                         if (score > 30.0) {
//                             at_target = false;
//                         }
//                     }
//
//                     if (at_target) {
//                         matching_frames++;
//
//                         if (matching_frames >= 3) {
//                             stream.log(
//                                 "[Mine path 01] First rock reached. Stopping.",
//                                 COLOR_GREEN
//                             );
//                             return true;
//                         }
//
//                         pbf_wait(context, 250ms);
//                         context.wait_for_all_requests();
//                         continue;
//                     }
//
//                     matching_frames = 0;
//
//                     // Trigger once, partway down the corridor.
//                     if (force_encounter &&
//                         !forced_encounter_completed &&
//                         movements >= 4) {
//
//                         if (!mine_force_encounter_test(stream, context)) {
//                             return false;
//                         }
//
//                         forced_encounter_completed = true;
//
//                         stream.log(
//                             "[Mine path 01] Forced encounter completed. Resuming navigation.",
//                             COLOR_GREEN
//                         );
//
//                         // Take a fresh picture before sending another movement.
//                         continue;
//                     }
//
//                     if (movements >= MAX_MOVEMENTS) {
//                         stream.log(
//                             "[Mine path 01] Movement limit reached. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     BlackScreenWatcher black;
//                     BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
//                     BattleDialogWatcher dialog(COLOR_YELLOW);
//
//                     const int result = run_until<ProControllerContext>(
//                         stream,
//                         context,
//                         [](ProControllerContext& ctx) {
//                             pbf_move_left_joystick(
//                                 ctx, { 0.0, -1.0 }, 80ms, 0ms
//                             );
//                             pbf_wait(ctx, 500ms);
//                             ctx.wait_for_all_requests();
//                         },
//                         { black, menu, dialog }
//                     );
//
//                     // Count commands as a safety budget, including interrupted commands.
//                     // This counter does not represent the player's position.
//                     movements++;
//
//                     if (result >= 0) {
//                         matching_frames = 0;
//
//                         if (++encounters > 10) {
//                             stream.log(
//                                 "[Mine path 01] Encounter limit reached. Stopping.",
//                                 COLOR_ORANGE
//                             );
//                             return false;
//                         }
//
//                         if (!mine_escape_and_settle(stream, context)) {
//                             return false;
//                         }
//
//                         stream.log("[Mine path 01] Resuming after encounter.");
//                         continue;
//                     }
//
//                 }
//             }
            // Move right until either accepted endpoint is recognized.
// Previous implementation retained during consolidation.
//             static bool oreburgh_mine_move_right(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 const ImageRGB32 reference1(
//                     RESOURCE_PATH() +
//                     "PokemonBDSP/Autostory/Oreburg_mine_nav/mine_path_01_1.png"
//                 );
//                 const ImageRGB32 reference2(
//                     RESOURCE_PATH() +
//                     "PokemonBDSP/Autostory/Oreburg_mine_nav/mine_path_01_2.png"
//                 );
//
//                 const std::array<ImageRGB32, 2> references = {
//                     reference1.scale_to(960, 540),
//                     reference2.scale_to(960, 540)
//                 };
//
//                 // Six rock regions for each endpoint.
//                 // The upper-right rock uses a higher patch to avoid the character.
//                 const std::array<std::array<ImageFloatBox, 6>, 2> boxes = { {
//                     {{
//                         ImageFloatBox(0.400, 0.585, 0.055, 0.055),
//                         ImageFloatBox(0.245, 0.670, 0.050, 0.055),
//                         ImageFloatBox(0.322, 0.670, 0.050, 0.055),
//                         ImageFloatBox(0.480, 0.752, 0.045, 0.055),
//                         ImageFloatBox(0.555, 0.752, 0.045, 0.055),
//                         ImageFloatBox(0.485, 0.290, 0.080, 0.090)
//                     }},
//                     {{
//                         ImageFloatBox(0.325, 0.585, 0.055, 0.055),
//                         ImageFloatBox(0.170, 0.670, 0.050, 0.055),
//                         ImageFloatBox(0.244, 0.670, 0.050, 0.055),
//                         ImageFloatBox(0.395, 0.752, 0.045, 0.055),
//                         ImageFloatBox(0.470, 0.752, 0.045, 0.055),
//                         ImageFloatBox(0.415, 0.290, 0.080, 0.090)
//                     }}
//                 } };
//
//                 size_t movements = 0;
//                 size_t encounters = 0;
//                 size_t matching_frames = 0;
//                 int previous_endpoint = -1;
//
//                 while (true) {
//                     context.wait_for_all_requests();
//                     const VideoSnapshot frame = stream.video().snapshot();
//
//                     if (!frame) {
//                         stream.log("[Mine right] No video frame. Stopping.", COLOR_ORANGE);
//                         return false;
//                     }
//
//                     const ImageRGB32 live = frame->scale_to(960, 540);
//                     int accepted_endpoint = -1;
//                     double accepted_score = 1000.0;
//
//                     for (size_t endpoint = 0; endpoint < references.size(); endpoint++) {
//                         double best_average = 1000.0;
//                         double best_worst = 1000.0;
//
//                         // Small tolerance around each accepted endpoint.
//                         for (int vertical = -6; vertical <= 6; vertical++) {
//                             for (int horizontal = -12; horizontal <= 12; horizontal++) {
//                                 double total = 0.0;
//                                 double worst = 0.0;
//
//                                 for (const ImageFloatBox& box : boxes[endpoint]) {
//                                     const ImageViewRGB32 reference_crop =
//                                         extract_box_reference(references[endpoint], box);
//
//                                     const ImageViewRGB32 live_crop =
//                                         extract_box_reference(
//                                             live, box, horizontal, vertical
//                                         );
//
//                                     const double score =
//                                         ImageMatch::pixel_RMSD(reference_crop, live_crop);
//
//                                     total += score;
//                                     if (score > worst) {
//                                         worst = score;
//                                     }
//                                 }
//
//                                 const double average = total / boxes[endpoint].size();
//
//                                 if (average < best_average) {
//                                     best_average = average;
//                                     best_worst = worst;
//                                 }
//                             }
//                         }
//
//                         stream.log(
//                             "[Mine right] Endpoint " + std::to_string(endpoint + 1) +
//                             ": average=" + std::to_string(best_average) +
//                             ", worst rock=" + std::to_string(best_worst)
//                         );
//
//                         if (best_average <= 20.0 &&
//                             best_worst <= 30.0 &&
//                             best_average < accepted_score) {
//
//                             accepted_endpoint = static_cast<int>(endpoint);
//                             accepted_score = best_average;
//                         }
//                     }
//
//                     if (accepted_endpoint >= 0) {
//                         matching_frames =
//                             accepted_endpoint == previous_endpoint
//                             ? matching_frames + 1
//                             : 1;
//
//                         previous_endpoint = accepted_endpoint;
//
//                         if (matching_frames >= 3) {
//                             stream.log(
//                                 "[Mine right] Endpoint " +
//                                 std::to_string(accepted_endpoint + 1) +
//                                 " reached. Stopping.",
//                                 COLOR_GREEN
//                             );
//                             return true;
//                         }
//
//                         pbf_wait(context, 250ms);
//                         context.wait_for_all_requests();
//                         continue;
//                     }
//
//                     matching_frames = 0;
//                     previous_endpoint = -1;
//
//                     if (movements >= 20) {
//                         stream.log(
//                             "[Mine right] Movement limit reached. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     BlackScreenWatcher black;
//                     BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
//                     BattleDialogWatcher dialog(COLOR_YELLOW);
//
//                     const int result = run_until<ProControllerContext>(
//                         stream,
//                         context,
//                         [](ProControllerContext& ctx) {
//                             pbf_move_left_joystick(
//                                 ctx, { 1.0, 0.0 }, 40ms, 0ms
//                             );
//                             pbf_wait(ctx, 500ms);
//                             ctx.wait_for_all_requests();
//                         },
//                         { black, menu, dialog }
//                     );
//
//                     movements++;
//
//                     if (result >= 0) {
//                         if (++encounters > 10) {
//                             stream.log(
//                                 "[Mine right] Encounter limit reached. Stopping.",
//                                 COLOR_ORANGE
//                             );
//                             return false;
//                         }
//
//                         if (!mine_escape_and_settle(stream, context)) {
//                             return false;
//                         }
//
//                         stream.log("[Mine right] Resuming after encounter.");
//                     }
//                 }
//             }
            // Walk down past the central rock to either accepted stopping point.
// Previous implementation retained during consolidation.
//             static bool oreburgh_mine_down_past_central_rock(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 const ImageRGB32 reference1(
//                     RESOURCE_PATH() +
//                     "PokemonBDSP/Autostory/Oreburg_mine_nav/mine_path_02_1.png"
//                 );
//                 const ImageRGB32 reference2(
//                     RESOURCE_PATH() +
//                     "PokemonBDSP/Autostory/Oreburg_mine_nav/mine_path_02_2.png"
//                 );
//
//                 const std::array<ImageRGB32, 2> references = {
//                     reference1.scale_to(960, 540),
//                     reference2.scale_to(960, 540)
//                 };
//
//                 // Rock surfaces in each reference picture.
//                 // Avoid the player standing above the lower-right pair.
//                 const std::array<std::array<ImageFloatBox, 6>, 2> boxes = { {
//                     {{
//                         ImageFloatBox(0.345, 0.420, 0.040, 0.045), // Central rock
//                         ImageFloatBox(0.195, 0.500, 0.040, 0.045), // Left pair: left
//                         ImageFloatBox(0.270, 0.500, 0.040, 0.045), // Left pair: right
//                         ImageFloatBox(0.410, 0.590, 0.040, 0.045), // Lower pair: left
//                         ImageFloatBox(0.480, 0.590, 0.040, 0.045), // Lower pair: right
//                         ImageFloatBox(0.445, 0.180, 0.060, 0.090)  // Large upper rock
//                     }},
//                     {{
//                         ImageFloatBox(0.420, 0.420, 0.040, 0.045),
//                         ImageFloatBox(0.270, 0.500, 0.040, 0.045),
//                         ImageFloatBox(0.345, 0.500, 0.040, 0.045),
//                         ImageFloatBox(0.485, 0.590, 0.040, 0.045),
//                         ImageFloatBox(0.555, 0.590, 0.040, 0.045),
//                         ImageFloatBox(0.520, 0.180, 0.060, 0.090)
//                     }}
//                 } };
//
//                 size_t movements = 0;
//                 size_t encounters = 0;
//                 size_t matching_frames = 0;
//                 int previous_endpoint = -1;
//
//                 while (true) {
//                     context.wait_for_all_requests();
//                     const VideoSnapshot frame = stream.video().snapshot();
//
//                     if (!frame) {
//                         stream.log("[Mine down 02] No video frame. Stopping.", COLOR_ORANGE);
//                         return false;
//                     }
//
//                     const ImageRGB32 live = frame->scale_to(960, 540);
//                     int accepted_endpoint = -1;
//                     double accepted_score = 1000.0;
//
//                     for (size_t endpoint = 0; endpoint < references.size(); endpoint++) {
//                         double best_average = 1000.0;
//                         double best_worst = 1000.0;
//
//                         // Allow small differences in the stopping position.
//                         for (int vertical = -6; vertical <= 6; vertical++) {
//                             for (int horizontal = -12; horizontal <= 12; horizontal++) {
//                                 double total = 0.0;
//                                 double worst = 0.0;
//
//                                 for (const ImageFloatBox& box : boxes[endpoint]) {
//                                     const ImageViewRGB32 reference_crop =
//                                         extract_box_reference(references[endpoint], box);
//
//                                     const ImageViewRGB32 live_crop =
//                                         extract_box_reference(
//                                             live, box, horizontal, vertical
//                                         );
//
//                                     const double score =
//                                         ImageMatch::pixel_RMSD(reference_crop, live_crop);
//
//                                     total += score;
//
//                                     if (score > worst) {
//                                         worst = score;
//                                     }
//                                 }
//
//                                 const double average = total / boxes[endpoint].size();
//
//                                 if (average < best_average) {
//                                     best_average = average;
//                                     best_worst = worst;
//                                 }
//                             }
//                         }
//
//                         stream.log(
//                             "[Mine down 02] Endpoint " + std::to_string(endpoint + 1) +
//                             ": average=" + std::to_string(best_average) +
//                             ", worst rock=" + std::to_string(best_worst)
//                         );
//
//                         if (best_average <= 20.0 &&
//                             best_worst <= 30.0 &&
//                             best_average < accepted_score) {
//
//                             accepted_endpoint = static_cast<int>(endpoint);
//                             accepted_score = best_average;
//                         }
//                     }
//
//                     if (accepted_endpoint >= 0) {
//                         matching_frames =
//                             accepted_endpoint == previous_endpoint
//                             ? matching_frames + 1
//                             : 1;
//
//                         previous_endpoint = accepted_endpoint;
//
//                         if (matching_frames >= 3) {
//                             stream.log(
//                                 "[Mine down 02] Endpoint " +
//                                 std::to_string(accepted_endpoint + 1) +
//                                 " reached. Stopping.",
//                                 COLOR_GREEN
//                             );
//                             return true;
//                         }
//
//                         pbf_wait(context, 250ms);
//                         context.wait_for_all_requests();
//                         continue;
//                     }
//
//                     matching_frames = 0;
//                     previous_endpoint = -1;
//
//                     if (movements >= 30) {
//                         stream.log(
//                             "[Mine down 02] Movement limit reached. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     BlackScreenWatcher black;
//                     BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
//                     BattleDialogWatcher dialog(COLOR_YELLOW);
//
//                     const int result = run_until<ProControllerContext>(
//                         stream,
//                         context,
//                         [](ProControllerContext& ctx) {
//                             pbf_move_left_joystick(
//                                 ctx, { 0.0, -1.0 }, 40ms, 0ms
//                             );
//                             pbf_wait(ctx, 500ms);
//                             ctx.wait_for_all_requests();
//                         },
//                         { black, menu, dialog }
//                     );
//
//                     movements++;
//
//                     if (result >= 0) {
//                         if (++encounters > 10) {
//                             stream.log(
//                                 "[Mine down 02] Encounter limit reached. Stopping.",
//                                 COLOR_ORANGE
//                             );
//                             return false;
//                         }
//
//                         if (!mine_escape_and_settle(stream, context)) {
//                             return false;
//                         }
//
//                         stream.log("[Mine down 02] Resuming after encounter.");
//                     }
//                 }
//             }

            // Move left into the central gap, then stop.
// Previous implementation retained during consolidation.
//             static bool oreburgh_mine_move_left_to_gap(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 const ImageRGB32 reference(
//                     RESOURCE_PATH() +
//                     "PokemonBDSP/Autostory/Oreburg_mine_nav/mine_path_03.png"
//                 );
//                 const ImageRGB32 reference_scaled = reference.scale_to(960, 540);
//
//                 // Four visible small rocks and the large upper-right rock.
//                 // Exclude the central rock hidden behind the character.
//                 const std::array<ImageFloatBox, 5> boxes = {
//                     ImageFloatBox(0.340, 0.495, 0.040, 0.045), // Left pair: left
//                     ImageFloatBox(0.410, 0.495, 0.040, 0.045), // Left pair: right
//                     ImageFloatBox(0.555, 0.580, 0.040, 0.045), // Right pair: left
//                     ImageFloatBox(0.625, 0.580, 0.040, 0.045), // Right pair: right
//                     ImageFloatBox(0.580, 0.180, 0.060, 0.090)  // Large upper-right rock
//                 };
//
//                 size_t movements = 0;
//                 size_t encounters = 0;
//                 size_t matching_frames = 0;
//
//                 while (true) {
//                     context.wait_for_all_requests();
//                     const VideoSnapshot frame = stream.video().snapshot();
//
//                     if (!frame) {
//                         stream.log("[Mine left 03] No video frame. Stopping.", COLOR_ORANGE);
//                         return false;
//                     }
//
//                     const ImageRGB32 live = frame->scale_to(960, 540);
//
//                     double best_average = 1000.0;
//                     double best_worst = 1000.0;
//                     int best_horizontal = 0;
//                     int best_vertical = 0;
//
//                     for (int vertical = -6; vertical <= 6; vertical++) {
//                         for (int horizontal = -12; horizontal <= 12; horizontal++) {
//                             double total = 0.0;
//                             double worst = 0.0;
//
//                             for (const ImageFloatBox& box : boxes) {
//                                 const ImageViewRGB32 reference_crop =
//                                     extract_box_reference(reference_scaled, box);
//
//                                 const ImageViewRGB32 live_crop =
//                                     extract_box_reference(
//                                         live, box, horizontal, vertical
//                                     );
//
//                                 const double score =
//                                     ImageMatch::pixel_RMSD(reference_crop, live_crop);
//
//                                 total += score;
//
//                                 if (score > worst) {
//                                     worst = score;
//                                 }
//                             }
//
//                             const double average = total / boxes.size();
//
//                             if (average < best_average) {
//                                 best_average = average;
//                                 best_worst = worst;
//                                 best_horizontal = horizontal;
//                                 best_vertical = vertical;
//                             }
//                         }
//                     }
//
//                     stream.log(
//                         "[Mine left 03] Horizontal=" +
//                         std::to_string(best_horizontal) +
//                         ", vertical=" + std::to_string(best_vertical) +
//                         ", average=" + std::to_string(best_average) +
//                         ", worst rock=" + std::to_string(best_worst)
//                     );
//
//                     const bool at_target =
//                         best_average <= 20.0 &&
//                         best_worst <= 30.0;
//
//                     if (at_target) {
//                         matching_frames++;
//
//                         if (matching_frames >= 3) {
//                             stream.log(
//                                 "[Mine left 03] Central gap reached. Stopping.",
//                                 COLOR_GREEN
//                             );
//                             return true;
//                         }
//
//                         pbf_wait(context, 250ms);
//                         context.wait_for_all_requests();
//                         continue;
//                     }
//
//                     matching_frames = 0;
//
//                     if (movements >= 30) {
//                         stream.log(
//                             "[Mine left 03] Movement limit reached. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     BlackScreenWatcher black;
//                     BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
//                     BattleDialogWatcher dialog(COLOR_YELLOW);
//
//                     const int result = run_until<ProControllerContext>(
//                         stream,
//                         context,
//                         [](ProControllerContext& ctx) {
//                             pbf_move_left_joystick(
//                                 ctx, { -1.0, 0.0 }, 40ms, 0ms
//                             );
//                             pbf_wait(ctx, 500ms);
//                             ctx.wait_for_all_requests();
//                         },
//                         { black, menu, dialog }
//                     );
//
//                     movements++;
//
//                     if (result >= 0) {
//                         if (++encounters > 10) {
//                             stream.log(
//                                 "[Mine left 03] Encounter limit reached. Stopping.",
//                                 COLOR_ORANGE
//                             );
//                             return false;
//                         }
//
//                         if (!mine_escape_and_settle(stream, context)) {
//                             return false;
//                         }
//
//                         stream.log("[Mine left 03] Resuming after encounter.");
//                     }
//                 }
//             }
            
            // Watch throughout an action, including its movement and picture checks.
            // After an encounter, restart the action using fresh watcher state.
            template <typename Action>
            static bool mine_run_with_encounters(
                VideoStream& stream,
                ProControllerContext& context,
                Action action
            ) {
                for (size_t encounters = 0;;) {
                    BlackScreenWatcher black;
                    BattleMenuWatcher menu(BattleType::STANDARD, COLOR_YELLOW);
                    BattleDialogWatcher dialog(COLOR_YELLOW);
                    bool completed = false;

                    const int result = run_until<ProControllerContext>(
                        stream, context,
                        [&](ProControllerContext& ctx) {
                            completed = action(ctx);
                        },
                        { black, menu, dialog }
                    );

                    if (result < 0) {
                        return completed;
                    }

                    if (++encounters > 10) {
                        stream.log("[Mine encounter] Encounter limit reached.", COLOR_ORANGE);
                        return false;
                    }

                    if (!mine_escape_and_settle(stream, context)) {
                        return false;
                    }

                    stream.log("[Mine encounter] Escaped; resuming interrupted action.");
                }
            }

            static bool mine_timed_walk(
                VideoStream& stream,
                ProControllerContext& context,
                double x,
                double y,
                std::chrono::milliseconds duration
            ) {
                auto remaining = duration;

                return mine_run_with_encounters(
                    stream, context,
                    [&](ProControllerContext& ctx) {
                        while (remaining > 0ms) {
                            const auto pulse = remaining < 100ms ? remaining : 100ms;
                            pbf_move_left_joystick(ctx, { x, y }, pulse, 0ms);
                            pbf_wait(ctx, 700ms);
                            ctx.wait_for_all_requests();

                            // An interrupted pulse is retried after escaping.
                            // This is a timing budget, not exact distance tracking.
                            remaining -= pulse;
                        }
                        return true;
                    }
                );
            }

            // Compatibility helper for callers such as alignment at position 18.
            static bool mine_watch_while_checking(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                return mine_run_with_encounters(
                    stream, context,
                    [](ProControllerContext& ctx) {
                        pbf_wait(ctx, 500ms);
                        ctx.wait_for_all_requests();
                        return true;
                    }
                );
            }

// Previous four-region scorer retained for comparison.
//             struct MinePictureScore {
//                 double average = 1000.0;
//                 double worst = 1000.0;
//             };
//
//             // Pure picture comparison: no movement, waits, or battle handling here.
//             static MinePictureScore mine_score_picture(
//                 const ImageRGB32& reference,
//                 const ImageRGB32& live,
//                 const std::array<ImageFloatBox, 4>& boxes
//             ) {
//                 MinePictureScore best;
//
//                 for (int vertical = -8; vertical <= 8; vertical++) {
//                     for (int horizontal = -16; horizontal <= 16; horizontal++) {
//                         double total = 0.0;
//                         double worst = 0.0;
//
//                         for (const ImageFloatBox& box : boxes) {
//                             const ImageViewRGB32 reference_crop =
//                                 extract_box_reference(reference, box);
//                             const ImageViewRGB32 live_crop =
//                                 extract_box_reference(live, box, horizontal, vertical);
//                             const double score =
//                                 ImageMatch::pixel_RMSD(reference_crop, live_crop);
//
//                             total += score;
//                             if (score > worst) {
//                                 worst = score;
//                             }
//                         }
//
//                         const double average = total / boxes.size();
//                         if (average < best.average) {
//                             best.average = average;
//                             best.worst = worst;
//                         }
//                     }
//                 }
//
//                 return best;
//             }

// Replace the existing MinePictureScore and mine_score_picture with this block.
struct MinePictureScore {
    double average = 1000.0;
    double worst = 1000.0;
    int horizontal = 0;
    int vertical = 0;
};

// Supports four, five, or six regions. Existing four-region callers keep
// their original search ranges through the default arguments.
template <size_t Regions>
static MinePictureScore mine_score_picture(
    const ImageRGB32& reference,
    const ImageRGB32& live,
    const std::array<ImageFloatBox, Regions>& boxes,
    int horizontal_range = 16,
    int vertical_range = 8
) {
    MinePictureScore best;
    for (int y = -vertical_range; y <= vertical_range; y++) {
        for (int x = -horizontal_range; x <= horizontal_range; x++) {
            double total = 0.0;
            double worst = 0.0;
            for (const ImageFloatBox& box : boxes) {
                const ImageViewRGB32 ref = extract_box_reference(reference, box);
                const ImageViewRGB32 patch = extract_box_reference(live, box, x, y);
                const double score = ImageMatch::pixel_RMSD(ref, patch);
                total += score;
                if (score > worst) worst = score;
            }
            const double average = total / boxes.size();
            if (average < best.average) {
                best.average = average;
                best.worst = worst;
                best.horizontal = x;
                best.vertical = y;
            }
        }
    }
    return best;
}


            static bool mine_check_picture(
                VideoStream& stream,
                ProControllerContext& context,
                const char* filename,
                const std::array<ImageFloatBox, 4>& boxes,
                const std::string& label,
                size_t max_attempts = 8,
                double max_average = 20.0,
                double max_patch = 30.0
            ) {
                const ImageRGB32 reference(
                    RESOURCE_PATH() +
                    "PokemonBDSP/Autostory/Oreburg_mine_nav/" + filename
                );
                const ImageRGB32 reference_scaled = reference.scale_to(960, 540);

                const bool confirmed = mine_run_with_encounters(
                    stream, context,
                    [&](ProControllerContext& ctx) {
                        // The watcher stays active throughout settling and sampling.
                        // There is no separate 500ms watcher wait before each sample.
                        ctx.wait_for_all_requests();
                        pbf_wait(ctx, 700ms);
                        ctx.wait_for_all_requests();

                        // Restart confirmation after a battle; do not reuse old matches.
                        size_t matching_frames = 0;

                        for (size_t attempt = 0; attempt < max_attempts; attempt++) {
                            const VideoSnapshot frame = stream.video().snapshot();
                            if (!frame) {
                                stream.log(label + ": no video frame.", COLOR_ORANGE);
                                return false;
                            }

                            const ImageRGB32 live = frame->scale_to(960, 540);
                            const MinePictureScore score =
                                mine_score_picture(reference_scaled, live, boxes);

                            stream.log(
                                label + ": average=" + std::to_string(score.average) +
                                ", worst patch=" + std::to_string(score.worst)
                            );

                            if (score.average <= max_average && score.worst <= max_patch) {
                                if (++matching_frames >= 3) {
                                    return true;
                                }
                            }
                            else {
                                matching_frames = 0;
                            }

                            pbf_wait(ctx, 250ms);
                            ctx.wait_for_all_requests();
                        }

                        return false;
                    }
                );

                stream.log(
                    label + (confirmed ? ": confirmed." : ": not confirmed."),
                    confirmed ? COLOR_GREEN : COLOR_ORANGE
                );
                return confirmed;
            }

// Paste this block AFTER mine_check_picture and BEFORE oreburgh_mine_confirm_05.
// Keep the four old early-route functions commented out above this block.

// Previous early-stage engine retained for comparison.
// struct MineEarlyWalkOptions {
//     double x = 0.0;
//     double y = 0.0;
//     std::chrono::milliseconds pulse = 40ms;
//     std::chrono::milliseconds settle = 500ms;
//     size_t max_movements = 30;
//     int horizontal_range = 12;
//     int vertical_range = 6;
//     double max_average = 20.0;
//     double max_patch = 30.0;
// };
//
// template <size_t Regions, size_t Endpoints>
// static bool mine_walk_early_endpoints(
//     VideoStream& stream,
//     ProControllerContext& context,
//     const std::array<const char*, Endpoints>& files,
//     const std::array<std::array<ImageFloatBox, Regions>, Endpoints>& boxes,
//     const MineEarlyWalkOptions& options,
//     const std::string& label,
//     bool force_encounter = false
// ) {
//     std::array<std::unique_ptr<ImageRGB32>, Endpoints> references;
//     for (size_t index = 0; index < Endpoints; index++) {
//         const ImageRGB32 image(
//             RESOURCE_PATH() + "PokemonBDSP/Autostory/Oreburg_mine_nav/" + files[index]
//         );
//         references[index] = std::make_unique<ImageRGB32>(image.scale_to(960, 540));
//     }
//
//     size_t movements = 0;
//     bool forced_completed = false;
//
//     while (true) {
//         bool request_forced_test = false;
//         const bool reached = mine_run_with_encounters(
//             stream, context,
//             [&](ProControllerContext& ctx) {
//                 // Reset the match history whenever an encounter interrupts us.
//                 size_t matching_frames = 0;
//                 int previous_endpoint = -1;
//
//                 while (true) {
//                     ctx.wait_for_all_requests();
//                     const VideoSnapshot frame = stream.video().snapshot();
//                     if (!frame) {
//                         stream.log(label + ": no video frame.", COLOR_ORANGE);
//                         return false;
//                     }
//
//                     const ImageRGB32 live = frame->scale_to(960, 540);
//                     int accepted = -1;
//                     double accepted_average = 1000.0;
//
//                     for (size_t endpoint = 0; endpoint < Endpoints; endpoint++) {
//                         const MinePictureScore score = mine_score_picture(
//                             *references[endpoint], live, boxes[endpoint],
//                             options.horizontal_range, options.vertical_range
//                         );
//                         stream.log(
//                             label + " Endpoint " + std::to_string(endpoint + 1) +
//                             ": average=" + std::to_string(score.average) +
//                             ", worst patch=" + std::to_string(score.worst) +
//                             ", offset=(" + std::to_string(score.horizontal) +
//                             "," + std::to_string(score.vertical) + ")"
//                         );
//
//                         if (score.average <= options.max_average &&
//                             score.worst <= options.max_patch &&
//                             score.average < accepted_average) {
//                             accepted = static_cast<int>(endpoint);
//                             accepted_average = score.average;
//                         }
//                     }
//
//                     if (accepted >= 0) {
//                         matching_frames = accepted == previous_endpoint
//                             ? matching_frames + 1 : 1;
//                         previous_endpoint = accepted;
//                         if (matching_frames >= 3) {
//                             stream.log(
//                                 label + " Endpoint " + std::to_string(accepted + 1) +
//                                 " confirmed.", COLOR_GREEN
//                             );
//                             return true;
//                         }
//                         pbf_wait(ctx, 250ms);
//                         ctx.wait_for_all_requests();
//                         continue;
//                     }
//
//                     matching_frames = 0;
//                     previous_endpoint = -1;
//
//                     // Exit the guard before invoking the diagnostic wiggle:
//                     // it has its own encounter watcher and must not be nested.
//                     if (force_encounter && !forced_completed && movements >= 4) {
//                         request_forced_test = true;
//                         return false;
//                     }
//
//                     if (movements >= options.max_movements) {
//                         stream.log(label + ": movement limit reached.", COLOR_ORANGE);
//                         return false;
//                     }
//
//                     // Count issued commands, including interrupted commands.
//                     movements++;
//                     pbf_move_left_joystick(
//                         ctx, { options.x, options.y }, options.pulse, 0ms
//                     );
//                     pbf_wait(ctx, options.settle);
//                     ctx.wait_for_all_requests();
//                 }
//             }
//         );
//
//         if (reached) return true;
//         if (!request_forced_test) return false;
//         if (!mine_force_encounter_test(stream, context)) return false;
//         forced_completed = true;
//         stream.log(label + ": forced encounter completed; resuming.");
//     }
// }
//
// static bool oreburgh_mine_walk_to_first_rock(
//     VideoStream& stream,
//     ProControllerContext& context,
//     bool force_encounter = false
// ) {
//     const std::array<const char*, 1> files = { "mine_path_01.png" };
//     const std::array<std::array<ImageFloatBox, 6>, 1> boxes = {{
//         {{
//             ImageFloatBox(0.482, 0.585, 0.055, 0.055),
//             ImageFloatBox(0.335, 0.670, 0.050, 0.055),
//             ImageFloatBox(0.412, 0.670, 0.050, 0.055),
//             ImageFloatBox(0.570, 0.752, 0.045, 0.055),
//             ImageFloatBox(0.645, 0.752, 0.045, 0.055),
//             ImageFloatBox(0.570, 0.290, 0.080, 0.120)
//         }}
//     }};
//
//     MineEarlyWalkOptions options;
//     options.x = 0.0;
//     options.y = -1.0;
//     options.pulse = 80ms;
//     options.max_movements = 40;
//     options.horizontal_range = 24;
//     return mine_walk_early_endpoints(
//         stream, context, files, boxes, options, "[Mine path 01]", force_encounter
//     );
// }
//
// static bool oreburgh_mine_move_right(
//     VideoStream& stream,
//     ProControllerContext& context
// ) {
//     const std::array<const char*, 2> files = { "mine_path_01_1.png", "mine_path_01_2.png" };
//     const std::array<std::array<ImageFloatBox, 6>, 2> boxes = {{
//         {{
//             ImageFloatBox(0.400, 0.585, 0.055, 0.055),
//             ImageFloatBox(0.245, 0.670, 0.050, 0.055),
//             ImageFloatBox(0.322, 0.670, 0.050, 0.055),
//             ImageFloatBox(0.480, 0.752, 0.045, 0.055),
//             ImageFloatBox(0.555, 0.752, 0.045, 0.055),
//             ImageFloatBox(0.485, 0.290, 0.080, 0.090)
//         }},
//         {{
//             ImageFloatBox(0.325, 0.585, 0.055, 0.055),
//             ImageFloatBox(0.170, 0.670, 0.050, 0.055),
//             ImageFloatBox(0.244, 0.670, 0.050, 0.055),
//             ImageFloatBox(0.395, 0.752, 0.045, 0.055),
//             ImageFloatBox(0.470, 0.752, 0.045, 0.055),
//             ImageFloatBox(0.415, 0.290, 0.080, 0.090)
//         }}
//     }};
//
//     MineEarlyWalkOptions options;
//     options.x = 1.0;
//     options.y = 0.0;
//     options.pulse = 40ms;
//     options.max_movements = 20;
//     options.horizontal_range = 12;
//     return mine_walk_early_endpoints(
//         stream, context, files, boxes, options, "[Mine right]"
//     );
// }
//
// static bool oreburgh_mine_down_past_central_rock(
//     VideoStream& stream,
//     ProControllerContext& context
// ) {
//     const std::array<const char*, 2> files = { "mine_path_02_1.png", "mine_path_02_2.png" };
//     const std::array<std::array<ImageFloatBox, 6>, 2> boxes = {{
//         {{
//             ImageFloatBox(0.345, 0.420, 0.040, 0.045),
//             ImageFloatBox(0.195, 0.500, 0.040, 0.045),
//             ImageFloatBox(0.270, 0.500, 0.040, 0.045),
//             ImageFloatBox(0.410, 0.590, 0.040, 0.045),
//             ImageFloatBox(0.480, 0.590, 0.040, 0.045),
//             ImageFloatBox(0.445, 0.180, 0.060, 0.090)
//         }},
//         {{
//             ImageFloatBox(0.420, 0.420, 0.040, 0.045),
//             ImageFloatBox(0.270, 0.500, 0.040, 0.045),
//             ImageFloatBox(0.345, 0.500, 0.040, 0.045),
//             ImageFloatBox(0.485, 0.590, 0.040, 0.045),
//             ImageFloatBox(0.555, 0.590, 0.040, 0.045),
//             ImageFloatBox(0.520, 0.180, 0.060, 0.090)
//         }}
//     }};
//
//     MineEarlyWalkOptions options;
//     options.x = 0.0;
//     options.y = -1.0;
//     options.pulse = 40ms;
//     options.max_movements = 30;
//     options.horizontal_range = 12;
//     return mine_walk_early_endpoints(
//         stream, context, files, boxes, options, "[Mine down 02]"
//     );
// }
//
// static bool oreburgh_mine_move_left_to_gap(
//     VideoStream& stream,
//     ProControllerContext& context
// ) {
//     const std::array<const char*, 1> files = { "mine_path_03.png" };
//     const std::array<std::array<ImageFloatBox, 5>, 1> boxes = {{
//         {{
//             ImageFloatBox(0.340, 0.495, 0.040, 0.045),
//             ImageFloatBox(0.410, 0.495, 0.040, 0.045),
//             ImageFloatBox(0.555, 0.580, 0.040, 0.045),
//             ImageFloatBox(0.625, 0.580, 0.040, 0.045),
//             ImageFloatBox(0.580, 0.180, 0.060, 0.090)
//         }}
//     }};
//
//     MineEarlyWalkOptions options;
//     options.x = -1.0;
//     options.y = 0.0;
//     options.pulse = 40ms;
//     options.max_movements = 30;
//     options.horizontal_range = 12;
//     return mine_walk_early_endpoints(
//         stream, context, files, boxes, options, "[Mine left 03]"
//     );
// }

struct MineWalkOptions {
    double x = 0.0;
    double y = 0.0;
    std::chrono::milliseconds pulse = 40ms;
    std::chrono::milliseconds settle = 500ms;
    std::chrono::milliseconds initial_settle = 0ms;
    size_t max_movements = 30;
    int horizontal_range = 12;
    int vertical_range = 6;
    double max_average = 20.0;
    double max_patch = 30.0;
};

template <size_t Regions, size_t Endpoints>
static bool mine_walk_to_endpoints(
    VideoStream& stream,
    ProControllerContext& context,
    const std::array<const char*, Endpoints>& files,
    const std::array<std::array<ImageFloatBox, Regions>, Endpoints>& boxes,
    const MineWalkOptions& options,
    const std::string& label,
    bool force_encounter = false
) {
    std::array<std::unique_ptr<ImageRGB32>, Endpoints> references;
    for (size_t index = 0; index < Endpoints; index++) {
        const ImageRGB32 image(
            RESOURCE_PATH() + "PokemonBDSP/Autostory/Oreburg_mine_nav/" + files[index]
        );
        references[index] = std::make_unique<ImageRGB32>(image.scale_to(960, 540));
    }

    size_t movements = 0;
    bool forced_completed = false;

    while (true) {
        bool request_forced_test = false;
        const bool reached = mine_run_with_encounters(
            stream, context,
            [&](ProControllerContext& ctx) {
                if (options.initial_settle > 0ms) {
                    pbf_wait(ctx, options.initial_settle);
                    ctx.wait_for_all_requests();
                }
                // Reset the match history whenever an encounter interrupts us.
                size_t matching_frames = 0;
                int previous_endpoint = -1;

                while (true) {
                    ctx.wait_for_all_requests();
                    const VideoSnapshot frame = stream.video().snapshot();
                    if (!frame) {
                        stream.log(label + ": no video frame.", COLOR_ORANGE);
                        return false;
                    }

                    const ImageRGB32 live = frame->scale_to(960, 540);
                    int accepted = -1;
                    double accepted_average = 1000.0;

                    for (size_t endpoint = 0; endpoint < Endpoints; endpoint++) {
                        const MinePictureScore score = mine_score_picture(
                            *references[endpoint], live, boxes[endpoint],
                            options.horizontal_range, options.vertical_range
                        );
                        stream.log(
                            label + " Endpoint " + std::to_string(endpoint + 1) +
                            ": average=" + std::to_string(score.average) +
                            ", worst patch=" + std::to_string(score.worst) +
                            ", offset=(" + std::to_string(score.horizontal) +
                            "," + std::to_string(score.vertical) + ")"
                        );

                        if (score.average <= options.max_average &&
                            score.worst <= options.max_patch &&
                            score.average < accepted_average) {
                            accepted = static_cast<int>(endpoint);
                            accepted_average = score.average;
                        }
                    }

                    if (accepted >= 0) {
                        matching_frames = accepted == previous_endpoint
                            ? matching_frames + 1 : 1;
                        previous_endpoint = accepted;
                        if (matching_frames >= 3) {
                            stream.log(
                                label + " Endpoint " + std::to_string(accepted + 1) +
                                " confirmed.", COLOR_GREEN
                            );
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
                    if (force_encounter && !forced_completed && movements >= 4) {
                        request_forced_test = true;
                        return false;
                    }

                    if (movements >= options.max_movements) {
                        stream.log(label + ": movement limit reached.", COLOR_ORANGE);
                        return false;
                    }

                    // Count issued commands, including interrupted commands.
                    movements++;
                    pbf_move_left_joystick(
                        ctx, { options.x, options.y }, options.pulse, 0ms
                    );
                    pbf_wait(ctx, options.settle);
                    ctx.wait_for_all_requests();
                }
            }
        );

        if (reached) return true;
        if (!request_forced_test) return false;
        if (!mine_force_encounter_test(stream, context)) return false;
        forced_completed = true;
        stream.log(label + ": forced encounter completed; resuming.");
    }
}

static bool oreburgh_mine_walk_to_first_rock(
    VideoStream& stream,
    ProControllerContext& context,
    bool force_encounter = false
) {
    const std::array<const char*, 1> files = { "mine_path_01.png" };
    const std::array<std::array<ImageFloatBox, 6>, 1> boxes = {{
        {{
            ImageFloatBox(0.482, 0.585, 0.055, 0.055),
            ImageFloatBox(0.335, 0.670, 0.050, 0.055),
            ImageFloatBox(0.412, 0.670, 0.050, 0.055),
            ImageFloatBox(0.570, 0.752, 0.045, 0.055),
            ImageFloatBox(0.645, 0.752, 0.045, 0.055),
            ImageFloatBox(0.570, 0.290, 0.080, 0.120)
        }}
    }};

    MineWalkOptions options;
    options.x = 0.0;
    options.y = -1.0;
    options.pulse = 80ms;
    options.max_movements = 40;
    options.horizontal_range = 24;
    return mine_walk_to_endpoints(
        stream, context, files, boxes, options, "[Mine path 01]", force_encounter
    );
}

static bool oreburgh_mine_move_right(
    VideoStream& stream,
    ProControllerContext& context
) {
    const std::array<const char*, 2> files = { "mine_path_01_1.png", "mine_path_01_2.png" };
    const std::array<std::array<ImageFloatBox, 6>, 2> boxes = {{
        {{
            ImageFloatBox(0.400, 0.585, 0.055, 0.055),
            ImageFloatBox(0.245, 0.670, 0.050, 0.055),
            ImageFloatBox(0.322, 0.670, 0.050, 0.055),
            ImageFloatBox(0.480, 0.752, 0.045, 0.055),
            ImageFloatBox(0.555, 0.752, 0.045, 0.055),
            ImageFloatBox(0.485, 0.290, 0.080, 0.090)
        }},
        {{
            ImageFloatBox(0.325, 0.585, 0.055, 0.055),
            ImageFloatBox(0.170, 0.670, 0.050, 0.055),
            ImageFloatBox(0.244, 0.670, 0.050, 0.055),
            ImageFloatBox(0.395, 0.752, 0.045, 0.055),
            ImageFloatBox(0.470, 0.752, 0.045, 0.055),
            ImageFloatBox(0.415, 0.290, 0.080, 0.090)
        }}
    }};

    MineWalkOptions options;
    options.x = 1.0;
    options.y = 0.0;
    options.pulse = 40ms;
    options.max_movements = 20;
    options.horizontal_range = 12;
    return mine_walk_to_endpoints(
        stream, context, files, boxes, options, "[Mine right]"
    );
}

static bool oreburgh_mine_down_past_central_rock(
    VideoStream& stream,
    ProControllerContext& context
) {
    const std::array<const char*, 2> files = { "mine_path_02_1.png", "mine_path_02_2.png" };
    const std::array<std::array<ImageFloatBox, 6>, 2> boxes = {{
        {{
            ImageFloatBox(0.345, 0.420, 0.040, 0.045),
            ImageFloatBox(0.195, 0.500, 0.040, 0.045),
            ImageFloatBox(0.270, 0.500, 0.040, 0.045),
            ImageFloatBox(0.410, 0.590, 0.040, 0.045),
            ImageFloatBox(0.480, 0.590, 0.040, 0.045),
            ImageFloatBox(0.445, 0.180, 0.060, 0.090)
        }},
        {{
            ImageFloatBox(0.420, 0.420, 0.040, 0.045),
            ImageFloatBox(0.270, 0.500, 0.040, 0.045),
            ImageFloatBox(0.345, 0.500, 0.040, 0.045),
            ImageFloatBox(0.485, 0.590, 0.040, 0.045),
            ImageFloatBox(0.555, 0.590, 0.040, 0.045),
            ImageFloatBox(0.520, 0.180, 0.060, 0.090)
        }}
    }};

    MineWalkOptions options;
    options.x = 0.0;
    options.y = -1.0;
    options.pulse = 40ms;
    options.max_movements = 30;
    options.horizontal_range = 12;
    return mine_walk_to_endpoints(
        stream, context, files, boxes, options, "[Mine down 02]"
    );
}

static bool oreburgh_mine_move_left_to_gap(
    VideoStream& stream,
    ProControllerContext& context
) {
    const std::array<const char*, 1> files = { "mine_path_03.png" };
    const std::array<std::array<ImageFloatBox, 5>, 1> boxes = {{
        {{
            ImageFloatBox(0.340, 0.495, 0.040, 0.045),
            ImageFloatBox(0.410, 0.495, 0.040, 0.045),
            ImageFloatBox(0.555, 0.580, 0.040, 0.045),
            ImageFloatBox(0.625, 0.580, 0.040, 0.045),
            ImageFloatBox(0.580, 0.180, 0.060, 0.090)
        }}
    }};

    MineWalkOptions options;
    options.x = -1.0;
    options.y = 0.0;
    options.pulse = 40ms;
    options.max_movements = 30;
    options.horizontal_range = 12;
    return mine_walk_to_endpoints(
        stream, context, files, boxes, options, "[Mine left 03]"
    );
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

static bool mine_run_picture_step(
    VideoStream& stream, ProControllerContext& context,
    const MinePictureStep& step
) {
    MineWalkOptions options;
    options.x = step.x;
    options.y = step.y;
    options.pulse = step.pulse;
    options.max_movements = step.max_movements;
    options.max_average = step.max_average;
    options.max_patch = step.max_patch;
    options.horizontal_range = 16;
    options.vertical_range = 8;
    options.initial_settle = 700ms;
    options.settle = 700ms;
    const std::array<const char*, 1> files = {step.file};
    const std::array<std::array<ImageFloatBox, 4>, 1> boxes = {{step.boxes}};
    return mine_walk_to_endpoints(stream, context, files, boxes, options, step.label);
}

// Previous checkpoint loop retained for comparison.
//             static bool oreburgh_mine_confirm_05(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 // Stationary rock surfaces in mine_path_05.png.
//                 // Avoid the hidden player, NPC, and Machop.
//                 const std::array<ImageFloatBox, 4> boxes = {
//                     ImageFloatBox(0.240, 0.240, 0.055, 0.080), // Large left rock
//                     ImageFloatBox(0.390, 0.220, 0.050, 0.090), // Large middle rock
//                     ImageFloatBox(0.360, 0.090, 0.030, 0.030), // Upper-left small rock
//                     ImageFloatBox(0.550, 0.140, 0.040, 0.040)  // Upper-right small rock
//                 };
//
//                 for (size_t extra = 0; extra <= 10; extra++) {
//                     if (mine_check_picture(
//                         stream,
//                         context,
//                         "mine_path_05.png",
//                         boxes,
//                         "[Mine checkpoint 05]",
//                         3
//                     )) {
//                         stream.log(
//                             "[Mine checkpoint 05] Confirmed; leftward movement can begin.",
//                             COLOR_GREEN
//                         );
//                         return true;
//                     }
//
//                     if (extra == 10) {
//                         stream.log(
//                             "[Mine checkpoint 05] Extra movement limit reached. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     stream.log("[Mine checkpoint 05] Adding 60ms downward movement.");
//
//                     if (!mine_timed_walk(
//                         stream, context, 0.0, -1.0, 60ms
//                     )) {
//                         return false;
//                     }
//                 }
//
//                 return false;
//             }

static bool oreburgh_mine_confirm_05(VideoStream& stream, ProControllerContext& context) {
    const MinePictureStep step = {
        "mine_path_05.png", "[Mine endpoint 05]",
        {ImageFloatBox(0.240, 0.240, 0.055, 0.080), ImageFloatBox(0.390, 0.220, 0.050, 0.090), ImageFloatBox(0.360, 0.090, 0.030, 0.030), ImageFloatBox(0.550, 0.140, 0.040, 0.040)},
        0.0, -1.0, 60ms, 10, 20.0, 30.0
    };
    return mine_run_picture_step(stream, context, step);
}

            // Temporary timed trial from the central gap through paths 6 and 7.
            static bool oreburgh_mine_timed_to_07(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log("[Mine 04-07] Walking down for 1500ms.");

                if (!mine_timed_walk(stream, context, 0.0, -1.0, 1500ms)) {
                    return false;
                }

                if (!oreburgh_mine_confirm_05(stream, context)) {
                    return false;
                }

                stream.log("[Mine 04-07] Walking left for 3000ms.");

                if (!mine_timed_walk(stream, context, -1.0, 0.0, 3000ms)) {
                    return false;
                }

                // Path 6: crate, nearby small rock, large rock, upper-right small rock.
                const std::array<ImageFloatBox, 4> path6_boxes = {
                    ImageFloatBox(0.350, 0.250, 0.085, 0.075),
                    ImageFloatBox(0.405, 0.510, 0.035, 0.040),
                    ImageFloatBox(0.520, 0.300, 0.050, 0.080),
                    ImageFloatBox(0.620, 0.350, 0.030, 0.040)
                };

                if (!mine_check_picture(
                    stream, context,
                    "mine_path_06.png", path6_boxes,
                    "[Mine midpoint 06]"
                )) {
                    return false;
                }

                // Initial estimate: tune after the first live test.
                stream.log("[Mine 04-07] Path 6 confirmed; walking down for 600ms.");

                if (!mine_timed_walk(stream, context, 0.0, -1.0, 600ms)) {
                    return false;
                }

                // Path 7: crate and three visible rock surfaces.
                const std::array<ImageFloatBox, 4> path7_boxes = {
                    ImageFloatBox(0.365, 0.025, 0.070, 0.055),
                    ImageFloatBox(0.420, 0.270, 0.035, 0.040),
                    ImageFloatBox(0.560, 0.415, 0.025, 0.040),
                    ImageFloatBox(0.530, 0.650, 0.060, 0.090)
                };

                return mine_check_picture(
                    stream, context,
                    "mine_path_07.png", path7_boxes,
                    "[Mine endpoint 07]"
                );
            }

            // Move left from path 7 to the position shown in path 8.

// Previous checkpoint loop retained for comparison.
//             static bool oreburgh_mine_left_to_08(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 // Crate, upper small rock, ledge-side rock, and lower-left small rock.
//                 // Avoid the character and its shadow.
//                 const std::array<ImageFloatBox, 4> boxes = {
//                     ImageFloatBox(0.495, 0.035, 0.070, 0.055),
//                     ImageFloatBox(0.550, 0.270, 0.035, 0.040),
//                     ImageFloatBox(0.695, 0.420, 0.030, 0.040),
//                     ImageFloatBox(0.555, 0.585, 0.035, 0.040)
//                 };
//
//                 // Check before every pulse, including after the final allowed pulse.
//                 for (size_t movement = 0; movement <= 20; movement++) {
//                     if (mine_check_picture(
//                         stream,
//                         context,
//                         "mine_path_08.png",
//                         boxes,
//                         "[Mine endpoint 08]",
//                         3
//                     )) {
//                         stream.log(
//                             "[Mine left 08] Left checkpoint reached. Stopping.",
//                             COLOR_GREEN
//                         );
//                         return true;
//                     }
//
//                     if (movement == 20) {
//                         stream.log(
//                             "[Mine left 08] Movement limit reached. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     stream.log("[Mine left 08] Moving left for 40ms.");
//
//                     if (!mine_timed_walk(
//                         stream, context, -1.0, 0.0, 40ms
//                     )) {
//                         return false;
//                     }
//                 }
//
//                 return false;
//             }

static bool oreburgh_mine_left_to_08(VideoStream& stream, ProControllerContext& context) {
    const MinePictureStep step = {
        "mine_path_08.png", "[Mine endpoint 08]",
        {ImageFloatBox(0.495, 0.035, 0.070, 0.055), ImageFloatBox(0.550, 0.270, 0.035, 0.040), ImageFloatBox(0.695, 0.420, 0.030, 0.040), ImageFloatBox(0.555, 0.585, 0.035, 0.040)},
        -1.0, 0.0, 40ms, 20, 20.0, 30.0
    };
    return mine_run_picture_step(stream, context, step);
}

            // Walk down along the left wall to path 9.

// Previous checkpoint loop retained for comparison.
//             static bool oreburgh_mine_down_to_09(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 // Upper small rock, rock pair, and the crate below the player.
//                 // Avoid the player and shadow.
//                 const std::array<ImageFloatBox, 4> boxes = {
//                     ImageFloatBox(0.550, 0.115, 0.030, 0.040),
//                     ImageFloatBox(0.550, 0.390, 0.035, 0.040),
//                     ImageFloatBox(0.620, 0.390, 0.025, 0.035),
//                     ImageFloatBox(0.500, 0.720, 0.085, 0.080)
//                 };
//
//                 // Check before each movement and after the last allowed movement.
//                 for (size_t movement = 0; movement <= 25; movement++) {
//                     if (mine_check_picture(
//                         stream,
//                         context,
//                         "mine_path_09.png",
//                         boxes,
//                         "[Mine endpoint 09]",
//                         3,
//                         25.0,
//                         35.0
//                     )) {
//                         stream.log(
//                             "[Mine down 09] Checkpoint reached. Stopping.",
//                             COLOR_GREEN
//                         );
//                         return true;
//                     }
//
//                     if (movement == 25) {
//                         stream.log(
//                             "[Mine down 09] Movement limit reached. Stopping.",
//                             COLOR_ORANGE
//                         );
//                         return false;
//                     }
//
//                     stream.log("[Mine down 09] Moving down for 60ms.");
//
//                     if (!mine_timed_walk(
//                         stream, context, 0.0, -1.0, 40ms
//                     )) {
//                         return false;
//                     }
//                 }
//
//                 return false;
//             }

static bool oreburgh_mine_down_to_09(VideoStream& stream, ProControllerContext& context) {
    const MinePictureStep step = {
        "mine_path_09.png", "[Mine endpoint 09]",
        {ImageFloatBox(0.550, 0.115, 0.030, 0.040), ImageFloatBox(0.550, 0.390, 0.035, 0.040), ImageFloatBox(0.620, 0.390, 0.025, 0.035), ImageFloatBox(0.500, 0.720, 0.085, 0.080)},
        0.0, -1.0, 40ms, 25, 25.0, 35.0
    };
    return mine_run_picture_step(stream, context, step);
}

// Previous per-pulse picture loader retained for comparison.
//
//             static bool mine_walk_to_picture(
//                 VideoStream& stream,
//                 ProControllerContext& context,
//                 double x,
//                 double y,
//                 const char* filename,
//                 const std::array<ImageFloatBox, 4>& boxes,
//                 const std::string& label
//             ) {
//                 for (size_t movement = 0; movement <= 30; movement++) {
//                     if (mine_check_picture(
//                         stream, context, filename, boxes, label, 3
//                     )) {
//                         return true;
//                     }
//
//                     if (movement == 30) {
//                         stream.log(label + ": movement limit reached.", COLOR_ORANGE);
//                         return false;
//                     }
//
//                     if (!mine_timed_walk(stream, context, x, y, 80ms)) {
//                         return false;
//                     }
//                 }
//
//                 return false;
//             }

            static bool mine_walk_to_picture(
                VideoStream& stream,
                ProControllerContext& context,
                double x,
                double y,
                const char* filename,
                const std::array<ImageFloatBox, 4>& boxes,
                const std::string& label
            ) {
    const MinePictureStep step = {filename, label.c_str(), boxes, x, y};
    return mine_run_picture_step(stream, context, step);
}

            // Require dialogue to appear first.
            // Then finish only after it has been absent for one continuous second.
            static bool mine_finish_item_dialogue(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                ShortDialogDetector dialogue(COLOR_YELLOW);

                bool saw_dialogue = false;
                auto absent_since = std::chrono::steady_clock::now();

                const auto start = std::chrono::steady_clock::now();
                const auto deadline = start + 30s;

                stream.log("[Mine item] Starting interaction.");

                while (std::chrono::steady_clock::now() < deadline) {
                    context.wait_for_all_requests();
                    const VideoSnapshot frame = stream.video().snapshot();

                    if (!frame) {
                        stream.log("[Mine item] No video frame. Stopping.", COLOR_ORANGE);
                        return false;
                    }

                    const auto now = std::chrono::steady_clock::now();
                    const bool visible = dialogue.detect(frame);

                    if (visible) {
                        if (!saw_dialogue) {
                            stream.log("[Mine item] Dialogue detected.");
                        }

                        saw_dialogue = true;
                        absent_since = now;

                        // Short mash bursts allow frequent checks.
                        pbf_mash_button(context, BUTTON_A, 100ms);
                    }
                    else if (!saw_dialogue) {
                        if (now - start >= 10s) {
                            stream.log(
                                "[Mine item] No dialogue appeared. Stopping.",
                                COLOR_ORANGE
                            );
                            return false;
                        }

                        // Try to start the interaction.
                        pbf_press_button(context, BUTTON_A, 60ms, 100ms);
                    }
                    else if (now - absent_since >= 1s) {
                        stream.log(
                            "[Mine item] Dialogue absent for one second. Finished.",
                            COLOR_GREEN
                        );
                        return true;
                    }

                    pbf_wait(context, 100ms);
                    context.wait_for_all_requests();
                }

                stream.log("[Mine item] Dialogue timeout. Stopping.", COLOR_ORANGE);
                return false;
            }

// Previous route block retained for comparison.
//             static bool oreburgh_mine_09_to_14(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 const std::array<ImageFloatBox, 4> path10_boxes = {
//                     ImageFloatBox(0.405, 0.720, 0.085, 0.080), // Crate
//                     ImageFloatBox(0.480, 0.120, 0.030, 0.040), // Upper rock
//                     ImageFloatBox(0.575, 0.420, 0.045, 0.080), // Large right rock
//                     ImageFloatBox(0.550, 0.720, 0.030, 0.040)  // Lower small rock
//                 };
//
//                 if (!mine_walk_to_picture(
//                     stream, context, 1.0, 0.0,
//                     "mine_path_10.png", path10_boxes, "[Mine endpoint 10]"
//                 )) {
//                     return false;
//                 }
//
//                 const std::array<ImageFloatBox, 4> path11_boxes = {
//                     ImageFloatBox(0.405, 0.560, 0.070, 0.080),
//                     ImageFloatBox(0.475, 0.270, 0.030, 0.040),
//                     ImageFloatBox(0.575, 0.290, 0.045, 0.080),
//                     ImageFloatBox(0.550, 0.575, 0.030, 0.040)
//                 };
//
//                 if (!mine_walk_to_picture(
//                     stream, context, 0.0, -1.0,
//                     "mine_path_11.png", path11_boxes, "[Mine endpoint 11]"
//                 )) {
//                     return false;
//                 }
//
//                 const std::array<ImageFloatBox, 4> path12_boxes = {
//                     ImageFloatBox(0.260, 0.560, 0.080, 0.080),
//                     ImageFloatBox(0.355, 0.270, 0.025, 0.035),
//                     ImageFloatBox(0.435, 0.290, 0.040, 0.070),
//                     ImageFloatBox(0.415, 0.585, 0.025, 0.035)
//                 };
//
//                 if (!mine_walk_to_picture(
//                     stream, context, 1.0, 0.0,
//                     "mine_path_12.png", path12_boxes, "[Mine endpoint 12]"
//                 )) {
//                     return false;
//                 }
//
//                 const std::array<ImageFloatBox, 4> path13_boxes = {
//                     ImageFloatBox(0.290, 0.230, 0.070, 0.060), // Upper crate
//                     ImageFloatBox(0.285, 0.390, 0.070, 0.060), // Lower crate
//                     ImageFloatBox(0.425, 0.265, 0.025, 0.035), // Upper small rock
//                     ImageFloatBox(0.405, 0.580, 0.030, 0.040)  // Bottom-left small rock
//                 };
//
//                 if (!mine_walk_to_picture(
//                     stream, context, 0.0, -1.0,
//                     "mine_path_13.png", path13_boxes, "[Mine endpoint 13]"
//                 )) {
//                     return false;
//                 }
//
//                 // Turn left, then verify position before starting the interaction.
//                 pbf_move_left_joystick(context, { -1.0, 0.0 }, 40ms, 0ms);
//                 pbf_wait(context, 500ms);
//                 context.wait_for_all_requests();
//
//                 if (!mine_check_picture(
//                     stream, context,
//                     "mine_path_14.png", path13_boxes, "[Mine position 14]", 3
//                 )) {
//                     return false;
//                 }
//                 stream.log("[Mine item] Position confirmed. Pressing A.");
//
//                 pbf_press_button(context, BUTTON_A, 80ms, 200ms);
//                 context.wait_for_all_requests();
//
//                 return mine_finish_item_dialogue(stream, context);
//             }

            static bool oreburgh_mine_09_to_14(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                const std::array<ImageFloatBox, 4> path10_boxes = {
                    ImageFloatBox(0.405, 0.720, 0.085, 0.080), // Crate
                    ImageFloatBox(0.480, 0.120, 0.030, 0.040), // Upper rock
                    ImageFloatBox(0.575, 0.420, 0.045, 0.080), // Large right rock
                    ImageFloatBox(0.550, 0.720, 0.030, 0.040)  // Lower small rock
                };

                const MinePictureStep path10_boxes_step = {
                    "mine_path_10.png", "[Mine endpoint 10]", path10_boxes, 1.0, 0.0,
                    80ms, 30, 20.0, 30.0
                };
                if (!mine_run_picture_step(stream, context, path10_boxes_step)) {
                    return false;
                }

                const std::array<ImageFloatBox, 4> path11_boxes = {
                    ImageFloatBox(0.405, 0.560, 0.070, 0.080),
                    ImageFloatBox(0.475, 0.270, 0.030, 0.040),
                    ImageFloatBox(0.575, 0.290, 0.045, 0.080),
                    ImageFloatBox(0.550, 0.575, 0.030, 0.040)
                };

                const MinePictureStep path11_boxes_step = {
                    "mine_path_11.png", "[Mine endpoint 11]", path11_boxes, 0.0, -1.0,
                    80ms, 30, 20.0, 30.0
                };
                if (!mine_run_picture_step(stream, context, path11_boxes_step)) {
                    return false;
                }

                const std::array<ImageFloatBox, 4> path12_boxes = {
                    ImageFloatBox(0.260, 0.560, 0.080, 0.080),
                    ImageFloatBox(0.355, 0.270, 0.025, 0.035),
                    ImageFloatBox(0.435, 0.290, 0.040, 0.070),
                    ImageFloatBox(0.415, 0.585, 0.025, 0.035)
                };

                const MinePictureStep path12_boxes_step = {
                    "mine_path_12.png", "[Mine endpoint 12]", path12_boxes, 1.0, 0.0,
                    80ms, 30, 20.0, 30.0
                };
                if (!mine_run_picture_step(stream, context, path12_boxes_step)) {
                    return false;
                }

                const std::array<ImageFloatBox, 4> path13_boxes = {
                    ImageFloatBox(0.290, 0.230, 0.070, 0.060), // Upper crate
                    ImageFloatBox(0.285, 0.390, 0.070, 0.060), // Lower crate
                    ImageFloatBox(0.425, 0.265, 0.025, 0.035), // Upper small rock
                    ImageFloatBox(0.405, 0.580, 0.030, 0.040)  // Bottom-left small rock
                };

                const MinePictureStep path13_boxes_step = {
                    "mine_path_13.png", "[Mine endpoint 13]", path13_boxes, 0.0, -1.0,
                    80ms, 30, 20.0, 30.0
                };
                if (!mine_run_picture_step(stream, context, path13_boxes_step)) {
                    return false;
                }

                // Turn left, then verify position before starting the interaction.
                pbf_move_left_joystick(context, { -1.0, 0.0 }, 40ms, 0ms);
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();

                if (!mine_check_picture(
                    stream, context,
                    "mine_path_14.png", path13_boxes, "[Mine position 14]", 3
                )) {
                    return false;
                }
                stream.log("[Mine item] Position confirmed. Pressing A.");

                pbf_press_button(context, BUTTON_A, 80ms, 200ms);
                context.wait_for_all_requests();

                return mine_finish_item_dialogue(stream, context);
            }

            // Compare a reference patch with a shifted live patch using the same
            // brightness adjustment as the native ImageMatchDetector.
            static double mine_offset_patch_score(
                const ImageRGB32& reference,
                const ImageRGB32& live,
                const ImageFloatBox& box,
                int horizontal,
                int vertical
            ) {
                const ImageViewRGB32 reference_crop = extract_box_reference(reference, box);
                const ImageViewRGB32 live_crop =
                    extract_box_reference(live, box, horizontal, vertical);
                ImageRGB32 adjusted = live_crop.scale_to(
                    reference_crop.width(), reference_crop.height()
                );

                const auto reference_average =
                    ImageMatch::pixel_average(reference_crop, reference_crop);
                const auto live_average = ImageMatch::pixel_average(adjusted, reference_crop);
                auto scale = reference_average / live_average;
                if (std::isnan(scale.r)) scale.r = 1.0;
                if (std::isnan(scale.g)) scale.g = 1.0;
                if (std::isnan(scale.b)) scale.b = 1.0;
                scale.bound(0.8, 1.2);
                ImageMatch::scale_brightness(adjusted, scale);

                return ImageMatch::pixel_RMSD(reference_crop, adjusted);
            }

            struct MineRoarkOffsetScore {
                double left = 1000.0;
                double right = 1000.0;
                double helmet = 1000.0;
                int horizontal = 0;
                int vertical = 0;
                int helmet_horizontal = 0;
                int helmet_vertical = 0;
            };

            static MineRoarkOffsetScore mine_score_roark_offsets(
                const ImageRGB32& reference,
                const ImageRGB32& live
            ) {
                const ImageFloatBox left_box(0.415, 0.400, 0.050, 0.095);
                const ImageFloatBox right_box(0.555, 0.400, 0.055, 0.095);
                const ImageFloatBox helmet_box(0.485, 0.335, 0.035, 0.045);

                MineRoarkOffsetScore best;
                double best_worst_rock = 1000.0;
                double best_total = 2000.0;

                // Both rocks must use the SAME offset. They cannot match independently.
                // Offsets are pixels on the 960 x 540 comparison image.
                for (int y = -8; y <= 8; y++) {
                    for (int x = -16; x <= 16; x++) {
                        const double left = mine_offset_patch_score(reference, live, left_box, x, y);
                        const double right = mine_offset_patch_score(reference, live, right_box, x, y);
                        const double worst = left > right ? left : right;
                        const double total = left + right;

                        if (worst < best_worst_rock ||
                            (worst == best_worst_rock && total < best_total)) {
                            best_worst_rock = worst;
                            best_total = total;
                            best.left = left;
                            best.right = right;
                            best.horizontal = x;
                            best.vertical = y;
                        }
                    }
                }

                // Roark's helmet may move slightly relative to the stationary rocks.
                // Keep this extra search small so another red object cannot match far away.
                for (int extra_y = -6; extra_y <= 6; extra_y++) {
                    for (int extra_x = -6; extra_x <= 6; extra_x++) {
                        const double score = mine_offset_patch_score(
                            reference, live, helmet_box,
                            best.horizontal + extra_x,
                            best.vertical + extra_y
                        );
                        if (score < best.helmet) {
                            best.helmet = score;
                            best.helmet_horizontal = extra_x;
                            best.helmet_vertical = extra_y;
                        }
                    }
                }

                return best;
            }

            static bool oreburgh_mine_align_to_18(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                const ImageRGB32 reference(
                    RESOURCE_PATH() +
                    "PokemonBDSP/Autostory/Oreburg_mine_nav/Mine_path_18.png"
                );
                const ImageRGB32 reference_scaled = reference.scale_to(960, 540);
                const double max_rmsd = 65.0;
                size_t movements = 0;

                return mine_run_with_encounters(
                    stream, context,
                    [&](ProControllerContext& ctx) {
                        // Fresh confirmation after any encounter.
                        size_t matching_frames = 0;
                        pbf_wait(ctx, 500ms);
                        ctx.wait_for_all_requests();

                        while (true) {
                            const VideoSnapshot frame = stream.video().snapshot();
                            if (!frame) {
                                stream.log("[Mine endpoint 18] No video frame.", COLOR_ORANGE);
                                return false;
                            }

                            const ImageRGB32 live = frame->scale_to(960, 540);
                            const MineRoarkOffsetScore score =
                                mine_score_roark_offsets(reference_scaled, live);

                            stream.log(
                                "[Mine endpoint 18] Left rock=" + std::to_string(score.left) +
                                ", Right rock=" + std::to_string(score.right) +
                                ", Helmet=" + std::to_string(score.helmet) +
                                ", shared offset=(" + std::to_string(score.horizontal) +
                                "," + std::to_string(score.vertical) +
                                "), helmet extra=(" + std::to_string(score.helmet_horizontal) +
                                "," + std::to_string(score.helmet_vertical) + ")"
                            );

                            if (score.left <= max_rmsd && score.right <= max_rmsd &&
                                score.helmet <= max_rmsd) {
                                if (++matching_frames >= 3) {
                                    stream.log(
                                        "[Mine endpoint 18] Rocks and helmet confirmed.", COLOR_GREEN
                                    );
                                    return true;
                                }
                                pbf_wait(ctx, 250ms);
                                ctx.wait_for_all_requests();
                                continue;
                            }

                            matching_frames = 0;
                            if (movements >= 80) {
                                stream.log("[Mine endpoint 18] Movement limit reached.", COLOR_ORANGE);
                                return false;
                            }

                            const bool approaching = score.left <= 120.0 &&
                                score.right <= 120.0 && score.helmet <= 150.0;
                            const bool near = score.left <= 100.0 &&
                                score.right <= 100.0 && score.helmet <= 100.0;
                            const auto pulse = near ? 20ms : approaching ? 40ms : 60ms;

                            // Use the outer encounter watcher; do not nest mine_timed_walk.
                            // Count issued pulses, including any interrupted by a battle.
                            movements++;
                            pbf_move_left_joystick(ctx, { 1.0, 0.0 }, pulse, 0ms);
                            pbf_wait(ctx, 700ms);
                            ctx.wait_for_all_requests();
                        }
                    }
                );
            }

            /*static bool oreburgh_mine_align_to_18(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                auto reference = std::make_shared<const ImageRGB32>(
                    RESOURCE_PATH() +
                    "PokemonBDSP/Autostory/Oreburg_mine_nav/Mine_path_18.png"
                );

                // Match fixed details around Roark.
                // Keep the player outside these boxes.
                const double max_rmsd = 65.0;

                ImageMatchDetector left_rock(
                    reference,
                    ImageFloatBox(0.415, 0.400, 0.050, 0.095),
                    max_rmsd, true
                );
                ImageMatchDetector right_rock(
                    reference,
                    ImageFloatBox(0.555, 0.400, 0.055, 0.095),
                    max_rmsd, true
                );
                ImageMatchDetector helmet(
                    reference,
                    ImageFloatBox(0.485, 0.335, 0.035, 0.045),
                    max_rmsd, true
                );

                size_t matching_frames = 0;

                // Include a final check after the last allowed movement.
                for (size_t movement = 0; movement <= 80;) {
                    if (!mine_watch_while_checking(stream, context)) {
                        return false;
                    }

                    context.wait_for_all_requests();
                    const VideoSnapshot frame = stream.video().snapshot();

                    if (!frame) {
                        stream.log("[Mine endpoint 18] No video frame.", COLOR_ORANGE);
                        return false;
                    }

                    const double left = left_rock.rmsd(frame);
                    const double right = right_rock.rmsd(frame);
                    const double hat = helmet.rmsd(frame);

                    stream.log(
                        "[Mine endpoint 18] Left rock=" + std::to_string(left) +
                        ", Right rock=" + std::to_string(right) +
                        ", Helmet=" + std::to_string(hat)
                    );

                    if (left <= max_rmsd &&
                        right <= max_rmsd &&
                        hat <= max_rmsd
                        ) {
                        // Stand still while confirming the match.
                        if (++matching_frames >= 3) {
                            stream.log(
                                "[Mine endpoint 18] Rocks and helmet confirmed.",
                                COLOR_GREEN
                            );
                            return true;
                        }

                        pbf_wait(context, 150ms);
                        context.wait_for_all_requests();
                        continue;
                    }

                    matching_frames = 0;

                    if (movement == 80) {
                        stream.log(
                            "[Mine endpoint 18] Movement limit reached. Stopping.",
                            COLOR_ORANGE
                        );
                        return false;
                    }

                    // Small pulses reduce the chance of passing the stopping point.
                    // Use smaller movements when all three landmarks are getting close.
                    // Slow down earlier when the landmarks begin matching.
                    const bool approaching_target =
                        left <= 120.0 &&
                        right <= 120.0 &&
                        hat <= 150.0;

                    const bool near_target =
                        left <= 100.0 &&
                        right <= 100.0 &&
                        hat <= 100.0;

                    const auto pulse = near_target ? 20ms :
                        approaching_target ? 40ms : 60ms;

                    if (!mine_timed_walk(stream, context, 1.0, 0.0, pulse)) {
                        return false;
                    }
                    movement++;
                }

                return false;
            }*/

// Previous route block retained for comparison.
//             static bool oreburgh_mine_14_to_19(
//                 VideoStream& stream,
//                 ProControllerContext& context
//             ) {
//                 const std::array<ImageFloatBox, 4> path15_boxes = {
//                     ImageFloatBox(0.140, 0.220, 0.080, 0.070), // Upper crate
//                     ImageFloatBox(0.125, 0.370, 0.080, 0.070), // Lower crate
//                     ImageFloatBox(0.285, 0.250, 0.035, 0.045), // Small upper rock
//                     ImageFloatBox(0.325, 0.560, 0.035, 0.045)  // Lower rock
//                 };
//
//                 if (!mine_walk_to_picture(
//                     stream, context, 1.0, 0.0,
//                     "mine_path_15_actual.png", path15_boxes, "[Mine endpoint 15]"
//                 )) {
//                     return false;
//                 }
//
//                 // Move below the worker before continuing right.
//                 const std::array<ImageFloatBox, 4> path16_boxes = {
//                     ImageFloatBox(0.145, 0.160, 0.080, 0.070), // Upper crate
//                     ImageFloatBox(0.135, 0.305, 0.080, 0.070), // Lower crate
//                     ImageFloatBox(0.290, 0.195, 0.035, 0.045), // Small upper rock
//                     ImageFloatBox(0.330, 0.495, 0.035, 0.045)  // Lower rock
//                 };
//
//                 if (!mine_walk_to_picture(
//                     stream, context, 0.0, -1.0,
//                     "mine_path_16.png", path16_boxes, "[Mine endpoint 16]"
//                 )) {
//                     return false;
//                 }
//
//                 // Skip 17 and find the actual stopping point at 18.
//                 if (!oreburgh_mine_align_to_18(stream, context)) {
//                     return false;
//                 }
//
//                 // Position 19 is the upward turn toward Roark.
//                 pbf_move_left_joystick(context, { 0.0, 1.0 }, 40ms, 0ms);
//                 pbf_wait(context, 500ms);
//                 context.wait_for_all_requests();
//
//                 stream.log(
//                     "[Mine endpoint 19] Position 18 confirmed; north turn sent.",
//                     COLOR_GREEN
//                 );
//                 return true;
//             }

            static bool oreburgh_mine_14_to_19(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                const std::array<ImageFloatBox, 4> path15_boxes = {
                    ImageFloatBox(0.140, 0.220, 0.080, 0.070), // Upper crate
                    ImageFloatBox(0.125, 0.370, 0.080, 0.070), // Lower crate
                    ImageFloatBox(0.285, 0.250, 0.035, 0.045), // Small upper rock
                    ImageFloatBox(0.325, 0.560, 0.035, 0.045)  // Lower rock
                };

                const MinePictureStep path15_boxes_step = {
                    "mine_path_15_actual.png", "[Mine endpoint 15]", path15_boxes, 1.0, 0.0,
                    80ms, 30, 20.0, 30.0
                };
                if (!mine_run_picture_step(stream, context, path15_boxes_step)) {
                    return false;
                }

                // Move below the worker before continuing right.
                const std::array<ImageFloatBox, 4> path16_boxes = {
                    ImageFloatBox(0.145, 0.160, 0.080, 0.070), // Upper crate
                    ImageFloatBox(0.135, 0.305, 0.080, 0.070), // Lower crate
                    ImageFloatBox(0.290, 0.195, 0.035, 0.045), // Small upper rock
                    ImageFloatBox(0.330, 0.495, 0.035, 0.045)  // Lower rock
                };

                const MinePictureStep path16_boxes_step = {
                    "mine_path_16.png", "[Mine endpoint 16]", path16_boxes, 0.0, -1.0,
                    80ms, 30, 20.0, 30.0
                };
                if (!mine_run_picture_step(stream, context, path16_boxes_step)) {
                    return false;
                }

                // Skip 17 and find the actual stopping point at 18.
                if (!oreburgh_mine_align_to_18(stream, context)) {
                    return false;
                }

                // Position 19 is the upward turn toward Roark.
                pbf_move_left_joystick(context, { 0.0, 1.0 }, 40ms, 0ms);
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();

                stream.log(
                    "[Mine endpoint 19] Position 18 confirmed; north turn sent.",
                    COLOR_GREEN
                );
                return true;
            }

            enum class MineRouteStage {
                ALIGN_ENTRANCE,
                FIRST_ROCK,
                RIGHT_OF_FIRST_ROCK,
                BELOW_FIRST_ROCK,
                CENTER_GAP,
                POSITION_07,
                POSITION_08,
                POSITION_09,
                ITEM_AT_14,
                ROARK_AT_19
            };

            struct MineRouteTestOptions {
                MineRouteStage first = MineRouteStage::ALIGN_ENTRANCE;
                MineRouteStage last = MineRouteStage::ROARK_AT_19;

                // False: stop the test on failure.
                // True: let the checkpoint retry handler reset the game.
                bool reset_on_failure = false;
            };

            static bool oreburgh_mine_first_leg_test(
                VideoStream& stream,
                ProControllerContext& context,
                const MineRouteTestOptions& options = MineRouteTestOptions()
            ) {
                struct Step {
                    const char* name;
                    bool (*run)(VideoStream&, ProControllerContext&);
                };

                const std::array<Step, 10> steps = { {
                    {
                        "Align entrance",
                        oreburgh_mine_alignment_test
                    },
                    {
                        "Walk to first rock",
                        [](VideoStream& video, ProControllerContext& controller) {
                            return oreburgh_mine_walk_to_first_rock(
                                video, controller, false
                            );
                        }
                    },
                    {
                        "Move right of first rock",
                        oreburgh_mine_move_right
                    },
                    {
                        "Move below first rock",
                        oreburgh_mine_down_past_central_rock
                    },
                    {
                        "Move left into center gap",
                        oreburgh_mine_move_left_to_gap
                    },
                    {
                        "Reach position 07",
                        oreburgh_mine_timed_to_07
                    },
                    {
                        "Reach position 08",
                        oreburgh_mine_left_to_08
                    },
                    {
                        "Reach position 09",
                        oreburgh_mine_down_to_09
                    },
                    {
                        "Reach 14 and collect item",
                        oreburgh_mine_09_to_14
                    },
                    {
                        "Reach 18 and turn north for 19",
                        oreburgh_mine_14_to_19
                    }
                } };

                const size_t first = static_cast<size_t>(options.first);
                const size_t last = static_cast<size_t>(options.last);

                if (first > last || last >= steps.size()) {
                    stream.log("[Mine route] Invalid test range.", COLOR_ORANGE);
                    return false;
                }

                for (size_t index = first; index <= last; index++) {
                    const Step& step = steps[index];

                    stream.log(
                        std::string("[Mine route] Starting: ") + step.name
                    );

                    bool succeeded = false;

                    try {
                        succeeded = step.run(stream, context);
                    }
                    catch (OperationFailedException& error) {
                        stream.log(
                            std::string("[Mine route] Failed during ") +
                            step.name + ": " + error.message(),
                            COLOR_ORANGE
                        );

                        if (options.reset_on_failure) {
                            throw;
                        }

                        stream.log(
                            "[Mine route] Test stopped without requesting a reset.",
                            COLOR_ORANGE
                        );
                        return false;
                    }

                    if (!succeeded) {
                        stream.log(
                            std::string("[Mine route] Stage failed: ") + step.name,
                            COLOR_ORANGE
                        );

                        if (options.reset_on_failure) {
                            OperationFailedException::fire(
                                ErrorReport::SEND_ERROR_REPORT,
                                std::string("Mine route failed during: ") + step.name,
                                stream
                            );
                        }

                        return false;
                    }

                    stream.log(
                        std::string("[Mine route] Completed: ") + step.name,
                        COLOR_GREEN
                    );
                }

                stream.log("[Mine route] Selected stages completed.", COLOR_GREEN);
                return true;
            }

            static void oreburgh_mine_nav(
                VideoStream& stream, ProControllerContext& context) {
                walk_through_grass(stream, context, 0, -1, 1, "down into mine");
                context.wait_for_all_requests();
                walk_until_doorway(stream, context, MoveDirection::DOWN, 4350ms);
                context.wait_for_all_requests();
                pbf_wait(context, 1824ms);
                walk_through_grass(stream, context, 0, -1, 8, "down through mine");
                pbf_wait(context, 571ms);
                walk_through_grass(stream, context, 1, 0, 2, "right through mine");
                pbf_wait(context, 504ms);
                walk_through_grass(stream, context, 0, -1, 2, "down through mine");
                pbf_wait(context, 470ms);
                walk_through_grass(stream, context, -1, 0, 2, "left through mine");
                pbf_wait(context, 574ms);
                walk_through_grass(stream, context, 0, -1, 4, "down through mine");
                pbf_wait(context, 440ms);
                walk_through_grass(stream, context, 0, -1, 3, "down through mine");
                pbf_wait(context, 4426ms);

                walk_through_grass(stream, context, 0, -1, 2, "down through mine");
                pbf_wait(context, 672ms);
                walk_through_grass(stream, context, -1, 0, 9, "left through mine");
                pbf_wait(context, 325ms);
                walk_through_grass(stream, context, 0, -1, 4, "down through mine");
                pbf_wait(context, 390ms);
                walk_through_grass(stream, context, -1, 0, 3, "left through mine");
                pbf_wait(context, 205ms);
                walk_through_grass(stream, context, 0, -1, 4, "down through mine");
                pbf_wait(context, 156ms);
                walk_through_grass(stream, context, 1, 0, 4, "right through mine");
                pbf_wait(context, 956ms);
                walk_through_grass(stream, context, 0, 1, 1, "up through mine");
                pbf_wait(context, 1206ms);
                walk_through_grass(stream, context, 1, 0, 7, "right through mine");
                pbf_wait(context, 3473ms);
                walk_through_grass(stream, context, 0, -1, 5, "down through mine");
                pbf_wait(context, 874ms);
                walk_through_grass(stream, context, -1, 0, 1, "left through mine");
                pbf_wait(context, 2590ms);
                pbf_mash_button(context, BUTTON_A, 1000ms);
                walk_through_grass(stream, context, 1, 0, 12, "right through mine");
                pbf_wait(context, 539ms);
                align_between_rocks_and_talk(stream, context);
                pbf_press_button(context, BUTTON_X, 142ms, 0ms);
                pbf_wait(context, 1707ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 162ms, 0ms);
                pbf_wait(context, 457ms);
                pbf_press_button(context, BUTTON_A, 144ms, 0ms);
                pbf_wait(context, 1704ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 159ms, 0ms);
                pbf_wait(context, 640ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 160ms, 0ms);
                pbf_wait(context, 555ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 180ms, 0ms);
                pbf_wait(context, 494ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 171ms, 0ms);
                pbf_wait(context, 826ms);
                pbf_press_button(context, BUTTON_A, 92ms, 0ms);
                pbf_wait(context, 823ms);
                pbf_press_button(context, BUTTON_A, 110ms, 0ms);

            }
            // ---------------------------------------------------------------------------
            //  Segment 1 Part 3: Oreburgh Mine to Gym
            //  Assumes that escape rope was picked up and used to exit the mine.
            //  Navigates to the gym.
            //  ----------------------------------------------------------------------------
            static void oreburgh_mine_to_gym(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 4443ms, 0ms);
                pbf_wait(context, 1165ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 540ms, 0ms);
                pbf_wait(context, 281ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 858ms, 0ms);
                pbf_wait(context, 764ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2972ms, 0ms);
                pbf_wait(context, 677ms);
                pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 160ms, 0ms);
                pbf_press_button(context, BUTTON_UP, 180ms, 0ms);
            }

            // ---------------------------------------------------------------------------
            //  Segment 1 Part 4: Gym navigation
            //  Navigates the player through the first Gym in Oreburgh City and completes the Gym battle.
            // ---------------------------------------------------------------------------

            static void gym_Puzzle_01(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {

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
                pbf_press_button(context, BUTTON_A, 180ms, 0ms);
                wait_for_dialogue(stream, context, "Phase 3 (Roark fight dialog)");
                pbf_mash_button(context, BUTTON_B, 5000ms);
                //Barry 1 battle logic can be used here since the moves don't matter this early.
                Barry_1_fight(env, stream, context);

                //Button presses after battle with small cutscene of badge being put in case between dialogue boxes.

                pbf_mash_button(context, BUTTON_B, 5000ms);
                context.wait_for_all_requests();

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

                AutoStory_checkpoint_0101().run_checkpoint(env, context, options, stats);
                AutoStory_checkpoint_0102().run_checkpoint(env, context, options, stats);
                AutoStory_checkpoint_0103().run_checkpoint(env, context, options, stats);
                context.wait_for_all_requests();
                env.console.log("End Segment " + name(), COLOR_GREEN);
            }


            std::string AutoStory_checkpoint_0101::name()       const { return "01.01 - Maxed Starter from Home"; }
            std::string AutoStory_checkpoint_0101::start_text() const { return "Recieved Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki."; }
            std::string AutoStory_checkpoint_0101::end_text()   const { return "Standing outside the Oreburgh Gate on the Jubilife City side."; }
            // ========================
            // Checkpoint 0101 Wrapper
            // ========================
            void AutoStory_checkpoint_0101::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                checkpoint_0101(env, context, options, stats);
            }


            void checkpoint_0101(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                    [&](size_t /*attempt*/) {
                        jubilife_to_oreburgh_gate(env, env.console, context);
                        oreburgh_gate_nav(env, env.console, context);
                    },
                    true
                );
            }

            // ========================
            // Checkpoint 0102 Wrapper
            // ========================

            std::string AutoStory_checkpoint_0102::name()       const { return "01.02 - Oreburgh Gate"; }
            std::string AutoStory_checkpoint_0102::start_text() const { return AutoStory_checkpoint_0101().end_text(); }
            std::string AutoStory_checkpoint_0102::end_text()   const { return "Standing At the first gym leader after defeating him."; }

            void AutoStory_checkpoint_0102::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                checkpoint_0102(env, context, options, stats);
            }


            void checkpoint_0102(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                    [&](size_t /*attempt*/) {
                        //oreburgh_gate_to_mine(env, env.console, context);
                        //oreburgh_mine_nav(env.console, context);
                        oreburgh_mine_first_leg_test(env.console, context);
                        //oreburgh_mine_to_gym(env, env.console, context);
                        //#. Oreburgh Gym navigation and battle

                    },

                    true
                );
            }

            // ========================
            // Checkpoint 0103 Wrapper
            // ========================

            std::string AutoStory_checkpoint_0103::name()       const { return "01.03 - Oreburgh Gym"; }
            std::string AutoStory_checkpoint_0103::start_text() const { return AutoStory_checkpoint_0102().end_text(); }
            std::string AutoStory_checkpoint_0103::end_text()   const { return "Standing At the first gym leader after defeating him."; }

            void AutoStory_checkpoint_0103::run_checkpoint(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) const {
                checkpoint_0103(env, context, options, stats);
            }


            void checkpoint_0103(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                AutoStoryOptions options,
                AutoStoryStats& stats
            ) {
                checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                    [&](size_t /*attempt*/) {
                        //#. Oreburgh Gym navigation and battle
                        //gym_Puzzle_01(env, env.console, context);

                    },

                    true
                );
            }


        }
    }
}
