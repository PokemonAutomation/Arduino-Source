/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_31_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_31_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_31 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats
    ) const override;
};

class AutoStory_Checkpoint_78 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_79 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_80 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_81 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};



// start: At North Province Area One Pokecenter.
// end: At Team Star (Fighting) base. Spoke to Clavell.
void checkpoint_78(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: At Team Star (Fighting) base. Spoke to Clavell.
// end: Beat Team Star (Fighting) grunt. At gate of base.
void checkpoint_79(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);


// start: Beat Team Star (Fighting) grunt. At gate of base.
// end: Beat Team Star (Fighting)
void checkpoint_80(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Beat Team Star (Fighting)
// end: At North Province Area Two Pokecenter
void checkpoint_81(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);


// moves player from North Province Area One to Team Star Fighting base
void move_from_north_province_area_one_to_fighting_base(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


void beat_team_star_fighting1(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

void beat_team_star_fighting2(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

void move_from_fighting_base_to_north_province_area_two(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


}
}
}
#endif
