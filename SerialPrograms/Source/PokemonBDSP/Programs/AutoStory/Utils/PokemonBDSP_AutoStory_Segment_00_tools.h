#pragma once

#include "PokemonBDSP_OverworldMatchers.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "../StorySegments/PokemonBDSP_AutoStory_Segment_00.h"
#include "CommonTools/Async/InferenceRoutines.h"

#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"

#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // ---------------------------------------------------------------------------
            // Utility Watchers
            // ---------------------------------------------------------------------------

            class BattleDialogWatcher : public DetectorToFinder<BattleDialogDetector> {
            public:
                BattleDialogWatcher(Color color = COLOR_RED);
            };

            // ---------------------------------------------------------------------------
            // Time / setup helpers
            // ---------------------------------------------------------------------------

            void verify_tutorial_settings(VideoStream& stream, ProControllerContext& context);

            void set_time_fast(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                const DateTime& target
            );

            // ---------------------------------------------------------------------------
            // Battles
            // ---------------------------------------------------------------------------

            void fight_starly(
                VideoStream& stream,
                ProControllerContext& context
            );

            void fight_tristan(
                VideoStream& stream,
                ProControllerContext& context
            );

            void fight_Natalie(
                VideoStream& stream,
                ProControllerContext& context
            );

            // ---------------------------------------------------------------------------
            // Catching / healing
            // ---------------------------------------------------------------------------

            void catch_1_pokemon(
                VideoStream& stream,
                ProControllerContext& context
            );

            // One wild encounter using Scratch, followed by verified Potion healing.
            void battle_1_pokemon(VideoStream& stream, ProControllerContext& context);

            // Burst wiggle variant of catch_1_pokemon for testing.
            // Commented out in the .cpp - uncomment both to enable.
            // void catch_1_pokemon_v2(
            //     SingleSwitchProgramEnvironment& env,
            //     VideoStream& stream,
            //     ProControllerContext& context
            // );

            void use_potion_first_pokemon(
                VideoStream& stream,
                ProControllerContext& context
            );

            // ---------------------------------------------------------------------------
            // Grass movement helpers
            // ---------------------------------------------------------------------------

            void walk_through_grass(
                VideoStream& stream,
                ProControllerContext& context,
                int8_t x,
                int8_t y,
                size_t steps,
                const std::string& direction_name,
                BattleType battle_type = BattleType::STANDARD,
                Milliseconds movement_duration = Milliseconds(200),
                OverworldMatchResult (*destination)(const ImageRGB32&) = nullptr
            );

            bool detect_black_screen_after_movement(
                VideoStream& stream,
                ProControllerContext& context,
                Milliseconds timeout
            );

            bool try_right_down_exit_search(
                VideoStream& stream,
                ProControllerContext& context,
                size_t iterations
            );

            void leave_pokemon_center_before_barry_1(
                VideoStream& stream,
                ProControllerContext& context
            );
			// ---------------------------------------------------------------------------
			// Helper function to navigate the poketech challenge clown dialogue.
            // Example usage
            // Move up and talk to clown.
            // clown_dialog_helper(
            //     stream,
            //     context,
            //     DPAD_UP
            // );

            // Move left and talk to clown.
            // clown_dialog_helper(
            //    stream,
            //    context,
            //    DPAD_LEFT
            // );

            // Move right and talk to clown.
            // clown_dialog_helper(
            //    stream,
            //    context,
            //    DPAD_RIGHT
            // );
			// ----------------------------------------------------------------------------
            void clown_dialog_helper(
                VideoStream& stream,
                ProControllerContext& context,
                DpadPosition direction
            );
        }
    }
}