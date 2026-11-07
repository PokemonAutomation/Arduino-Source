/*  BDSP AutoStory - Segment 00
 *  From beginning cinematic until after choosing the starter Pokemon.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Segment00_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Segment00_H

#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {


class AutoStory_Segment_00 : public AutoStory_Segment {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_0000 : public AutoStory_Checkpoint {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};
void checkpoint_0000(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats);

class AutoStory_Checkpoint_0001 : public AutoStory_Checkpoint {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};
void checkpoint_0001(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats);

class AutoStory_Checkpoint_0002 : public AutoStory_Checkpoint {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};
void checkpoint_0002(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats);

class AutoStory_Checkpoint_0003 : public AutoStory_Checkpoint {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};
void checkpoint_0003(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats);

class AutoStory_Checkpoint_0004 : public AutoStory_Checkpoint {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};
void checkpoint_0004(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats);

class AutoStory_Checkpoint_0005 : public AutoStory_Checkpoint {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};
void checkpoint_0005(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats);

class AutoStory_Checkpoint_0006 : public AutoStory_Checkpoint {
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};
void checkpoint_0006(SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats);

}
}
}
#endif
