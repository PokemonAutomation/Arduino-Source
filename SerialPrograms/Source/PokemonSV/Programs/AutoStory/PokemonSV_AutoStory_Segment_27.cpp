/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_27.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_27::name() const{
    return "27: Dondozo/Tatsugiri Titan";
}

std::string AutoStory_Segment_27::start_text() const{
    return "Start: Beat Medali Gym (Normal). At Glaseado Mountain Pokecenter.";
}

std::string AutoStory_Segment_27::end_text() const{
    return "End: Beat Dondozo/Tatsugiri Titan. At North Province Area Three Pokecenter.";
}

void AutoStory_Segment_27::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_64().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_65().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_66().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_67().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_64::name() const{ return "064 - " + AutoStory_Segment_27().name(); }
std::string AutoStory_Checkpoint_64::start_text() const{ return "At Glaseado Mountain Pokecenter";}
std::string AutoStory_Checkpoint_64::end_text() const{ return "At Casseroya Watchtower 3";}
void AutoStory_Checkpoint_64::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_64(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_65::name() const{ return "065 - " + AutoStory_Segment_27().name(); }
std::string AutoStory_Checkpoint_65::start_text() const{ return AutoStory_Checkpoint_64().end_text();}
std::string AutoStory_Checkpoint_65::end_text() const{ return "Defeat Dondozo Titan phase 1.";}
void AutoStory_Checkpoint_65::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_65(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_66::name() const{ return "066 - " + AutoStory_Segment_27().name(); }
std::string AutoStory_Checkpoint_66::start_text() const{ return AutoStory_Checkpoint_65().end_text();}
std::string AutoStory_Checkpoint_66::end_text() const{ return "Defeat Dondozo Titan phase 2-3.";}
void AutoStory_Checkpoint_66::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_66(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_67::name() const{ return "067 - " + AutoStory_Segment_27().name(); }
std::string AutoStory_Checkpoint_67::start_text() const{ return AutoStory_Checkpoint_66().end_text();}
std::string AutoStory_Checkpoint_67::end_text() const{ return "At North Province Area Three Pokecenter";}
void AutoStory_Checkpoint_67::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_67(env, context, options.notif_status_update, stats);
}



void checkpoint_64(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        context.wait_for_all_requests();
        move_from_glaseado_mountain_to_casseroya_watchtower3(env, context, attempt_number);

    });         
}

void checkpoint_65(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        context.wait_for_all_requests();
        move_from_casseroya_watchtower3_to_dondozo_titan(env, context);
        
    }); 
}

void checkpoint_66(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        context.wait_for_all_requests();
        move_from_dondozo_titan_phase1_to_phase2(env, context);
        
    }); 
}

void checkpoint_67(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        context.wait_for_all_requests();
        // fly to Glaseado Mountain Pokecenter
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 0, 0, 0}, FlyPoint::POKECENTER);
        move_from_glaseado_mountain_to_north_province_area_three(env, context);

    }); 
}




void move_from_glaseado_mountain_to_casseroya_watchtower3(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t attempt_number){
    context.wait_for_all_requests();

    heal_at_pokecenter(env.program_info(), env.console, context);

    DirectionDetector direction;
    if (attempt_number >= 0){
        day_skip_from_overworld(env.console, context);
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 100, 255, 60});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 128, 0, 60});
    }

    direction.change_direction(env.program_info(), env.console, context, 1.448679);

    pbf_move_left_joystick(context, 128, 0, 200, 50);


    // marker 1   x=0.548438, y=0.273148
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.548438, 0.273148}
    );

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2.    :  x=0.693229, y=0.459259
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.693229, 0.459259}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}


void move_from_casseroya_watchtower3_to_dondozo_titan(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 
    
    // marker 1     x=0.779167, y=0.274074
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.779167, 0.274074}
    );

    // get_on_ride(env.program_info(), env.console, context);
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 0, 20, 10, false);


    mash_button_till_overworld(env.console, context, BUTTON_A);

    // resume marker 1
    realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 0, 20, 10, false);    

    // marker 2.    :   x=0.76875, y=0.298148
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.76875, 0.298148}
    );        
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 0, 30, 10, false);


    // marker 3.    :   x=0.752604, y=0.401852
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.752604, 0.401852}
    );        
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 
        128, 0, 20, 10, false);

    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
    env.console.log("Battle Dondozo/Tatsugiri Titan phase 1.");
    run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);


}

void move_from_dondozo_titan_phase1_to_phase2(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    
    // marker 1      x=0.832292, y=0.54537
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.832292, 0.54537}
    );

    get_on_ride(env.program_info(), env.console, context);

    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 0, 40, 10, false);

    // marker 2       x=0.393229, y=0.748148
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::FAST_TRAVEL, 
        {0.393229, 0.748148}
    );
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 0, 20, 10, false);


    // marker 3. go to Dondozo/Tatsugiri part 2       x=0.55625, y=0.324074
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 255, 40}, 
        FlyPoint::FAST_TRAVEL, 
        {0.55625, 0.324074}
    );
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 0, 80, 40, false);

    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
    
    env.console.log("Battle Dondozo/Tatsugiri Titan phase 2.");
    run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
    
    env.console.log("Battle Dondozo/Tatsugiri Titan phase 3.");
    run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
    mash_button_till_overworld(env.console, context, BUTTON_A, 360);

}


void move_from_glaseado_mountain_to_north_province_area_three(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    
    context.wait_for_all_requests();

    // marker 1. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}
    );  
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 80, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 


    // marker 2. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}



}
}
}
