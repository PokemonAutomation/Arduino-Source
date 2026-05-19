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

#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"

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
                stream.log("Starting Starly battle...");

                // Give time for battle transition
                pbf_wait(context, 5000ms);

                // Mash A to get through intro dialogue
                pbf_mash_button(context, BUTTON_A, 5000ms);

                // Use first move repeatedly (Tackle/Scratch/Pound)
                for (int i = 0; i < 6; i++) {
                    pbf_press_button(context, BUTTON_A, 100ms, 2000ms); // Select move
                    pbf_wait(context, 3000ms); // Wait for animation + damage
                }

                // Mash through faint + post-battle dialogue
                pbf_mash_button(context, BUTTON_A, 8000ms);

                stream.log("Starly battle complete.");

                wait_for_dialogue(stream, context, "Phase 9 (post battle text)");

                pbf_mash_button(context, BUTTON_B, 20000ms);

                wait_for_dialogue(stream, context, "Phase 9 (post battle text)");

                pbf_mash_button(context, BUTTON_B, 5000ms);

                context.wait_for_all_requests();


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
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log("[AutoStory] Starting catch_1_pokemon helper...", COLOR_BLUE);

                context.wait_for_all_requests();
                pbf_wait(context, 1000ms);

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
                        "[AutoStory] Failed to detect battle menu.",
                        COLOR_RED
                    );
                    return;
                }

                stream.log("[AutoStory] Battle menu detected.", COLOR_GREEN);

                context.wait_for_all_requests();
                pbf_wait(context, 500ms);

                // Catch the pokemon.
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
                        "[AutoStory] Pokemon successfully caught.",
                        COLOR_GREEN
                    );
                    break;

                default:
                    stream.log(
                        "[AutoStory] Catch attempt failed.",
                        COLOR_ORANGE
                    );
                    break;
                }

                context.wait_for_all_requests();

                // Force clear any remaining text boxes / summaries.
                pbf_mash_button(context, BUTTON_B, 15000ms);

                context.wait_for_all_requests();
                pbf_wait(context, 2000ms);
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

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {

                            // Advance text.
                            pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                            context.wait_for_all_requests();

                            // Move cursor to Run.
                            pbf_move_left_joystick(context, {0, 1}, 80ms, 80ms);

                            // Select Run.
                            pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            if (wait_until(stream, context, 1500ms, {{battle_end}}) == 0) {
                                escaped = true;
                                break;
                            }
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

                    // Let overworld stabilize.
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);
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
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                        // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

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
                        "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                        COLOR_ORANGE
                    );

                    bool escaped = false;
                    auto deadline = std::chrono::steady_clock::now() + 30s;

                    while (std::chrono::steady_clock::now() < deadline) {
                    // Spam escape inputs
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        EndBattleWatcher battle_end;
                        if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                            escaped = true;
                            break;
                        }
                    }

                    // Light text advance
                    pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                    context.wait_for_all_requests();

                    // Reposition cursor on "Run"
                    pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                    pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                    context.wait_for_all_requests();

                    if (!escaped) {
                        stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                        pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                        pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT,
                            "Failed to run from battle while moving right.",
                            stream
                        );
                    }

                    // Critical: let overworld stabilize before next step
                    context.wait_for_all_requests();
                    context.wait_for(1000ms);

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
                            "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                            COLOR_ORANGE
                        );

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {
                            // Spam escape inputs
                            pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                            context.wait_for_all_requests();

                            pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                            pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                                escaped = true;
                                break;
                            }
                        }

                        // Light text advance
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        // Reposition cursor on "Run"
                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        if (!escaped) {
                            stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                            pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                            pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                            pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                            throw OperationFailedException(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to run from battle while moving right.",
                                stream
                            );
                        }

                        // Critical: let overworld stabilize before next step
                        context.wait_for_all_requests();
                        context.wait_for(1000ms);

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
                            "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                            COLOR_ORANGE
                        );

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {
                            // Spam escape inputs
                            pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                            context.wait_for_all_requests();

                            pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms); // down
                            pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                                escaped = true;
                                break;
                            }
                        }

                        // Light text advance
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        // Reposition cursor on "Run"
                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        if (!escaped) {
                            stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                            pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                            pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                            pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                            throw OperationFailedException(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to run from battle while moving right.",
                                stream
                            );
                        }

                        // Critical: let overworld stabilize before next step
                        context.wait_for_all_requests();
                        context.wait_for(1000ms);

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
                            "[AutoStory] battle detected during movement. ret = " + std::to_string(battle_ret),
                            COLOR_ORANGE
                        );

                        bool escaped = false;
                        auto deadline = std::chrono::steady_clock::now() + 30s;

                        while (std::chrono::steady_clock::now() < deadline) {
                            // Spam escape inputs
                            pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                            context.wait_for_all_requests();

                            pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                            pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                            context.wait_for_all_requests();

                            EndBattleWatcher battle_end;
                            if (wait_until(stream, context, 1500ms, { {battle_end} }) == 0) {
                                escaped = true;
                                break;
                            }
                        }

                        // Light text advance
                        pbf_press_button(context, BUTTON_B, 40ms, 120ms);
                        context.wait_for_all_requests();

                        // Reposition cursor on "Run"
                        pbf_move_left_joystick(context, { 0, 1 }, 80ms, 80ms);
                        pbf_press_button(context, BUTTON_A, 60ms, 180ms);
                        context.wait_for_all_requests();

                        if (!escaped) {
                            stream.log("[AutoStory] run_from_battle failed. Exiting to HOME.", COLOR_RED);

                            pbf_press_button(context, BUTTON_HOME, 80ms, 1200ms);
                            pbf_press_button(context, BUTTON_X, 80ms, 300ms);
                            pbf_press_button(context, BUTTON_A, 80ms, 1200ms);

                            throw OperationFailedException(
                                ErrorReport::SEND_ERROR_REPORT,
                                "Failed to run from battle while moving right.",
                                stream
                            );
                        }

                        // Critical: let overworld stabilize before next step
                        context.wait_for_all_requests();
                        context.wait_for(1000ms);

                        continue;
                    }
                }

                stream.log("[AutoStory] Completed fixed up movement (18 steps).", COLOR_GREEN);
            }
        }
    }
}
