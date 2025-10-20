/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_Segment_34_H
#define PokemonAutomation_PokemonSV_AutoStory_Segment_34_H

#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class AutoStory_Segment_34 : public AutoStory_Segment{
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

static constexpr std::string segment_num = "34";



inline std::string checkpoint90_start(){ return segment_num + ": Beat Geeta. At Pokemon League Pokecenter.";}
inline std::string checkpoint90_end(){ return segment_num + ": Beat Nemona. At dormitory room, next to bed.";}
// start: Beat Geeta. At Pokemon League Pokecenter.
// end: Beat Nemona. At dormitory room, next to bed.
void checkpoint_90(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

inline std::string checkpoint91_start(){ return checkpoint90_end();}
inline std::string checkpoint91_end(){ return segment_num + ": Beat Penny. At Academy fly point.";}
// start: Beat Nemona. At dormitory room, next to bed."
// end: Beat Penny. At Academy fly point.
void checkpoint_91(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

inline std::string checkpoint92_start(){ return checkpoint91_end();}
inline std::string checkpoint92_end(){ return segment_num + ": Beat Arven. At Los Platos Pokecenter.";}
// start: Beat Penny. At Academy fly point.
// end: Beat Arven. At Los Platos Pokecenter.
void checkpoint_92(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// std::string checkpoint_start(){ return segment_num + "";}
// std::string checkpoint_end(){ return segment_num + "";}
// start: 
// end: 
void checkpoint_93(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// std::string checkpoint_start(){ return segment_num + "";}
// std::string checkpoint_end(){ return segment_num + "";}
// start: 
// end: 
void checkpoint_94(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);

// std::string checkpoint_start(){ return segment_num + "";}
// std::string checkpoint_end(){ return segment_num + "";}
// start: 
// end: 
void checkpoint_95(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats);





}
}
}
#endif
