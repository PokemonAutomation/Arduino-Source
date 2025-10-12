/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_27_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_27_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_27 : public AutoStory_Segment{
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

// start: At Glaseado Mountain Pokecenter
// end: At Casseroya Watchtower 3
void checkpoint_64(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: At Casseroya Watchtower 3
// end: Defeat Dondozo Titan phase 1.
void checkpoint_65(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Defeat Dondozo Titan phase 1.
// end: Defeat Dondozo Titan phase 2-3.
void checkpoint_66(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Defeat Dondozo Titan phase 2-3.
// end: At North Province Area Three Pokecenter
void checkpoint_67(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: 
// end: 
void checkpoint_68(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: 
// end: 
void checkpoint_69(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: 
// end: 
void checkpoint_70(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);



// moves player from Glaseado Mountain Pokecenter to Casseroya Watchtower 3
void move_from_glaseado_mountain_to_casseroya_watchtower3(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

// moves player from Casseroya Watchtower 3 to Dondozo Titan. Defeat Dondozo Titan phase 1.
void move_from_casseroya_watchtower3_to_dondozo_titan(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

// moves player from Dondozo Titan phase 1 to phase 2. Defeat Dondozo Titan phase 2-3.
void move_from_dondozo_titan_phase1_to_phase2(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

// moves player from Glaseado Mountain Pokecenter to North Province Area Three Pokecenter
void move_from_glaseado_mountain_to_north_province_area_three(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


}
}
}
#endif
