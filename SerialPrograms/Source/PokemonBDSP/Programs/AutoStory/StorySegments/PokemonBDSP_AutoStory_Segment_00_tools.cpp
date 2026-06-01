/*  BDSP AutoStory - Segment 00 Tools
 *
 *  Helper implementations for Segment 00.
 * 
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <chrono>
#include "PokemonBDSP_AutoStory_Segment_00_tools.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"

#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"

#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h"

#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"

#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // ---------------------------------------------------------------------------
            // BattleDialogWatcher
            // ---------------------------------------------------------------------------

            BattleDialogWatcher::BattleDialogWatcher(Color color)
                : DetectorToFinder(
                    "BattleDialogWatcher",
                    std::chrono::milliseconds(250),
                    color
                )
            {
            }

            // ---------------------------------------------------------------------------
            // Poketech Clown dialog helper
            // ---------------------------------------------------------------------------

            int detect_clown_choice_arrow(
                VideoOverlay& overlay,
                VideoSnapshot snapshot
            ) {
                SelectionArrowFinder top_arrow(
                    overlay,
                    { 0.62, 0.37, 0.08, 0.10 },
                    COLOR_RED
                );

                SelectionArrowFinder bottom_arrow(
                    overlay,
                    { 0.62, 0.48, 0.08, 0.10 },
                    COLOR_RED
                );

                if (top_arrow.detect(snapshot)) {
                    return 0;
                }

                if (bottom_arrow.detect(snapshot)) {
                    return 1;
                }

                return -1;
            }

            void clown_dialog_helper(
                VideoStream& stream,
                ProControllerContext& context,
                DpadPosition direction
            ) {
                stream.log(
                    "[AutoStory] Starting clown dialog helper...",
                    COLOR_BLUE
                );

                const auto overall_deadline =
                    std::chrono::steady_clock::now() + 30s;

                // -------------------------------------------------
                // Phase 1: Move + interact until dialog is detected.
                // -------------------------------------------------
                ShortDialogWatcher dialog(COLOR_YELLOW);
                bool dialog_started = false;

                while (!dialog_started) {

                    if (std::chrono::steady_clock::now() > overall_deadline) {
                        stream.log(
                            "[AutoStory] Clown helper timed out finding dialog.",
                            COLOR_RED
                        );
                        return;
                    }

                    context.wait_for_all_requests();
                    pbf_press_dpad(context, direction, 80ms, 200ms);
                    pbf_press_button(context, BUTTON_A, 80ms, 500ms);

                    if (wait_until(stream, context, 1500ms, { {dialog} }) == 0) {
                        dialog_started = true;
                        stream.log("[AutoStory] Dialog detected.", COLOR_GREEN);
                    }
                }

                // -------------------------------------------------
                // Phase 2: A mash for 2 seconds.
                // -------------------------------------------------
                stream.log("[AutoStory] Mashing A to advance dialog...", COLOR_BLUE);
                pbf_mash_button(context, BUTTON_A, 4000ms);
                context.wait_for_all_requests();

                // -------------------------------------------------
                // Phase 3: Clear remaining dialog.
                // -------------------------------------------------
                pbf_mash_button(context, BUTTON_B, 4000ms);

                context.wait_for_all_requests();

                stream.log(
                    "[AutoStory] Clown dialog helper complete.",
                    COLOR_GREEN
                );
            }
            // ---------------------------------------------------------------------------
            // Time helper
            // ---------------------------------------------------------------------------

            void set_time_fast(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                const DateTime& target
            ) {
                env.log("[AutoStory] Fast date set starting...", COLOR_ORANGE);

                // 1) Home
                NintendoSwitch::go_home(env.console, context);

                // 2) Navigate to Date/Time settings quickly.
                NintendoSwitch::home_to_date_time(env.console, context, true);

                // 3) Enter date editor.
                pbf_press_button(context, BUTTON_A, 80ms, 240ms);

                // 4) Wait for date screen and set exact time.
                NintendoSwitch::DateChangeWatcher date_reader(env.console);
                int ret = wait_until(
                    env.console, context, 10s,
                    { date_reader }
                );
                if (ret < 0) {
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to enter Date Change screen.",
                        env.console
                    );
                }

                {
                    VideoOverlaySet overlays(env.console.overlay());
                    date_reader.make_overlays(overlays);
                    date_reader.set_date(env.program_info(), env.console, context, target);
                }

                // 5) Commit date.
                pbf_press_button(context, BUTTON_A, 160ms, 240ms);

                // 6) Back to game.
                pbf_press_button(
                    context, BUTTON_HOME, 160ms,
                    ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0
                );
                NintendoSwitch::resume_game_from_home(env.console, context, false);

                context.wait_for_all_requests();
                context.wait_for(500ms);
            }

         
            // ---------------------------------------------------------------------------
            // Natalie battle
            // ---------------------------------------------------------------------------
            void fight_Natalie(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log(
                    "[AutoStory] Starting Natalie battle...",
                    COLOR_BLUE
                );

                // Allow battle transition to finish.
                pbf_wait(context, 5000ms);

                bool battle_menu_seen = false;

                // State machine loop.
                while (true) {

                    context.wait_for_all_requests();

                    BattleMenuWatcher battle_menu(BattleType::TRAINER);
                    EndBattleWatcher end_battle;

                    // Detect move-learning arrows.
                    SelectionArrowFinder learn_move(
                        stream.overlay(),
                        {0.50, 0.62, 0.40, 0.18},
                        COLOR_YELLOW
                    );

                    int ret = run_until<ProControllerContext>(
                        stream,
                        context,
                        [](ProControllerContext& context) {
                            // Continuously clear:
                            // EXP gain
                            // level up
                            // faint text
                            // send-out text
                            // battle narration
                            pbf_mash_button(
                                context,
                                BUTTON_B,
                                120000ms
                            );
                        },
                        {
                            {battle_menu},
                            battle_menu_seen
                                ? PeriodicInferenceCallback{end_battle}
                                : PeriodicInferenceCallback{},
                            {learn_move},
                        }
                    );

                    switch (ret) {

                        // -------------------------------------------------
                        // Battle menu detected.
                        // -------------------------------------------------
                    case 0:
                    {
                        stream.log(
                            "[AutoStory] Battle menu detected.",
                            COLOR_BLUE
                        );

                        battle_menu_seen = true;

                        // Open Fight menu.
                        pbf_press_button(
                            context,
                            BUTTON_A,
                            80ms,
                            500ms
                        );

                        // Hard reset move cursor to top.
                        pbf_press_dpad(
                            context,
                            DPAD_UP,
                            80ms,
                            150ms
                        );

                        pbf_press_dpad(
                            context,
                            DPAD_UP,
                            80ms,
                            150ms
                        );

                        pbf_press_dpad(
                            context,
                            DPAD_UP,
                            80ms,
                            150ms
                        );

                        // Move to move slot 3.
                        pbf_press_dpad(
                            context,
                            DPAD_DOWN,
                            80ms,
                            150ms
                        );

                        pbf_press_dpad(
                            context,
                            DPAD_DOWN,
                            80ms,
                            250ms
                        );

                        // Use move slot 3.
                        pbf_press_button(
                            context,
                            BUTTON_A,
                            80ms,
                            3000ms
                        );

                        break;
                    }

                    // -------------------------------------------------
                    // Battle finished.
                    // -------------------------------------------------
                    case 1:
                    {
                        stream.log(
                            "[AutoStory] Natalie battle complete.",
                            COLOR_GREEN
                        );

                        pbf_mash_button(
                            context,
                            BUTTON_B,
                            5000ms
                        );

                        return;
                    }

                    // -------------------------------------------------
                    // Move learning detected.
                    // -------------------------------------------------
                    case 2:
                    {
                        stream.log(
                            "[AutoStory] Learn move detected.",
                            COLOR_ORANGE
                        );

                        // Always refuse move learning.
                        pbf_move_right_joystick(
                            context,
                            { 0, -1 },
                            160ms,
                            840ms
                        );

                        pbf_press_button(
                            context,
                            BUTTON_A,
                            160ms,
                            840ms
                        );

                        break;
                    }

                    // -------------------------------------------------
                    // Timeout.
                    // -------------------------------------------------
                    default:
                    {
                        stream.log(
                            "[AutoStory] Natalie battle timed out.",
                            COLOR_RED
                        );

                        pbf_mash_button(
                            context,
                            BUTTON_B,
                            5000ms
                        );

                        return;
                    }

                    }
                }
            }

            // ---------------------------------------------------------------------------
            // Starly battle
            // ---------------------------------------------------------------------------

            void fight_starly(VideoStream& stream, ProControllerContext& context) {
                stream.log("[AutoStory] Starting Starly battle...", COLOR_BLUE);

                bool battle_menu_seen = false;

                while (true) {

                    context.wait_for_all_requests();

                    BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                    EndBattleWatcher end_battle;

                    int ret = run_until<ProControllerContext>(
                        stream,
                        context,
                        [](ProControllerContext& ctx) {
                            pbf_mash_button(ctx, BUTTON_B, 120000ms);
                        },
                        {
                            {battle_menu},
                            battle_menu_seen
                                ? PeriodicInferenceCallback{end_battle}
                                : PeriodicInferenceCallback{},
                        }
                    );

                    switch (ret) {

                    case 0:
                    {
                        battle_menu_seen = true;
                        stream.log("[AutoStory] Battle menu detected.", COLOR_BLUE);

                        // Select Fight, then move slot 1.
                        pbf_press_button(context, BUTTON_A, 80ms, 500ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 3000ms);
                        break;
                    }

                    case 1:
                    {
                        stream.log("[AutoStory] Starly battle complete.", COLOR_GREEN);

                        // Mash through post-battle XP / dialog, wait for overworld.
                        OverworldWatcher overworld;
                        run_until<ProControllerContext>(
                            stream, context,
                            [](ProControllerContext& ctx) {
                                pbf_mash_button(ctx, BUTTON_B, 120000ms);
                            },
                            {{overworld}}
                        );
                        return;
                    }

                    default:
                        stream.log("[AutoStory] Starly battle timed out.", COLOR_RED);
                        return;
                    }
                }
            }

            // ---------------------------------------------------------------------------
            // Tristan battle
            // ---------------------------------------------------------------------------
            void fight_tristan(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log(
                    "[AutoStory] Starting Tristan battle...",
                    COLOR_BLUE
                );

                // Wait for battle transition to fully finish.
                pbf_wait(context, 5000ms);

                bool battle_menu_seen = false;

                while (true) {

                    context.wait_for_all_requests();

                    BattleMenuWatcher battle_menu(BattleType::TRAINER);
                    EndBattleWatcher end_battle;

                    int ret = run_until<ProControllerContext>(
                        stream,
                        context,
                        [](ProControllerContext& context) {
                            // Mash through battle text continuously.
                            pbf_mash_button(
                                context,
                                BUTTON_B,
                                10000ms
                            );
                        },
                        {
                            {battle_menu},
                            battle_menu_seen
                                ? PeriodicInferenceCallback{end_battle}
                                : PeriodicInferenceCallback{},
                        }
                    );

                    switch (ret) {

                        // -------------------------------------------------
                        // Battle menu detected.
                        // -------------------------------------------------
                    case 0:
                        battle_menu_seen = true;

                        stream.log(
                            "[AutoStory] Battle menu detected.",
                            COLOR_BLUE
                        );

                        // Select Fight.
                        pbf_press_button(
                            context,
                            BUTTON_A,
                            120ms,
                            500ms
                        );

                        // Select move slot 1 (Scratch).
                        pbf_press_button(
                            context,
                            BUTTON_A,
                            120ms,
                            3000ms
                        );

                        break;

                        // -------------------------------------------------
                        // End battle detected.
                        // -------------------------------------------------
                    case 1:
                        stream.log(
                            "[AutoStory] Tristan battle complete.",
                            COLOR_GREEN
                        );

                        pbf_mash_button(
                            context,
                            BUTTON_B,
                            5000ms
                        );

                        return;

                    default:
                        stream.log(
                            "[AutoStory] Tristan battle timeout.",
                            COLOR_RED
                        );

                        return;
                    }
                }
            }

            // ---------------------------------------------------------------------------
            // Catch 1 Pokemon
            // ---------------------------------------------------------------------------

            void catch_1_pokemon(
                SingleSwitchProgramEnvironment& env,
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log("[AutoStory] Starting catch_1_pokemon helper...", COLOR_BLUE);

                // Save once before attempting to catch. On failure the game
                // resets back to this save, so the save lives outside the loop.
                stream.log("[AutoStory] Saving game before catch attempt...", COLOR_BLUE);
                save_game(stream, context);
                context.wait_for_all_requests();
                pbf_wait(context, 1000ms);

                // -------------------------------------------------------
                // Mini-checkpoint loop: retry from the save above if we
                // run out of balls or our Pokemon faints.
                // -------------------------------------------------------
                while (true) {

                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                bool encounter_found = false;

                // Wiggle left/right until battle starts.
                while (!encounter_found) {

                    // Turn left.
                    pbf_move_left_joystick(context, {-1, 0}, 120ms, 200ms);
                    context.wait_for_all_requests();

                    int ret = wait_until(
                        stream,
                        context,
                        1500ms,
                        {
                            {battle_start},
                            {battle_menu},
                            {battle_dialog},
                        }
                    );

                    if (ret >= 0) {
                        encounter_found = true;
                        break;
                    }

                    // Turn right.
                    pbf_move_left_joystick(context, {1, 0}, 120ms, 200ms);
                    context.wait_for_all_requests();

                    ret = wait_until(
                        stream,
                        context,
                        1500ms,
                        {
                            {battle_start},
                            {battle_menu},
                            {battle_dialog},
                        }
                    );

                    if (ret >= 0) {
                        encounter_found = true;
                        break;
                    }
                }

                stream.log("[AutoStory] Wild battle detected!", COLOR_GREEN);

                // Wait until the actual battle menu appears.
                BattleMenuWatcher confirm_menu(BattleType::STANDARD);

                int menu_ret = wait_until(
                    stream,
                    context,
                    15s,
                    {
                        {confirm_menu},
                    }
                );

                if (menu_ret < 0) {
                    stream.log(
                        "[AutoStory] Failed to detect battle menu. Resetting to checkpoint...",
                        COLOR_RED
                    );
                    reset_game_from_home(env, env.console, context, true);
                    continue;
                }

                stream.log("[AutoStory] Battle menu detected.", COLOR_GREEN);

                context.wait_for_all_requests();
                pbf_wait(context, 500ms);

                // --- Throw balls ---
                CatchResults result = basic_catcher(
                    stream,
                    context,
                    Language::English,
                    "poke-ball",
                    10
                );

                switch (result.result) {
                case CatchResult::POKEMON_CAUGHT:
                    stream.log(
                        "[AutoStory] Pokemon successfully caught after " +
                        std::to_string(result.balls_used) + " ball(s).",
                        COLOR_GREEN
                    );
                    {
                        OverworldWatcher overworld;
                        run_until<ProControllerContext>(
                            stream, context,
                            [](ProControllerContext& ctx) {
                                pbf_mash_button(ctx, BUTTON_B, 120000ms);
                            },
                            {{overworld}}
                        );
                    }
                    return;

                case CatchResult::POKEMON_FAINTED:
                    stream.log("[AutoStory] Wild Pokemon fainted. Exiting battle.", COLOR_ORANGE);
                    {
                        OverworldWatcher overworld;
                        run_until<ProControllerContext>(
                            stream, context,
                            [](ProControllerContext& ctx) {
                                pbf_mash_button(ctx, BUTTON_B, 120000ms);
                            },
                            {{overworld}}
                        );
                    }
                    return;

                case CatchResult::OUT_OF_BALLS:
                    stream.log(
                        "[AutoStory] Ran out of Poke Balls after " +
                        std::to_string(result.balls_used) + " throw(s). Resetting to checkpoint...",
                        COLOR_RED
                    );
                    reset_game_from_home(env, env.console, context, true);
                    continue;

                case CatchResult::OWN_FAINTED:
                    stream.log(
                        "[AutoStory] Own Pokemon fainted. Resetting to checkpoint...",
                        COLOR_RED
                    );
                    reset_game_from_home(env, env.console, context, true);
                    continue;

                case CatchResult::BALL_LIMIT_REACHED:
                    stream.log(
                        "[AutoStory] Ball limit reached (" +
                        std::to_string(result.balls_used) + " throw(s)). Resetting to checkpoint...",
                        COLOR_RED
                    );
                    reset_game_from_home(env, env.console, context, true);
                    continue;

                case CatchResult::CANNOT_THROW_BALL:
                    stream.log(
                        "[AutoStory] Cannot throw ball (menu timing issue). Resetting to checkpoint...",
                        COLOR_RED
                    );
                    reset_game_from_home(env, env.console, context, true);
                    continue;

                default:
                    stream.log(
                        "[AutoStory] Catch timed out after " +
                        std::to_string(result.balls_used) + " ball(s). Resetting to checkpoint...",
                        COLOR_RED
                    );
                    reset_game_from_home(env, env.console, context, true);
                    continue;
                }

                } // end while(true)
            }

            // ---------------------------------------------------------------------------
            // Movement Helpers
            // ---------------------------------------------------------------------------

            // ---------------------------------------------------------------------------
			// Generic Movement Helper - Walk through grass in specified direction for 
            // specified steps, attempting to run from any battles that are encountered.
            // ---------------------------------------------------------------------------
            void walk_through_grass(
                VideoStream& stream,
                ProControllerContext& context,
                int8_t x,
                int8_t y,
                size_t steps,
                const std::string& direction_name
            ) {
                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                for (size_t step = 0; step < steps; step++) {

                    context.wait_for_all_requests();

                    // Move.
                    pbf_move_left_joystick(
                        context,
                        {(double)x, (double)y},
                        200ms,
                        0ms
                    );

                    context.wait_for_all_requests();

                    // Check for battle.
                    int battle_ret = wait_until(
                        stream,
                        context,
                        2500ms,
                        {
                            {battle_start},
                            {battle_menu},
                            {battle_dialog},
                        }
                    );

                    if (battle_ret >= 0) {

                        stream.log(
                            "[AutoStory] Battle detected during movement.",
                            COLOR_ORANGE
                        );

                        // If battle_ret == 0 (black screen transition), wait for the
                        // battle menu to fully appear before attempting to navigate.
                        if (battle_ret == 0) {
                            BattleMenuWatcher wait_for_menu(BattleType::STANDARD, COLOR_YELLOW);
                            wait_until(stream, context, 10s, {{wait_for_menu}});
                            context.wait_for_all_requests();
                        }

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {

                            // Advance any dialog / transition text.
                            pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                            context.wait_for_all_requests();

                            // Wait for the action-selection menu to be visible before navigating.
                            BattleMenuWatcher menu_ready(BattleType::STANDARD, COLOR_YELLOW);
                            if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                                // Menu not yet visible, loop again.
                                continue;
                            }

                            // Navigate to Run. Menu is 1x4 (Fight/Bag/Pokemon/Run) and wraps,
                            // so pressing up once from Fight lands on Run.
                            pbf_move_left_joystick(context, {0, -1}, 80ms, 80ms);

                            // Select Run.
                            pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                            context.wait_for_all_requests();

                            // Watch for escape success or "can't escape" return to menu.
                            EndBattleWatcher battle_end;
                            BattleMenuWatcher back_to_menu(BattleType::STANDARD, COLOR_YELLOW);
                            int run_result = wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}});
                            if (run_result == 0) {
                                escaped = true;
                                break;
                            }
                            // run_result == 1: failed to run, back at battle menu - retry.
                            // run_result == -1: timed out - retry.
                        }

                    if (!escaped) {
                        stream.log(
                            "[AutoStory] Failed to run from battle.",
                            COLOR_RED
                        );
                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);
                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle.",
                            stream
                        );
                    }

                    // Wait until truly back in the overworld before the next step.
                    OverworldWatcher overworld;
                    wait_until(stream, context, 5s, {{overworld}});
                }
            }

            stream.log(
                "[AutoStory] Completed grass movement: " + direction_name,
                COLOR_GREEN
            );
        }

        // ---------------------------------------------------------------------------
        // Helper function for 1st grass patch
        // ---------------------------------------------------------------------------
        void walk_right_until_on_path(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 20; step++) {

                context.wait_for_all_requests();
                
                // Move right (fixed step)
                pbf_move_left_joystick(context, { +1, 0 }, 200ms, 0ms);
                
                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                    );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] Battle detected during movement.",
                        COLOR_ORANGE
                    );

                    if (battle_ret == 0) {
                        BattleMenuWatcher wait_for_menu(BattleType::STANDARD, COLOR_YELLOW);
                        wait_until(stream, context, 10s, {{wait_for_menu}});
                        context.wait_for_all_requests();
                    }

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                        context.wait_for_all_requests();

                        BattleMenuWatcher menu_ready(BattleType::STANDARD, COLOR_YELLOW);
                        if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                            continue;
                        }

                        // Press up once to wrap from Fight to Run.
                        pbf_move_left_joystick(context, { 0, -1 }, 80ms, 80ms);

                        pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        BattleMenuWatcher back_to_menu(BattleType::STANDARD, COLOR_YELLOW);
                        int run_result = wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}});
                        if (run_result == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    if (!escaped) {
                        stream.log("[AutoStory] Failed to run from battle.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Wait until truly back in the overworld before the next step.
                    OverworldWatcher overworld;
                    wait_until(stream, context, 5s, {{overworld}});

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed right movement (18 steps).", COLOR_GREEN);
        }

        // ---------------------------------------------------------------------------
        // Helper function to walk up through the second grass patch
        // ---------------------------------------------------------------------------
        void walk_up_through_grass_2(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 18; step++) {

                context.wait_for_all_requests();

                // Move right (fixed step)
                pbf_move_left_joystick(context, { 0, 1 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                    stream, context,
                    2500ms,
                    {
                        {battle_start},  // 0
                        {battle_menu},   // 1
                        {battle_dialog}, // 2
                    }
                );

                if (battle_ret >= 0) {
                    stream.log(
                        "[AutoStory] Battle detected during movement.",
                        COLOR_ORANGE
                    );

                    if (battle_ret == 0) {
                        BattleMenuWatcher wait_for_menu(BattleType::STANDARD, COLOR_YELLOW);
                        wait_until(stream, context, 10s, {{wait_for_menu}});
                        context.wait_for_all_requests();
                    }

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                        context.wait_for_all_requests();

                        BattleMenuWatcher menu_ready(BattleType::STANDARD, COLOR_YELLOW);
                        if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                            continue;
                        }

                        // Press up once to wrap from Fight to Run.
                        pbf_move_left_joystick(context, { 0, -1 }, 80ms, 80ms);

                        pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        BattleMenuWatcher back_to_menu(BattleType::STANDARD, COLOR_YELLOW);
                        int run_result = wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}});
                        if (run_result == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    if (!escaped) {
                        stream.log("[AutoStory] Failed to run from battle.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving up.",
                            stream
                        );
                    }

                    // Wait until truly back in the overworld before the next step.
                    OverworldWatcher overworld;
                    wait_until(stream, context, 5s, {{overworld}});

                    continue;
                }
            }

            stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
        }
        // ---------------------------------------------------------------------------
        //  Helper function to walk right through the second grass patch
        // ---------------------------------------------------------------------------
        void walk_right_through_grass_2(VideoStream& stream, ProControllerContext& context) {

            BlackScreenWatcher battle_start;
            BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            BattleDialogWatcher battle_dialog(COLOR_YELLOW);

            for (size_t step = 0; step < 10; step++) {

                context.wait_for_all_requests();

                // Move right (fixed step)
                pbf_move_left_joystick(context, { +1, 0 }, 200ms, 0ms);

                context.wait_for_all_requests();

                // Check for battle after movement
                int battle_ret = wait_until(
                        stream, context,
                        2500ms,
                        {
                            {battle_start},  // 0
                            {battle_menu},   // 1
                            {battle_dialog}, // 2
                        }
                        );

                    if (battle_ret >= 0) {
                        stream.log(
                            "[AutoStory] Battle detected during movement.",
                            COLOR_ORANGE
                        );

                        if (battle_ret == 0) {
                            BattleMenuWatcher wait_for_menu(BattleType::STANDARD, COLOR_YELLOW);
                            wait_until(stream, context, 10s, {{wait_for_menu}});
                            context.wait_for_all_requests();
                        }

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {
                            pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                            context.wait_for_all_requests();

                            BattleMenuWatcher menu_ready(BattleType::STANDARD, COLOR_YELLOW);
                            if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                                continue;
                            }

                            // Press up once to wrap from Fight to Run.
                            pbf_move_left_joystick(context, { 0, -1 }, 80ms, 80ms);

                            pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            BattleMenuWatcher back_to_menu(BattleType::STANDARD, COLOR_YELLOW);
                            int run_result = wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}});
                            if (run_result == 0) {
                                escaped = true;
                                break;
                            }
                        }

                        if (!escaped) {
                            stream.log("[AutoStory] Failed to run from battle.", COLOR_RED);

                            pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                            pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                            pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                            throw OperationFailedException(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to run from battle while moving right.",
                                stream
                            );
                        }

                        // Wait until truly back in the overworld before the next step.
                        OverworldWatcher overworld;
                        wait_until(stream, context, 5s, {{overworld}});

                        continue;
                    }
                }

                stream.log("[AutoStory] Completed fixed right movement (10 steps).", COLOR_GREEN);
            }
            // ---------------------------------------------------------------------------
            // Helper function to walk left through the third grass patch
            // ---------------------------------------------------------------------------
            void walk_left_through_grass_3(VideoStream& stream, ProControllerContext& context) {

                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                for (size_t step = 0; step < 18; step++) {

                    context.wait_for_all_requests();

                    // Move left (fixed step)
                    pbf_move_left_joystick(context, { -1, 0 }, 200ms, 0ms);

                    context.wait_for_all_requests();

                    // Check for battle after movement
                    int battle_ret = wait_until(
                        stream, context,
                        2500ms,
                        {
                            {battle_start},  // 0
                            {battle_menu},   // 1
                            {battle_dialog}, // 2
                        }
                        );

                    if (battle_ret >= 0) {
                        stream.log(
                            "[AutoStory] Battle detected during movement.",
                            COLOR_ORANGE
                        );

                        if (battle_ret == 0) {
                            BattleMenuWatcher wait_for_menu(BattleType::STANDARD, COLOR_YELLOW);
                            wait_until(stream, context, 10s, {{wait_for_menu}});
                            context.wait_for_all_requests();
                        }

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {
                            pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                            context.wait_for_all_requests();

                            BattleMenuWatcher menu_ready(BattleType::STANDARD, COLOR_YELLOW);
                            if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                                continue;
                            }

                            // Press up once to wrap from Fight to Run.
                            pbf_move_left_joystick(context, { 0, -1 }, 80ms, 80ms);

                            pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            BattleMenuWatcher back_to_menu(BattleType::STANDARD, COLOR_YELLOW);
                            int run_result = wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}});
                            if (run_result == 0) {
                                escaped = true;
                                break;
                            }
                        }

                        if (!escaped) {
                            stream.log("[AutoStory] Failed to run from battle.", COLOR_RED);

                            pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                            pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                            pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                            throw OperationFailedException(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to run from battle while moving left.",
                                stream
                            );
                        }

                        // Wait until truly back in the overworld before the next step.
                        OverworldWatcher overworld;
                        wait_until(stream, context, 5s, {{overworld}});

                        continue;
                    }
                }

                stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
            }
            // ---------------------------------------------------------------------------
            // Helper function to walk up through the third grass patch
            // ---------------------------------------------------------------------------
            void walk_up_through_grass_3(VideoStream& stream, ProControllerContext& context) {

                BlackScreenWatcher battle_start;
                BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                for (size_t step = 0; step < 18; step++) {

                    context.wait_for_all_requests();

                    // Move up (fixed step)
                    pbf_move_left_joystick(context, { 0, 1 }, 200ms, 0ms);

                    context.wait_for_all_requests();

                    // Check for battle after movement
                    int battle_ret = wait_until(
                        stream, context,
                        2500ms,
                        {
                            {battle_start},  // 0
                            {battle_menu},   // 1
                            {battle_dialog}, // 2
                        }
                        );

                    if (battle_ret >= 0) {
                        stream.log(
                            "[AutoStory] Battle detected during movement.",
                            COLOR_ORANGE
                        );

                        if (battle_ret == 0) {
                            BattleMenuWatcher wait_for_menu(BattleType::STANDARD, COLOR_YELLOW);
                            wait_until(stream, context, 10s, {{wait_for_menu}});
                            context.wait_for_all_requests();
                        }

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {
                            pbf_press_button(context, BUTTON_B, 40ms, 200ms);
                            context.wait_for_all_requests();

                            BattleMenuWatcher menu_ready(BattleType::STANDARD, COLOR_YELLOW);
                            if (wait_until(stream, context, 3s, {{menu_ready}}) != 0) {
                                continue;
                            }

                            // Press up once to wrap from Fight to Run.
                            pbf_move_left_joystick(context, { 0, -1 }, 80ms, 80ms);

                            pbf_press_button(context, BUTTON_A, 60ms, 0ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            BattleMenuWatcher back_to_menu(BattleType::STANDARD, COLOR_YELLOW);
                            int run_result = wait_until(stream, context, 4s, {{battle_end}, {back_to_menu}});
                            if (run_result == 0) {
                                escaped = true;
                                break;
                            }
                        }

                        if (!escaped) {
                            stream.log("[AutoStory] Failed to run from battle.", COLOR_RED);

                            pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                            pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                            pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                            throw OperationFailedException(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to run from battle while moving up.",
                                stream
                            );
                        }

                        // Wait until truly back in the overworld before the next step.
                        OverworldWatcher overworld;
                        wait_until(stream, context, 5s, {{overworld}});

                        continue;
                    }
                }

                stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
            }
        }
    }
}
