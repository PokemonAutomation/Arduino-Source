/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_23.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_23::name() const{
    return "23: Orthworm Titan";
}

std::string AutoStory_Segment_23::start_text() const{
    return "Start: Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.";
}

std::string AutoStory_Segment_23::end_text() const{
    return "End: Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.";
}

void AutoStory_Segment_23::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_54().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_54::name() const{ return "054 - " + AutoStory_Segment_23().name(); }
std::string AutoStory_Checkpoint_54::start_text() const{ return "Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.";}
std::string AutoStory_Checkpoint_54::end_text() const{ return "Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.";}
void AutoStory_Checkpoint_54::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_54(env, context, options.notif_status_update, stats);
}


void checkpoint_54(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // empty checkpoint


    });

            

}


}
}
}
