/*  BDSP AutoStory - Segment 00
 *  From player's bedroom to pokedex acquisition.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonBDSP_AutoStory_Segment_00.h"
#include "PokemonBDSP_AutoStory_Segment_00_tools.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"

#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_StarterReset.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"

#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {
            // ---------------------------------------------------------------------------
            // Starter selection
            // ---------------------------------------------------------------------------

            static void select_turtwig(VideoStream& stream, ProControllerContext& context) {
                stream.log("[DEBUG] Turtwig Selection: Starting starter selection");

                pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
                context.wait_for_all_requests();

                ShortDialogWatcher watcher(COLOR_CYAN);
                int ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                    { watcher }
                );
                if (ret != 0) {
                    ShortDialogWatcher watcher_red(COLOR_RED);
                    ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                        { watcher_red }
                    );
                }
                stream.log(ret == 0 ? "[DEBUG] Turtwig Selection: Confirmation dialogue detected"
                    : "[WARNING] Turtwig Selection: Dialogue detection failed, proceeding anyway");

                pbf_wait(context, 400ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
                pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
                context.wait_for_all_requests();
                stream.log("[DEBUG] Turtwig Selection: Selection complete");
            }

            static void select_chimchar(VideoStream& stream, ProControllerContext& context) {
                stream.log("[DEBUG] Chimchar Selection: Starting starter selection");

                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
                pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
                context.wait_for_all_requests();

                ShortDialogWatcher watcher(COLOR_CYAN);
                int ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                    { watcher }
                );
                if (ret != 0) {
                    ShortDialogWatcher watcher_red(COLOR_RED);
                    ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                        { watcher_red }
                    );
                }
                stream.log(ret == 0 ? "[DEBUG] Chimchar Selection: Confirmation dialogue detected"
                    : "[WARNING] Chimchar Selection: Dialogue detection failed, proceeding anyway");

                pbf_wait(context, 400ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
                pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
                context.wait_for_all_requests();
                stream.log("[DEBUG] Chimchar Selection: Selection complete");
            }

            static void select_piplup(VideoStream& stream, ProControllerContext& context) {
                stream.log("[DEBUG] Piplup Selection: Starting starter selection");

                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 840ms);
                pbf_press_button(context, BUTTON_ZL, 160ms, 240ms);
                context.wait_for_all_requests();

                ShortDialogWatcher watcher(COLOR_CYAN);
                int ret = run_until<ProControllerContext>(
                    stream, context,
                    [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                    { watcher }
                );
                if (ret != 0) {
                    ShortDialogWatcher watcher_red(COLOR_RED);
                    ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& ctx) { ctx.wait_for(std::chrono::milliseconds(5000)); },
                        { watcher_red }
                    );
                }
                stream.log(ret == 0 ? "[DEBUG] Piplup Selection: Confirmation dialogue detected"
                    : "[WARNING] Piplup Selection: Dialogue detection failed, proceeding anyway");

                pbf_wait(context, 400ms);
                pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
                pbf_press_button(context, BUTTON_ZL, 80ms, 5000ms);
                context.wait_for_all_requests();
                stream.log("[DEBUG] Piplup Selection: Selection complete");
            }

        // Navigation functions for each part of the tutorial. These functions are meant to be used
        // in order and rely on the state of the game being what it is at the end of the previous function.
        // They may also be modified or removed without warning.

        // ---------------------------------------------------------------------------
        // Segment 00-000 begins
        // Navigates from player's room to Route 201 through Mom/Barry dialogues.
        // ---------------------------------------------------------------------------

        static void tutorial_part_1_navigation(SingleSwitchProgramEnvironment& env, 
            VideoStream& stream, ProControllerContext& context) {
            DateTime t{ 2026, 1, 1, 0, 0, 0 };
            set_time_fast(env, context, t);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 527ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, 1.000000 }, 456ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 915ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, 1.000000 }, 642ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 557ms, 0ms);
            pbf_wait(context, 978ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1112ms, 0ms);
            pbf_wait(context, 4967ms);

            wait_for_dialogue(stream, context, "Phase 1 (Mom)");

            pbf_mash_button(context, BUTTON_A, 5000ms);
            pbf_wait(context, 300ms);

            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1662ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -1.000000 }, 49ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1215ms, 0ms);
            pbf_wait(context, 1419ms);

            wait_for_dialogue(stream, context, "Phase 2 (Mom Warning)");

            pbf_mash_button(context, BUTTON_A, 2500ms);
            pbf_wait(context, 300ms);
            context.wait_for_all_requests();

            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 680ms, 0ms);

            pbf_wait(context, 4000ms);

            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1748ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 18ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2763ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 58ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1364ms, 0ms);
            pbf_wait(context, 4000ms);
            context.wait_for_all_requests();

            wait_for_dialogue(stream, context, "Phase 3 (Barry 1st)");
            pbf_mash_button(context, BUTTON_A, 6000ms);
            pbf_wait(context, 800ms);
            wait_for_dialogue(stream, context, "Phase 3 (Barry 1st)");
            pbf_mash_button(context, BUTTON_A, 6000ms);
            pbf_wait(context, 800ms);

            pbf_wait(context, 1306ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1000ms, 0ms);
			pbf_wait(context, 1422ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2722ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1600ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 501ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 652ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1817ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 975ms, 0ms);
            pbf_wait(context, 1422ms);

            wait_for_dialogue(stream, context, "Phase 8 (Barry 2nd dialog)");
            pbf_mash_button(context, BUTTON_A, 10000ms);

            pbf_wait(context, 856ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 920ms, 0ms);
            pbf_wait(context, 631ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 2183ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1076ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 781ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 2713ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 5861ms, 0ms);
            pbf_wait(context, 134ms);

            wait_for_dialogue(stream, context, "Phase 9 (Barry Route 201)");

            pbf_mash_button(context, BUTTON_B, 6000ms);
            pbf_wait(context, 800ms);

            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 700ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 279ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 0.000000 }, 10ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 145ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 0.000000 }, 7703ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 140ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1700ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 39ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 0.000000 }, 544ms, 1000ms);

            pbf_wait(context, 1000ms);

            save_game(stream, context);
            context.wait_for_all_requests();
        }
        // ---------------------------------------------------------------------------
        //  Segment 00-001 begins
        //  Navigates into lake Verity to recieve starter
        // ---------------------------------------------------------------------------
        static void starter_nav(VideoStream& stream, ProControllerContext& context) {
            // re-sync control
            pbf_press_button(context, BUTTON_B, 20ms, 200ms);
            pbf_wait(context, 2000ms);
            context.wait_for_all_requests();

            // continue movement
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 300ms, 0ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 300ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 10 (Barry before the starter selection)");

            pbf_mash_button(context, BUTTON_A, 1000ms);

            pbf_wait(context, 500ms);
            std::shared_ptr<const ImageRGB32> briefcase =
                std::make_shared<const ImageRGB32>(
                    RESOURCE_PATH() + "PokemonBDSP/StarterBriefcase.png"
                );

            //  Mash B until we see the briefcase.
            ImageMatchWatcher detector(briefcase, { 0.5, 0.1, 0.5, 0.7 }, 100, true);

            int ret = run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& context) {
                    pbf_mash_button(context, BUTTON_B, 120000ms);
                },
                { {detector} }
            );

            if (ret == 0) {
                stream.log("Detected briefcase!");
            }
            else {
                stream.log("Timed out waiting for briefcase.");
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to detect briefcase in Segment 00.",
                    stream
                );
            }

            //  Wait for briefcase to fully open.
            stream.log("Mashing B for briefcase to fully open.");
            pbf_mash_button(context, BUTTON_B, 2000ms);
        }
        static void post_starly_clenaup(VideoStream& stream, ProControllerContext& context) {
            pbf_mash_button(context, BUTTON_B, 60000ms);
        }
        // ---------------------------------------------------------------------------
        //  Segment 00-002 begins
        //  Navigates from Exit of lake Verity to receipt of the Pokedex
        // ---------------------------------------------------------------------------

        static void tutorial_part_2_navigation(SingleSwitchProgramEnvironment& env, 
            VideoStream& stream, ProControllerContext& context) {
            pbf_wait(context, 2000ms);
            context.wait_for_all_requests();

            pbf_wait(context, 563ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1700ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 510ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 50ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 310ms, 0ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 5500ms, 0ms);
            pbf_wait(context, 3963ms);
            context.wait_for_all_requests();

            wait_for_dialogue(stream, context, "Phase 10 (Professor dialog to mom)");
            pbf_mash_button(context, BUTTON_B, 40000ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 820ms, 0ms);
            pbf_wait(context, 4000ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 800ms, 0ms);
            pbf_wait(context, 555ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 50ms, 0ms);
            pbf_wait(context, 23ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 5020ms, 0ms);
            pbf_wait(context, 169ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 745ms, 0ms);
            DateTime t{ 2026, 1, 1, 0, 0, 50 };
            set_time_fast(env, context, t);
            walk_right_until_on_path(stream, context, BattleType::STARTER);
            walk_up_through_grass_2(stream, context, BattleType::STARTER);
            walk_right_through_grass_2(stream, context, BattleType::STARTER);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 730ms, 0ms);
            pbf_wait(context, 941ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1950ms, 0ms);
            wait_for_dialogue(stream, context, "Phase 11 (dawn conversation)");
            pbf_mash_button(context, BUTTON_B, 50000ms);
            wait_for_dialogue(stream, context, "Phase 12 (Nickname reject)");
            pbf_mash_button(context, BUTTON_B, 40000ms);
            wait_for_dialogue(stream, context, "Phase 13 (Pokedex accept)");
            pbf_mash_button(context, BUTTON_A, 40000ms);


            // pokedex received
        }


        // ---------------------------------------------------------------------------
        // End of Segment 00 - 002
        // Standing in front of the professor after receiving the pokedex.
        // ---------------------------------------------------------------------------

        // ---------------------------------------------------------------------------
        // Begin Segment 00 - 003
        // Will heal at Sandgem pokeCenter to reliably catch a second pokemon for Home
        // transfer.
        // Collects poketech and stands in front of the PC to end the tutorial section
        // and send the starter to pokemon HOME.
        // ---------------------------------------------------------------------------

        static void tutorial_part_3_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
            /*pbf_move_left_joystick(context, {0.000000, -1.000000}, 2010ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 14 (dawn tour)");

            pbf_mash_button(context, BUTTON_B, 25000ms);

            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 5125ms, 0ms);
            pbf_wait(context, 336ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1700ms, 0ms);
            pbf_wait(context, 213ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2325ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, 1.000000 }, 950ms, 0ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2360ms, 0ms);
            pbf_wait(context, 253ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 4610ms, 0ms);
            pbf_wait(context, 954ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 220ms, 0ms);
            pbf_wait(context, 587ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1375ms, 0ms);
            pbf_wait(context, 464ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 775ms, 0ms);
            pbf_wait(context, 1655ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 640ms, 0ms);
            pbf_wait(context, 3128ms);
            pbf_press_dpad(context, DPAD_UP, 180ms, 0ms);
            pbf_wait(context, 472ms);
            pbf_press_dpad(context, DPAD_UP, 130ms, 0ms);
            pbf_wait(context, 1621ms);
            pbf_press_dpad(context, DPAD_RIGHT, 130ms, 0ms);
            pbf_press_button(context, BUTTON_A, 180ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 15 (Mom conversation to accept parcel)");

            pbf_mash_button(context, BUTTON_B, 40000ms);

            context.wait_for_all_requests();
            pbf_wait(context, 1500ms);
            pbf_press_dpad(context, DPAD_LEFT, 150ms, 0ms);
            pbf_wait(context, 1143ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 875ms, 0ms);
            pbf_wait(context, 1169ms);
            pbf_press_button(context, BUTTON_A, 180ms, 0ms);
            pbf_wait(context, 946ms);
            pbf_press_button(context, BUTTON_A, 180ms, 0ms);
            pbf_wait(context, 9092ms);
            pbf_press_button(context, BUTTON_A, 180ms, 0ms);
            pbf_wait(context, 740ms);
            pbf_press_button(context, BUTTON_A, 180ms, 0ms);
            pbf_wait(context, 2998ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 240ms, 0ms);
            pbf_wait(context, 893ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 5300ms, 0ms);
            pbf_wait(context, 282ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 260ms, 0ms);
            context.wait_for_all_requests();
            pbf_wait(context, 1500ms);

            DateTime t{ 2026, 1, 1, 0, 0, 50 };
            set_time_fast(env, context, t);
            walk_right_until_on_path(stream, context, BattleType::STARTER);
            walk_up_through_grass_2(stream, context, BattleType::STARTER);
            walk_right_through_grass_2(stream, context, BattleType::STARTER);
            context.wait_for_all_requests();
            pbf_wait(context, 1500ms);

            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 560ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 5000ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1000ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1855ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 430ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 850ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1120ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 16 (catch tutorial)");

            pbf_mash_button(context, BUTTON_B, 70000ms);
            context.wait_for_all_requests();
            pbf_wait(context, 2000ms);*/
            catch_1_pokemon(stream, context);
            pbf_wait(context, 1500ms);
            context.wait_for_all_requests();
            save_game(stream, context);
            context.wait_for_all_requests();
            pbf_wait(context, 1500ms);
            //--------------------------------------------------------
            // To get potions
            // -------------------------------------------------------
            walk_right_through_grass_2(stream, context);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1421ms, 0ms);
            pbf_wait(context, 52ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1013ms, 0ms);
            pbf_wait(context, 142ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1112ms, 0ms);
            pbf_wait(context, 45ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 3206ms, 0ms);
            pbf_wait(context, 912ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 856ms, 0ms);
            context.wait_for_all_requests();
            pbf_wait(context, 1500ms);
            walk_left_through_grass_3(stream, context);
            pbf_press_button(context, BUTTON_A, 180ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 17 (accept potions)");

            pbf_mash_button(context, BUTTON_B, 10000ms);

            walk_right_through_grass_2(stream, context);
            context.wait_for_all_requests();
            pbf_wait(context, 1500ms);

            // ---------------------------------------------------------
            // Navigate back to catch tutorial grass
            // ---------------------------------------------------------
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 560ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 5000ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1000ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1855ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 430ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 850ms, 0ms);
            pbf_wait(context, 1500ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1120ms, 0ms);

            // Continue on to jubilife
            walk_left_through_grass_3(stream, context);
            walk_up_through_grass_3(stream, context);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1000ms, 0ms);
            wait_for_dialogue(stream, context, "Phase 18 (tristan encounter dialog)");

            pbf_mash_button(context, BUTTON_B, 5000ms);
            fight_tristan(stream, context);

            use_potion_first_pokemon(stream, context);
            context.wait_for_all_requests();
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1366ms, 0ms);
            pbf_wait(context, 467ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1018ms, 0ms);
            pbf_wait(context, 311ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 353ms, 0ms);
            pbf_wait(context, 660ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 580ms, 0ms);
            pbf_wait(context, 1672ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1344ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 19 (natalie encounter dialog)");

            pbf_mash_button(context, BUTTON_B, 5000ms);

            fight_Natalie(stream, context);
            context.wait_for_all_requests();
            use_potion_first_pokemon(stream, context);
            context.wait_for_all_requests();

            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1301ms, 0ms);
            walk_through_grass(stream, context, 0, 1, 15, "up through grass");
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 392ms, 0ms);
            pbf_wait(context, 1261ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 815ms, 0ms);

            wait_for_dialogue(stream, context, "Phase 20 (logan encounter dialog)");

            pbf_mash_button(context, BUTTON_B, 5000ms);
            fight_Natalie(stream, context);
            context.wait_for_all_requests();
            use_potion_first_pokemon(stream, context);
            context.wait_for_all_requests();
            walk_through_grass(stream, context, -1, 0, 8, "left through grass");

            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2469ms, 0ms);
            wait_for_dialogue(stream, context, "Phase 21 (dawn Jubilife dialog)");

            pbf_mash_button(context, BUTTON_B, 5000ms);
        }
        // ---------------------------------------------------------------------------
        // End of Segment 00 - 003
        // Standing in Jubilife after speaking to dawn.
        // ---------------------------------------------------------------------------
        // ---------------------------------------------------------------------------
        // Begin Segment 00 - 004
        // Standing at the entrance to jubilife after speaking to dawn, will navigate to the poketech challenge.
        // Collects poketech and stands in front of the PC to end the tutorial section
        // and send the starter to pokemon HOME.
        // ---------------------------------------------------------------------------

        static void tutorial_part_4_navigation(SingleSwitchProgramEnvironment& env, VideoStream& stream, ProControllerContext& context) {
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2580ms, 0ms);
            pbf_wait(context, 504ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 830ms, 0ms);
            pbf_wait(context, 617ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 800ms, 0ms);
            pbf_wait(context, 509ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 2000ms, 0ms);
            pbf_wait(context, 598ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 200ms, 0ms);
            pbf_wait(context, 581ms);
            pbf_press_button(context, BUTTON_A, 150ms, 0ms);
            pbf_wait(context, 795ms);

            wait_for_dialogue(stream, context, "Phase 22 (barry town map dialog)");

            pbf_mash_button(context, BUTTON_B, 15000ms);
            context.wait_for_all_requests();
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1521ms, 0ms);
            pbf_wait(context, 602ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1100ms, 0ms);
            pbf_wait(context, 864ms);
            pbf_press_dpad(context, DPAD_RIGHT, 138ms, 0ms);
            pbf_wait(context, 2398ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1018ms, 0ms);
            pbf_wait(context, 3811ms);

            wait_for_dialogue(stream, context, "Phase 23 (poketech challenge dialog)");

            pbf_mash_button(context, BUTTON_B, 5000ms);
            context.wait_for_all_requests();

            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 192ms, 0ms);
            pbf_wait(context, 820ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 980ms, 0ms);
            pbf_wait(context, 543ms);
            pbf_press_dpad(context, DPAD_UP, 203ms, 0ms);
            pbf_wait(context, 593ms);
            pbf_press_dpad(context, DPAD_UP, 115ms, 0ms);
            pbf_wait(context, 566ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1567ms, 0ms);
            pbf_wait(context, 538ms);
            pbf_press_dpad(context, DPAD_LEFT, 157ms, 0ms);
            pbf_wait(context, 735ms);
            pbf_press_dpad(context, DPAD_LEFT, 164ms, 0ms);
            pbf_wait(context, 564ms);
            pbf_press_dpad(context, DPAD_LEFT, 250ms, 0ms);
            pbf_wait(context, 513ms);
            pbf_press_dpad(context, DPAD_LEFT, 250ms, 0ms);
            pbf_wait(context, 513ms);
            pbf_press_dpad(context, DPAD_DOWN, 163ms, 0ms);
            pbf_wait(context, 609ms);
            pbf_press_dpad(context, DPAD_DOWN, 193ms, 0ms);
            pbf_wait(context, 584ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 151ms, 0ms);
            context.wait_for_all_requests();

            //phase 24 Clown 1 dialog
            clown_dialog_helper(stream, context, DPAD_RIGHT);
            context.wait_for_all_requests();

            pbf_press_dpad(context, DPAD_UP, 187ms, 0ms);
            pbf_wait(context, 106ms);
            pbf_press_dpad(context, DPAD_UP, 32ms, 0ms);
            pbf_wait(context, 671ms);
            pbf_press_dpad(context, DPAD_UP, 262ms, 0ms);
            pbf_wait(context, 684ms);
            pbf_press_dpad(context, DPAD_UP, 372ms, 0ms);
            pbf_wait(context, 392ms);
            pbf_press_dpad(context, DPAD_UP, 419ms, 0ms);
            pbf_wait(context, 658ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 2013ms, 0ms);
            pbf_wait(context, 369ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1448ms, 0ms);
            pbf_wait(context, 2435ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1077ms, 0ms);
            pbf_wait(context, 1084ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 1280ms, 0ms);
            pbf_wait(context, 1895ms);
            pbf_press_dpad(context, DPAD_LEFT, 88ms, 0ms);

            context.wait_for_all_requests();

            //phase 25 Clown 2 dialog

            clown_dialog_helper(stream, context, DPAD_UP);

            context.wait_for_all_requests();
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 511ms, 0ms);
            pbf_wait(context, 1318ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 140ms, 0ms);
            pbf_wait(context, 1502ms);
            pbf_move_left_joystick(context, { -1.000000, -0.000000 }, 2093ms, 0ms);
            pbf_wait(context, 2327ms);
            pbf_press_dpad(context, DPAD_LEFT, 151ms, 0ms);
            pbf_wait(context, 549ms);
            pbf_press_dpad(context, DPAD_LEFT, 118ms, 0ms);
            pbf_wait(context, 999ms);
            pbf_press_dpad(context, DPAD_UP, 206ms, 0ms);
            pbf_wait(context, 474ms);
            pbf_press_dpad(context, DPAD_UP, 249ms, 0ms);
            pbf_wait(context, 333ms);
            pbf_press_dpad(context, DPAD_UP, 212ms, 0ms);
            pbf_wait(context, 427ms);
            pbf_press_dpad(context, DPAD_UP, 210ms, 0ms);
            context.wait_for_all_requests();
            //phase 26 Clown 3 dialog

            clown_dialog_helper(stream, context, DPAD_LEFT);

            context.wait_for_all_requests();
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1098ms, 0ms);
            pbf_wait(context, 2779ms);
            pbf_press_dpad(context, DPAD_LEFT, 177ms, 0ms);
            pbf_wait(context, 488ms);
            pbf_press_dpad(context, DPAD_LEFT, 200ms, 0ms);
            pbf_wait(context, 649ms);
            pbf_press_dpad(context, DPAD_LEFT, 160ms, 0ms);
            pbf_wait(context, 4285ms);
            pbf_press_dpad(context, DPAD_DOWN, 307ms, 0ms);
            pbf_wait(context, 639ms);
            pbf_press_dpad(context, DPAD_DOWN, 463ms, 0ms);
            pbf_wait(context, 2521ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 3600ms, 0ms);
            pbf_wait(context, 610ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1907ms, 0ms);
            pbf_wait(context, 529ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 275ms, 0ms);
            pbf_wait(context, 3600ms);
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 899ms, 0ms);
            pbf_wait(context, 1308ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 192ms, 0ms);
            pbf_wait(context, 3238ms);
            pbf_press_dpad(context, DPAD_UP, 108ms, 0ms);
            pbf_wait(context, 433ms);
            pbf_press_dpad(context, DPAD_UP, 260ms, 0ms);
            pbf_wait(context, 365ms);
            pbf_press_dpad(context, DPAD_UP, 187ms, 0ms);
            pbf_wait(context, 542ms);
            pbf_press_dpad(context, DPAD_UP, 243ms, 0ms);
            pbf_wait(context, 370ms);
            pbf_press_dpad(context, DPAD_UP, 217ms, 0ms);
            pbf_wait(context, 495ms);
            pbf_press_dpad(context, DPAD_UP, 219ms, 0ms);
            pbf_wait(context, 572ms);
            pbf_press_dpad(context, DPAD_UP, 231ms, 0ms);
            pbf_wait(context, 1757ms);
            pbf_press_dpad(context, DPAD_RIGHT, 115ms, 0ms);
            pbf_press_button(context, BUTTON_A, 150ms, 0ms);
            pbf_wait(context, 795ms);
            context.wait_for_all_requests();
            wait_for_dialogue(stream, context, "Phase 27 (poketech challenge completed dialog)");

            pbf_mash_button(context, BUTTON_B, 15000ms);
            context.wait_for_all_requests();

            // poketech acquired
            pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 1194ms, 0ms);
            pbf_wait(context, 1984ms);
            pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 832ms, 0ms);
            pbf_wait(context, 1522ms);
            pbf_press_dpad(context, DPAD_RIGHT, 144ms, 0ms);
            pbf_wait(context, 1767ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1344ms, 0ms);
            pbf_wait(context, 3428ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 1099ms, 0ms);
            pbf_wait(context, 1222ms);
            pbf_press_dpad(context, DPAD_RIGHT, 228ms, 0ms);
            pbf_wait(context, 625ms);
            pbf_press_dpad(context, DPAD_RIGHT, 238ms, 0ms);
            pbf_wait(context, 537ms);
            pbf_press_dpad(context, DPAD_RIGHT, 210ms, 0ms);
            pbf_wait(context, 566ms);
            pbf_press_dpad(context, DPAD_RIGHT, 179ms, 0ms);
            pbf_wait(context, 692ms);
            pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 642ms, 0ms);
        }
        // ---------------------------------------------------------------------------
        // Segment 00-04 ends
        // Standing in front of the Jubilife City Pokemon Center PC.
        // Player will need to deposit pokemon in the pc and transfer to Pokemon Home
        // and level to 100 before starting the next segment.
        // ---------------------------------------------------------------------------

        // ---------------------------------------------------------------------------
        //  Segment / Checkpoint classes
        // ---------------------------------------------------------------------------

        std::string AutoStory_Segment_00::name()       const { return "00: Intro to Poketech"; }
        std::string AutoStory_Segment_00::start_text() const { return "Start: Game launched, Standing in the player's room after tv stops playing"; }
        std::string AutoStory_Segment_00::end_text()   const { return "End: Starter Pokemon chosen, received, and Pokedex acquired, standing in front of the PC."; }

        void AutoStory_Segment_00::run_segment(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) const {
            stats.m_segment++;
            env.update_stats();
            context.wait_for_all_requests();
            env.console.log("Start Segment " + name(), COLOR_ORANGE);

            AutoStory_Checkpoint_0001().run_checkpoint(env, context, options, stats);
            AutoStory_Checkpoint_0002().run_checkpoint(env, context, options, stats);
            AutoStory_Checkpoint_0003().run_checkpoint(env, context, options, stats);
            AutoStory_Checkpoint_0004().run_checkpoint(env, context, options, stats);
            AutoStory_Checkpoint_0005().run_checkpoint(env, context, options, stats);

            context.wait_for_all_requests();
            env.console.log("End Segment " + name(), COLOR_GREEN);
        }

        static void checkpoint_0001(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) {
            checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                [&](size_t /*attempt*/) {
                    // Segment 00-00: Intro and starter selection
                    tutorial_part_1_navigation(env, env.console, context);
                },
                true
            );
        }
        static void checkpoint_0002(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) {
            checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                [&](size_t /*attempt*/) {
                    // Segment 00-01: Starter acquisition
                    starter_nav(env.console, context);
                    // Segment 00-00 starter selection
                    switch (options.starter_choice) {
                    case StarterChoice::TURTWIG:  select_turtwig(env.console, context);  break;
                    case StarterChoice::CHIMCHAR: select_chimchar(env.console, context); break;
                    case StarterChoice::PIPLUP:   select_piplup(env.console, context);   break;
                    }
                    //Fight Starly
                    fight_starly(env.console, context);
                },
                true
					post_starly_clenaup(env.console, context);
                }
                true
            );
        }
        static void checkpoint_0003(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) {
            checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                [&](size_t /*attempt*/) {
                    // Segment 00-002: Pokedex acquisition
                    tutorial_part_2_navigation(env, env.console, context);
                },
                true
            );
        }
        static void checkpoint_0004(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) {
            checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                [&](size_t /*attempt*/) {
                    // Segment 00-003: Poketech acquisition
                    tutorial_part_3_navigation(env, env.console, context);
                },
                true
            );
        }
        static void checkpoint_0005(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) {
            checkpoint_reattempt_loop(env, context, options.notif_status_update, stats,
                [&](size_t /*attempt*/) {
                    // Segment 00-004: Poketech acquisition
                    tutorial_part_4_navigation(env, env.console, context);
                },
                true
            );
        }

        // ========================
        // Checkpoint 0001 Wrapper
        // ========================
        std::string AutoStory_Checkpoint_0001::name()       const { return "00.01 - After TV"; }
        std::string AutoStory_Checkpoint_0001::start_text() const { return "Game launched, Standing in the player's room after tv stops playing"; }
        std::string AutoStory_Checkpoint_0001::end_text()   const { return "Standing outside Lake Verity before picking starter."; }

        void AutoStory_Checkpoint_0001::run_checkpoint(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) const {
            checkpoint_0001(env, context, options, stats);
        }

        // ========================
        // Checkpoint 0002 Wrapper
        // ========================

        std::string AutoStory_Checkpoint_0002::name()       const { return "00.02 - Outside Verity before starter"; }
        std::string AutoStory_Checkpoint_0002::start_text() const { return AutoStory_Checkpoint_0001().end_text(); }
        std::string AutoStory_Checkpoint_0002::end_text()   const { return "Standing outside Lake Verity after picking starter."; }

        void AutoStory_Checkpoint_0002::run_checkpoint(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) const {
            checkpoint_0002(env, context, options, stats);
        }


        // ========================
        // Checkpoint 0003 Wrapper
        // ========================

        std::string AutoStory_Checkpoint_0003::name()       const { return "00.03 - Outside Verity after starter"; }
        std::string AutoStory_Checkpoint_0003::start_text() const { return AutoStory_Checkpoint_0002().end_text(); }
        std::string AutoStory_Checkpoint_0003::end_text()   const { return "Standing in front of The professor after recieving the Pokedex"; }

        void AutoStory_Checkpoint_0003::run_checkpoint(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) const {
            checkpoint_0003(env, context, options, stats);
        }
        // ========================
        // Checkpoint 0004 Wrapper
        // ========================

        std::string AutoStory_Checkpoint_0004::name()       const { return "00.04 - After Pokedex"; }
        std::string AutoStory_Checkpoint_0004::start_text() const { return AutoStory_Checkpoint_0003().end_text(); }
        std::string AutoStory_Checkpoint_0004::end_text()   const { return "Standing in in jubilife after speaking to dawn. Next: give town map to barry"; }

        void AutoStory_Checkpoint_0004::run_checkpoint(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) const {
            checkpoint_0004(env, context, options, stats);
        }
        // ========================
        // Checkpoint 0005 Wrapper
        // ========================

        std::string AutoStory_Checkpoint_0005::name()       const { return "00.05 - Jubilife After Dawn"; }
        std::string AutoStory_Checkpoint_0005::start_text() const { return AutoStory_Checkpoint_0004().end_text(); }
        std::string AutoStory_Checkpoint_0005::end_text()   const { return "Standing in front of PC. Recieved the Poketech. Ready to deposit starter"; }

        void AutoStory_Checkpoint_0005::run_checkpoint(
            SingleSwitchProgramEnvironment& env,
            ProControllerContext& context,
            AutoStoryOptions options,
            AutoStoryStats& stats
        ) const {
            checkpoint_0005(env, context, options, stats);
        }
        
        }
    }
}
