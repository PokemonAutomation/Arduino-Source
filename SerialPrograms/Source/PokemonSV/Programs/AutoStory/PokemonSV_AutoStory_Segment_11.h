/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_11_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_11_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_11 : public AutoStory_Segment{
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


// start: At Cortondo East Pokecenter
// end: Spoke to Cortondo Gym reception. At Cortondo West Pokecenter.
void checkpoint_24(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Spoke to Cortondo Gym reception. At Cortondo West Pokecenter.
// end: Defeated the trainers at Olive Roll, but left Olive unmoved. Then backed out, standing in front of the Olive Roll NPC.
void checkpoint_25(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Defeated the trainers at Olive Roll, but left Olive unmoved. Then backed out, standing in front of the Olive Roll NPC.
// end: Completed Olive roll gym challenge.
void checkpoint_26(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Completed Olive roll gym challenge. 
// end: At Cortondo East Pokecenter.
void checkpoint_27(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);



}
}
}
#endif
