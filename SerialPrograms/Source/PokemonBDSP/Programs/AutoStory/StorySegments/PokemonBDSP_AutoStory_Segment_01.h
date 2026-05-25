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

class AutoStory_checkpoint_005 : public AutoStory_Checkpoint{
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
void checkpoint_005(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);
class AutoStory_checkpoint_006 : public AutoStory_Checkpoint {
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
void checkpoint_006(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

}
}
}
#endif
