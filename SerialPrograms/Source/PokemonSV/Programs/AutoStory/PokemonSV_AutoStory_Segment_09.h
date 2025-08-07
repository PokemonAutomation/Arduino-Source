/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_09_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_09_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_09 : public AutoStory_Segment{
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


// start: Talked to Jacq in classroom. Standing in classroom.
// end: Talked to Arven. Received Titan questline (Path of Legends). Talked to Cassiopeia. Standing in main hall.
void checkpoint_16(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Talked to Arven. Received Titan questline (Path of Legends).
// end: Talked to Cassiopeia. Saw Geeta. Talked to Nemona. Received Gym/Elite Four questline (Victory Road). Standing in staff room.
void checkpoint_17(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Talked to Cassiopeia. Saw Geeta. Talked to Nemona. Received Gym/Elite Four questline (Victory Road). Standing in staff room.
// end: Talked to Clavell and the professor.
void checkpoint_18(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Talked to Clavell and the professor.
// end: Talked to Nemona, visited dorm, time passed.
void checkpoint_19(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);

// start: Talked to Nemona, visited dorm, time passed.
// end: Get on ride for first time.
void checkpoint_20(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
);


}
}
}
#endif
