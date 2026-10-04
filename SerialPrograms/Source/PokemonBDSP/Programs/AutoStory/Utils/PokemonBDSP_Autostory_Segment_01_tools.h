// Navigation and battle helpers for AutoStory Segment 01.
#pragma once

#include "../PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {
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

            void jubilife_to_oreburgh_gate(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context);

            void oreburgh_gate_nav(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context);

            void oreburgh_gate_to_mine(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context);

            void oreburgh_mine_nav(
                VideoStream& stream, ProControllerContext& context);

            void oreburgh_mine_to_gym(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context);

            void gym_Puzzle_01(SingleSwitchProgramEnvironment& env,
                VideoStream& stream, ProControllerContext& context);

            // Starts inside the mine at an entrance position; stops outside after
            // talking to Roark and using Escape Rope. Does not reset on failure.
            bool oreburgh_mine_route_and_escape_test(
                VideoStream& stream,
                ProControllerContext& context
            );

            // Shared stage runner for production navigation and isolated route tests.
            bool oreburgh_mine_route(
                VideoStream& stream,
                ProControllerContext& context,
                const MineRouteTestOptions& options
            );

            bool oreburgh_mine_first_leg_test(
                VideoStream& stream,
                ProControllerContext& context,
                const MineRouteTestOptions& options = MineRouteTestOptions()
            );
        }
    }
}
