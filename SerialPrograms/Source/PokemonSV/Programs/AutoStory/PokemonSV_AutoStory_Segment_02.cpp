/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_02.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





std::string AutoStory_Segment_02::name() const{
    return "02: First Nemona Battle";
}

std::string AutoStory_Segment_02::start_text() const{
    return "Start: Picked the starter. Changed move order.";
}

std::string AutoStory_Segment_02::end_text() const{
    return "End: Battled Nemona on the beach.";
}

void AutoStory_Segment_02::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);

    checkpoint_04(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment 02: First Nemona Battle", COLOR_GREEN);         

}


void checkpoint_04(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){        
        context.wait_for_all_requests();

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 3.72);
        pbf_move_left_joystick(context, 128, 0, 400, 50);
        direction.change_direction(env.program_info(), env.console, context, 4.55);
        pbf_move_left_joystick(context, 128, 0, 600, 50);
        direction.change_direction(env.program_info(), env.console, context, 5.27);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20);

        context.wait_for_all_requests();
        env.console.log("Starting battle...");
        // TODO: Battle start prompt detection
        // can lose this battle, and story will continue
        mash_button_till_overworld(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Finished battle.");

    }
    );

}




}
}
}
