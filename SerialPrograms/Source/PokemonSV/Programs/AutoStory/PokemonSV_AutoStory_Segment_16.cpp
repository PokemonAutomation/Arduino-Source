/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_16.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_16::name() const{
    return "16: Cascarrafa Gym (Water): Get Kofu's wallet";
}

std::string AutoStory_Segment_16::start_text() const{
    return "Start: Defeated Team Star (Dark). At Cascarrafa (West) Pokecenter.";
}

std::string AutoStory_Segment_16::end_text() const{
    return "End: Received Kofu's wallet. At Porto Marinada Pokecenter.";
}

void AutoStory_Segment_16::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_35(env, context, options.notif_status_update, stats);
    checkpoint_36(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_35(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        DirectionDetector direction;
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                direction.change_direction(env.program_info(), env.console, context, 0.3491);
                pbf_move_left_joystick(context, 128, 0, 400, 100);
                direction.change_direction(env.program_info(), env.console, context, 5.075911);
                pbf_move_left_joystick(context, 128, 0, 525, 100);                
        });

        direction.change_direction(env.program_info(), env.console, context, 3.771252);
        get_on_ride(env.program_info(), env.console, context);
        // walk towards elevator
        pbf_move_left_joystick(context, 128, 0, 700, 100);
        // jump to ensure you get on elevator
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 200);
        pbf_wait(context, 3 * TICKS_PER_SECOND);
        // wait for overworld to reappear after stepping off elevator
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_left_joystick(context, 128, 0, 120, 100);     
        direction.change_direction(env.program_info(), env.console, context, 5.11);  
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);
        mash_button_till_overworld(env.console, context, BUTTON_A);
       
    });

}

void checkpoint_36(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 100, 0, 80});

        // section 1
        // warning: can't reliably set the marker when in Cascarrafa, possibly due to too many NPCs. worse when sandstorm is up.
        // what happens is that the program doesn't reliably push the cursor as much as it should
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 135, 410);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 60, 20, false);
        // talk to Arven over phone
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 80},
            {ZoomChange::ZOOM_IN, 255, 255, 110}
        );
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 60, 20, false);

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 40},
            {ZoomChange::ZOOM_IN, 255, 128, 45}
        );
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 10, false);
        
        // cutscene with Kofu looking at flowers
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);            

        // section 4. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0},
            {ZoomChange::ZOOM_IN, 0, 0, 0}
        );      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);   

        // section 5. set marker past pokecenter
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 100, 30);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);     

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);           
       
    });

}




}
}
}
