/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_22_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_22_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_22 : public AutoStory_Segment{
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


// start: Defeated Team Star (Fire). At East Province (Area Two) Pokecenter.
// end: At Levincia (South) Pokecenter.
void checkpoint_50(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: At Levincia (South) Pokecenter.
// end: At Levincia gym building. Talked to Hassel, met Rika.
void checkpoint_51(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);


// start: At Levincia gym building. Talked to Hassel, met Rika.
// end: Finished Levincia gym challenge.
void checkpoint_52(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);


// start: Finished Levincia gym challenge. Standing outside Levincia gym.
// end: Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.
void checkpoint_53(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);






}
}
}
#endif
