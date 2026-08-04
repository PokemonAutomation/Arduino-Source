/*  BDSP AutoStory - Segment 02
 *  From after acquiring the first badge until after acquiring the second badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Segment02_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Segment02_H

#include "../PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class AutoStory_Segment_02 : public AutoStory_Segment{
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

class AutoStory_checkpoint_0201 : public AutoStory_Checkpoint{
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

// start: first badge acquired
// end:   second Gym Badge acquired (Eterna City)
void checkpoint_0201(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_checkpoint_0202 : public AutoStory_Checkpoint {
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

// start: first badge acquired
// end:   second Gym Badge acquired (Eterna City)
void checkpoint_0202(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);
class AutoStory_checkpoint_0203 : public AutoStory_Checkpoint {
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

// start: first badge acquired
// end:   second Gym Badge acquired (Eterna City)
void checkpoint_0203(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_checkpoint_0204 : public AutoStory_Checkpoint {
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

// start: first badge acquired
// end:   second Gym Badge acquired (Eterna City)
void checkpoint_0204(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_checkpoint_0205 : public AutoStory_Checkpoint {
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

// start: first badge acquired
// end:   second Gym Badge acquired (Eterna City)
void checkpoint_0205(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);


}
}
}
#endif
