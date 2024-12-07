/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
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
    return "Start: Picked the starter.";
}

std::string AutoStory_Segment_02::end_text() const{
    return "End: Battled Nemona on the beach.";
}

void AutoStory_Segment_02::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);
    env.console.overlay().add_log("Start Segment 02: First Nemona Battle", COLOR_ORANGE);

    checkpoint_04(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment 02: First Nemona Battle", COLOR_GREEN);
    env.console.overlay().add_log("End Segment 02: First Nemona Battle", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats(); 

}


void checkpoint_04(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{        
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }        
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

        break;
    }catch(...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }        
    }

}




}
}
}
