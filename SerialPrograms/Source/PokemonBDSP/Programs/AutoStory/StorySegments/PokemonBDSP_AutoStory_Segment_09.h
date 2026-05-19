/*  BDSP AutoStory - Segment 09
 *  From after acquiring the eighth badge until defeating the Elite Four and Champion.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Segment09_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Segment09_H

#include "../PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class AutoStory_Segment_09 : public AutoStory_Segment{
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

class AutoStory_Checkpoint_022 : public AutoStory_Checkpoint{
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

// start: eighth badge acquired
// end:   Elite Four and Champion defeated, credits roll
void checkpoint_022(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);


}
}
}
#endif
