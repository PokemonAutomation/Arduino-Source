/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_38.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_38::name() const{
    return "";
}

std::string AutoStory_Segment_38::start_text() const{
    return "Start: ";
}

std::string AutoStory_Segment_38::end_text() const{
    return "End: ";
}

void AutoStory_Segment_38::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    // AutoStory_Checkpoint_9().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_100::name() const{ return "0100 - " + AutoStory_Segment_38().name(); }
std::string AutoStory_Checkpoint_100::start_text() const{ return "";}
std::string AutoStory_Checkpoint_100::end_text() const{ return "";}
void AutoStory_Checkpoint_100::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_100(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_101::name() const{ return "0101 - " + AutoStory_Segment_38().name(); }
std::string AutoStory_Checkpoint_101::start_text() const{ return "";}
std::string AutoStory_Checkpoint_101::end_text() const{ return "";}
void AutoStory_Checkpoint_101::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_101(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_102::name() const{ return "0102 - " + AutoStory_Segment_38().name(); }
std::string AutoStory_Checkpoint_102::start_text() const{ return "";}
std::string AutoStory_Checkpoint_102::end_text() const{ return "";}
void AutoStory_Checkpoint_102::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_102(env, context, options.notif_status_update, stats);
}

// std::string AutoStory_Checkpoint_103::name() const{ return "0103 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_103::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_103::end_text() const{ return "";}
// void AutoStory_Checkpoint_103::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_103(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_104::name() const{ return "0104 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_104::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_104::end_text() const{ return "";}
// void AutoStory_Checkpoint_104::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_104(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_105::name() const{ return "0105 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_105::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_105::end_text() const{ return "";}
// void AutoStory_Checkpoint_105::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_105(env, context, options.notif_status_update, stats);
// }

void checkpoint_100(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_101(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_102(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_103(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_104(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_105(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

}
}
}
