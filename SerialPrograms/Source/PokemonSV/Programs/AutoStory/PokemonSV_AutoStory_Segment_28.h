/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_28_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_28_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_28 : public AutoStory_Segment{
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

// start: At North Province Area Three Pokecenter
// end: Beat team star grunt. At gate of Team Star (Fairy) base.
void checkpoint_68(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Beat team star grunt. At gate of Team Star (Fairy) base.
// end: Beat Team Star (Fairy)
void checkpoint_69(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// start: Beat Team Star (Fairy)
// end: At Montenevera Pokecenter
void checkpoint_70(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);


// moves player from Glaseado Mountain Pokecenter to Montenevera Pokecenter
void move_from_glaseado_mountain_to_montenevera(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

}
}
}
#endif
