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
#include "PokemonSV_AutoStory_Segment_35.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_35::name() const{
    return "";
}

std::string AutoStory_Segment_35::start_text() const{
    return "Start: ";
}

std::string AutoStory_Segment_35::end_text() const{
    return "End: ";
}

void AutoStory_Segment_35::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_93().run_checkpoint(env, context, options, stats);


    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_93::name() const{ return "093 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_93::start_text() const{ return "";}
std::string AutoStory_Checkpoint_93::end_text() const{ return "";}
void AutoStory_Checkpoint_93::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_93(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_94::name() const{ return "094 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_94::start_text() const{ return "";}
std::string AutoStory_Checkpoint_94::end_text() const{ return "";}
void AutoStory_Checkpoint_94::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_94(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_95::name() const{ return "095 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_95::start_text() const{ return "";}
std::string AutoStory_Checkpoint_95::end_text() const{ return "";}
void AutoStory_Checkpoint_95::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_95(env, context, options.notif_status_update, stats);
}

// std::string AutoStory_Checkpoint_96::name() const{ return "096 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_96::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_96::end_text() const{ return "";}
// void AutoStory_Checkpoint_96::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_96(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_97::name() const{ return "097 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_97::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_97::end_text() const{ return "";}
// void AutoStory_Checkpoint_97::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_97(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_98::name() const{ return "098 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_98::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_98::end_text() const{ return "";}
// void AutoStory_Checkpoint_98::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_98(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_99::name() const{ return "099 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_99::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_99::end_text() const{ return "";}
// void AutoStory_Checkpoint_99::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_99(env, context, options.notif_status_update, stats);
// }

void checkpoint_93(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_94(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_95(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

// void checkpoint_96(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_97(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_98(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_99(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }




}
}
}
