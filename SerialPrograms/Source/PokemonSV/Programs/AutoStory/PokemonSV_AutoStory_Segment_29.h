/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_29_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_29_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_29 : public AutoStory_Segment{
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

// start: At Montenevera Pokecenter
// end: Spoke to gym receptionis
void checkpoint_71(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Spoke to gym receptionis
// end: Passed gym test with MC Sledge.
void checkpoint_72(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Passed gym test with MC Sledge.
// end: Beat Montenevera Gym. Inside gym building.
void checkpoint_73(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Beat Montenevera Gym. Inside gym building.
// end: At Glaseado gym Pokecenter.
void checkpoint_74(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// moves player from Montenevera Pokecenter to Glaseado Gym Pokecenter
void move_from_montenevera_to_glaseado_gym(SingleSwitchProgramEnvironment& env, ProControllerContext& context);



// start: 
// end: 
void checkpoint_75(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: 
// end: 
void checkpoint_76(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);


}
}
}
#endif
