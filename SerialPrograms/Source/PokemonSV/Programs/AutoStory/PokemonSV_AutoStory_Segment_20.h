/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_20_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_20_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_20 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options,
        AutoStoryStats& stats) const override;
};


// start: Defeated Klawf. At Artazon (West) Pokecenter.
// end: At Artazon Gym building. Battled Nemona. Received Sunflora gym challenge.
void checkpoint_43(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: At Artazon Gym building. Battled Nemona. Received Sunflora gym challenge.
// end: Finished Sunflora gym challenge.
void checkpoint_44(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Finished Sunflora gym challenge.
// end: Defeated Artazon Gym (Grass). Inside gym building.
void checkpoint_45(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Defeated Artazon Gym (Grass). Inside gym building.
// end: At East Province (Area One) Pokecenter.
void checkpoint_46(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);


}
}
}
#endif
