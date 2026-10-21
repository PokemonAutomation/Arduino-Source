/*  BDSP AutoStory - Segment 01
 *  From after choosing the starter Pokemon until after acquiring the first badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Segment01_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Segment01_H

#include "../PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// Route ownership: Oreburgh navigation and isolated route tests belong to Segment 01.
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

class AutoStory_Segment_01 : public AutoStory_Segment{
public:
    virtual std::string name()       const override;
    virtual std::string start_text() const override;
    virtual std::string end_text()   const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const override;
};

class AutoStory_checkpoint_0101 : public AutoStory_Checkpoint{
public:
    virtual std::string name()       const override;
    virtual std::string start_text() const override;
    virtual std::string end_text()   const override;
    virtual void run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const override;
};

// start: starter Pokemon taken out of PC at level 100 and ready to be used in battle
// end:   first Gym Badge acquired (Oreburgh City)
void checkpoint_0101(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);
class AutoStory_checkpoint_0102 : public AutoStory_Checkpoint {
public:
    virtual std::string name()       const override;
    virtual std::string start_text() const override;
    virtual std::string end_text()   const override;
    virtual void run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const override;
};

// start: starter Pokemon taken out of PC at level 100 and ready to be used in battle
// end:   first Gym Badge acquired (Oreburgh City)
void checkpoint_0102(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_checkpoint_0103 : public AutoStory_Checkpoint {
public:
    virtual std::string name()       const override;
    virtual std::string start_text() const override;
    virtual std::string end_text()   const override;
    virtual void run_checkpoint(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const override;
};

// start: starter Pokemon taken out of PC at level 100 and ready to be used in battle
// end:   first Gym Badge acquired (Oreburgh City)
void checkpoint_0103(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

}
}
}
#endif
