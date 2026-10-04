/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *  From: https://github.com/PokemonAutomation/
 */

#include "PokemonBDSP_AutoStory_Segment_01.h"
#include "../Utils/PokemonBDSP_Autostory_Segment_01_tools.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {
            std::string AutoStory_Segment_01::name()       const { return "01: Jubilife City to First Badge"; }
            std::string AutoStory_Segment_01::start_text() const { return "Start: Received Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki."; }
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
            std::string AutoStory_checkpoint_0101::start_text() const { return "Received Starter back from Pokemon Home. Max leveled pokemon should have the moveset in the wiki."; }
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

            std::string AutoStory_checkpoint_0102::name()       const { return "01.02 - Oreburgh Mine"; }
            std::string AutoStory_checkpoint_0102::start_text() const { return AutoStory_checkpoint_0101().end_text(); }
            std::string AutoStory_checkpoint_0102::end_text()   const { return "Inside Oreburgh Gym, ready to navigate to Roark."; }

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
                        oreburgh_mine_nav(env.console, context);
                        // Standalone test entry points are not part of the checkpoint route.
                        // oreburgh_mine_first_leg_test(env.console, context);
                        // oreburgh_mine_route_and_escape_test(env.console, context);
                        oreburgh_mine_to_gym(env, env.console, context);
                    },

                    true
                );
            }

            // ========================
            // Checkpoint 0103 Wrapper
            // ========================

            std::string AutoStory_checkpoint_0103::name()       const { return "01.03 - Oreburgh Gym"; }
            std::string AutoStory_checkpoint_0103::start_text() const { return AutoStory_checkpoint_0102().end_text(); }
            std::string AutoStory_checkpoint_0103::end_text()   const { return "Standing at Roark after defeating him and acquiring the first badge."; }

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
                        gym_Puzzle_01(env, env.console, context);

                    },

                    true
                );
            }


        }
    }
}
