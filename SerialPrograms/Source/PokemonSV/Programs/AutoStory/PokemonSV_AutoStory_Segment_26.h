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




// start: At Medali Pokecenter.
// end: At Medali Gym
void checkpoint_61(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: At Medali Gym
// end: Beat Medali Gym
void checkpoint_62(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);


// start: Beat Medali Gym
// end: At Glaseado Mountain Pokecenter
void checkpoint_63(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// moves player from Medali West Pokecenter to Glaseado Mountain Pokecenter
void move_from_medali_to_glaseado_mountain(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


}
}
}
#endif
