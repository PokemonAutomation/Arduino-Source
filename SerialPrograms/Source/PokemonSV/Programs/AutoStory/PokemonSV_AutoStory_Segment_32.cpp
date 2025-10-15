/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_32.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_32::name() const{
    return "32: Alfornada Gym (Psychic)";
}

std::string AutoStory_Segment_32::start_text() const{
    return "Start: Beat Team Star (Fighting). At North Province Area Two Pokecenter.";
}

std::string AutoStory_Segment_32::end_text() const{
    return "End: Beat Alfornada Gym (Psychic). At Alfornada Pokecenter.";
}

void AutoStory_Segment_32::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_82(env, context, options.notif_status_update, stats);
    checkpoint_83(env, context, options.notif_status_update, stats);
    checkpoint_84(env, context, options.notif_status_update, stats);
    checkpoint_85(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

void checkpoint_82(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });  
}

void checkpoint_83(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });  
}

void checkpoint_84(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });  
}

void checkpoint_85(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });  
}


void move_from_west_province_area_one_north_to_alfornada(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    DirectionDetector direction;

    do_action_and_monitor_for_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        // marker 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 140, 255, 30);
        direction.change_direction(env.program_info(), env.console, context, 4.047990);
        pbf_move_left_joystick(context, 128, 0, 200, 50);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
    });

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 2      zoom in{0.605729, 0.30463}, zoom out{0.684375, 0.616667}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.684375, 0.616667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 24, 8, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 3  {0.767708, 0.45}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 255, 60}, 
        FlyPoint::POKECENTER, 
        {0.767708, 0.45}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 60, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 4   {0.763021, 0.253704}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.763021, 0.253704}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 5  {0.780729, 0.216667}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.780729, 0.216667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6. place the marker at the top of the cliff. {0.633333, 0.304630}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.633333, 0.304630}
    );
    
    do_action_and_monitor_for_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        // walk towards wall
        direction.change_direction(env.program_info(), env.console, context, 2.949863);

        walk_forward_while_clear_front_path(env.program_info(), env.console, context, 700);

        // back away from wall and get on ride
        pbf_move_left_joystick(context, 128, 255, 50, 50);
        get_on_ride(env.program_info(), env.console, context);

        // move back towards wall
        pbf_move_left_joystick(context, 128, 0, 100, 50);
        pbf_move_left_joystick(context, 0, 0, 400, 50);

        direction.change_direction(env.program_info(), env.console, context, 2.566167);

        // climb the wall
        pbf_press_button(context, BUTTON_B, 31ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 855ms);
        pbf_move_left_joystick(context, 128, 0, 7905ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 779ms);
        pbf_move_left_joystick(context, 128, 0, 325ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 657ms);
        pbf_move_left_joystick(context, 128, 0, 3271ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 860ms);
        pbf_move_left_joystick(context, 128, 0, 220ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 993ms);
        pbf_move_left_joystick(context, 128, 0, 3572ms, 0ms);

        get_off_ride(env.program_info(), env.console, context);

        // realign to marker 6
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        
    });

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 7 {0.501042, 0.738889}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 200, 30}, 
        FlyPoint::POKECENTER, 
        {0.501042, 0.738889}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 255, 30},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}
    ); 

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 9. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 120, 255, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


    

}







void checkpoint_86(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_87(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_88(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_89(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_90(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}



}
}
}
