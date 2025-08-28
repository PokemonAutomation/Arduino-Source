/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_10.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_10::name() const{
    return "10: Cortondo Gym (Bug): Go to Cortondo city";
}

std::string AutoStory_Segment_10::start_text() const{
    return "Start: After the break, with level 100 Gardevoir. At Mesagoza West pokecenter.";
}

std::string AutoStory_Segment_10::end_text() const{
    return "End: At Cortondo East Pokecenter.";
}

void AutoStory_Segment_10::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_21(env, context, options.notif_status_update, stats);
    checkpoint_22(env, context, options.notif_status_update, stats);
    checkpoint_23(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

void checkpoint_21(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        fly_to_overlapping_flypoint(env.program_info(), env.console, context);

        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_L, 20, 20);
        // move forward
        pbf_move_left_joystick(context, 128, 0, 30, 100);
        // get on ride
        get_on_ride(env.program_info(), env.console, context);
        // turn left
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 0, 128, 50);
        // move forward
        pbf_move_left_joystick(context, 128, 0, 100, 100);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 200, 70);
        pbf_move_left_joystick(context, 128, 0, 400, 100);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 128, 70);
        pbf_move_left_joystick(context, 128, 0, 700, 100);

        // turn towards wall
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 50);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        // run and jump over wall
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 100);

        BlackScreenOverWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(30),
            { black_screen }
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "checkpoint_21(): Failed to jump the East Mesagoza wall.",
                env.console
            );
        }
        context.wait_for_all_requests();
        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
    });

}

void checkpoint_22(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        // section 1
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 80},
            {ZoomChange::KEEP_ZOOM, 255, 80, 37}            
        );
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 20);

        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 40},
            {ZoomChange::KEEP_ZOOM, 255, 255, 27}            
        );        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 15);
        
        // section 3. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 128, 128, 0},
            {ZoomChange::KEEP_ZOOM, 128, 128, 0}            
        );         
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 15);

        // section 3. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 70, 30);
        });            
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 20, 12, 12);        

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
    });

}



void checkpoint_23(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();

        // section 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 110, 0, 30);

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10);

        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 80},
            {ZoomChange::KEEP_ZOOM, 255, 95, 100}            
        );         
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 10);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );                

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 80},
            {ZoomChange::KEEP_ZOOM, 255, 75, 65}            
        );              
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 10);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );                     

        // section 4
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 50},
            {ZoomChange::KEEP_ZOOM, 255, 180, 17}            
        );              
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 10);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );                

        // section 5. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 128, 128, 0},
            {ZoomChange::KEEP_ZOOM, 128, 128, 0}            
        );              
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 10); 
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );                

        // section 6. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 110, 50);
        });   
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12);   

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);             
       
    });

}



}
}
}
