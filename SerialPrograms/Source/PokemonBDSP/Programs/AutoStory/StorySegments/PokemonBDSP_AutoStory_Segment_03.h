/*  BDSP AutoStory - Segment 03
 *  From after acquiring the second badge until after acquiring the third badge.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Segment03_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Segment03_H

#include "../PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class AutoStory_Segment_03 : public AutoStory_Segment{
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

class AutoStory_Checkpoint_0301 : public AutoStory_Checkpoint{
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

// start: second badge acquired
// end:   third Gym Badge acquired (Hearthome City)
void Checkpoint_0301(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);
class AutoStory_Checkpoint_0302 : public AutoStory_Checkpoint {
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

// start: second badge acquired
// end:   third Gym Badge acquired (Hearthome City)
void Checkpoint_0302(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_Checkpoint_0303 : public AutoStory_Checkpoint {
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

// start: second badge acquired
// end:   third Gym Badge acquired (Hearthome City)
void Checkpoint_0303(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_Checkpoint_0304 : public AutoStory_Checkpoint {
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

// start: second badge acquired
// end:   third Gym Badge acquired (Hearthome City)
void Checkpoint_0304(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_Checkpoint_0305 : public AutoStory_Checkpoint {
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

// start: second badge acquired
// end:   third Gym Badge acquired (Hearthome City)
void Checkpoint_0305(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_Checkpoint_0306 : public AutoStory_Checkpoint {
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

// start: second badge acquired
// end:   third Gym Badge acquired (Hearthome City)
void Checkpoint_0306(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

class AutoStory_Checkpoint_0307 : public AutoStory_Checkpoint {
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

// start: second badge acquired
// end:   third Gym Badge acquired (Hearthome City)
void Checkpoint_0307(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
);

}
}
}
#endif
