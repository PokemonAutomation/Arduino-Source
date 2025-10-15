/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_30_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_30_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_30 : public AutoStory_Segment{
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



// start: At Glaseado Gym Pokecenter
// end: Battled Nemona. Spoke to Glaseado Gym receptionist
void checkpoint_75(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Spoke to Glaseado Gym receptionist. 
// end: Beat Glaseado Gym Challenge. Beat Glaseado Gym (Ice).
void checkpoint_76(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Beat Glaseado Gym Challenge. Beat Glaseado Gym (Ice). 
// end: At North Province Area One Pokecenter
void checkpoint_77(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);



// moves player from Glaseado Gym Pokecenter to North Province Area One
void move_from_glaseado_gym_to_north_province_area_one(SingleSwitchProgramEnvironment& env, ProControllerContext& context);




}
}
}
#endif
