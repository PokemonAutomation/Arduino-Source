/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "../PokemonBDSP_AutoStoryTools.h"
#include "PokemonBDSP_AutoStory_Segment_01.h"
#include "PokemonBDSP_AutoStory_Segment_00_tools.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {
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
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 814ms, 0ms);
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
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 415ms, 0ms);
                pbf_wait(context, 390ms);
                pbf_move_left_joystick(context, { 0.000000, 1.000000 }, 321ms, 0ms);
                walk_through_grass(stream, context, 1, 0, 16, "right through grass");
                walk_through_grass(stream, context, 0, -1, 3, "down through grass");
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 1952ms, 0ms);
                walk_through_grass(stream, context, 1, 0, 16, "right through grass");
                walk_through_grass(stream, context, 0, 1, 16, "up through grass");
                pbf_press_button(context, BUTTON_DOWN, 180ms, 0ms);
                pbf_press_button(context, BUTTON_RIGHT, 180ms, 0ms);
                pbf_press_button(context, BUTTON_RIGHT, 180ms, 0ms);
            }
            static void oreburgh_gate_nav(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {

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
                wait_for_dialogue(stream, context, "Phase 2 (oreburgh tour dialog)");
                pbf_mash_button(context, BUTTON_B, 12000ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 319ms, 0ms);
                pbf_move_left_joystick(context, { 1.000000, -1.000000 }, 332ms, 0ms);
                pbf_move_left_joystick(context, { 1.000000, -0.000000 }, 3124ms, 0ms);
                pbf_wait(context, 23ms);
                pbf_move_left_joystick(context, { 0.000000, -1.000000 }, 4740ms, 0ms);

            }
            static void oreburgh_mine_nav(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context) {

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

                AutoStory_checkpoint_005().run_checkpoint(env, context, options, stats);
                AutoStory_checkpoint_006().run_checkpoint(env, context, options, stats);
                context.wait_for_all_requests();
                env.console.log("End Segment " + name(), COLOR_GREEN);
            }


            std::string AutoStory_checkpoint_005::name()       const { return "005 - " + AutoStory_Segment_01().name(); }
            std::string AutoStory_checkpoint_005::start_text() const { return "Recieved Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki."; }
            std::string AutoStory_checkpoint_005::end_text()   const { return "Standing outside the Oreburgh Gate on the Jubilife City side."; }
            // ========================
            // Checkpoint 005 Wrapper
            // ========================
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
                        oreburgh_gate_nav(env, env.console, context);
                    }
                );
            }

            // ========================
            // Checkpoint 006 Wrapper
            // ========================

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
                        oreburgh_gate_to_mine(env, env.console, context);
                        oreburgh_mine_nav(env, env.console, context);
                        oreburgh_mine_to_gym(env, env.console, context);
                        //#. Oreburgh Gym navigation and battle
                        gym_Puzzle_01(env, env.console, context);

                    }
                );
            }


        }
    }
}