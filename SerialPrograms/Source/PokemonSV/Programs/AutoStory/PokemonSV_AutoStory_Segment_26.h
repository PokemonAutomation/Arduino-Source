/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_26_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_26_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_26 : public AutoStory_Segment{
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

class AutoStory_Checkpoint_61 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_62 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};

class AutoStory_Checkpoint_63 : public AutoStory_Checkpoint{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const override;
};




// start: At Medali Pokecenter.
// end: Inside Medali Gym building
void checkpoint_61(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    const std::string& checkpoint_text
);

// start: Inside Medali Gym building
// end: Beat Medali Gym. Inside Medali Gym building.
void checkpoint_62(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    const std::string& checkpoint_text
);


// start: Beat Medali Gym. Inside Medali Gym building.
// end: At Glaseado Mountain Pokecenter
void checkpoint_63(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    const std::string& checkpoint_text
);

// moves player from Medali East Pokecenter to Glaseado Mountain Pokecenter
void move_from_medali_to_glaseado_mountain(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


}
}
}
#endif
