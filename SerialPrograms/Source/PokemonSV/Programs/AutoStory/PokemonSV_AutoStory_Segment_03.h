/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_03_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_03_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_03 : public AutoStory_Segment{
public:
    virtual std::string name() const override;
    virtual std::string start_text() const override;
    virtual std::string end_text() const override;
    virtual void run_segment(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        AutoStoryOptions options) const override;
};


// start: Battled Nemona on the beach.
// end: Met mom at gate. Received mom's sandwich.
void checkpoint_05(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update
);

// start: Met mom at gate. Received mom's sandwich.
// end: Cleared catch tutorial.
void checkpoint_06(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update
);

// start: Cleared catch tutorial.
// end: Moved to cliff. Heard mystery cry.
void checkpoint_07(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update
);


}
}
}
#endif
