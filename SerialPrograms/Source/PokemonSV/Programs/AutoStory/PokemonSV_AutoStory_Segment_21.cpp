/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "CommonTools/Async/InferenceRoutines.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_21.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_21::name() const{
    return "21: Team Star (Fire)";
}

std::string AutoStory_Segment_21::start_text() const{
    return "Start: Defeated Artazon Gym (Grass). At East Province (Area One) Pokecenter.";
}

std::string AutoStory_Segment_21::end_text() const{
    return "End: Defeated Team Star (Fire). At East Province (Area Two) Pokecenter.";
}

void AutoStory_Segment_21::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_47(env, context, options.notif_status_update, stats);
    checkpoint_48(env, context, options.notif_status_update, stats);
    checkpoint_49(env, context, options.notif_status_update, stats);    

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_47(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 110, 100);

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 2.06);
        pbf_move_left_joystick(context, 128, 0, 200, 100);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 60, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // speak to Clive and Cassiopeia
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // move towards Team Star base gate
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 20, 255, 40);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 20, false);

        // battle Team Star Grunt
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW, CallbackEnum::BATTLE});
        env.console.log("Battle team star grunt.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

       
    });

}


void checkpoint_48(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 240, 50);
                walk_forward_while_clear_front_path(env.program_info(), env.console, context, 300);
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
            }
        );
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG, CallbackEnum::TUTORIAL});

        AdvanceDialogWatcher    dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){

                DirectionDetector direction;
                uint16_t seconds_wait = 8;

                direction.change_direction(env.program_info(), env.console, context, 2.50);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


                direction.change_direction(env.program_info(), env.console, context, 3.54);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


                direction.change_direction(env.program_info(), env.console, context, 1.76);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                pbf_move_left_joystick(context, 128, 0, 400, 50);


                direction.change_direction(env.program_info(), env.console, context, 1.97);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);                

                direction.change_direction(env.program_info(), env.console, context, 2.60);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);  


                direction.change_direction(env.program_info(), env.console, context, 0.19);
                pbf_move_left_joystick(context, 128, 0, 400, 50);


                direction.change_direction(env.program_info(), env.console, context, 0.82);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);   

                direction.change_direction(env.program_info(), env.console, context, 2.13);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);   


                direction.change_direction(env.program_info(), env.console, context, 1.97);
                pbf_move_left_joystick(context, 128, 0, 500, 50);   
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);              

                direction.change_direction(env.program_info(), env.console, context, 3.02);
                pbf_move_left_joystick(context, 128, 0, 400, 50);


                direction.change_direction(env.program_info(), env.console, context, 3.87);
                pbf_move_left_joystick(context, 128, 0, 200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);   

                direction.change_direction(env.program_info(), env.console, context, 4.56);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);      

                direction.change_direction(env.program_info(), env.console, context, 4.98);
                pbf_move_left_joystick(context, 128, 0, 400, 50);

                direction.change_direction(env.program_info(), env.console, context, 5.18);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);     

                direction.change_direction(env.program_info(), env.console, context, 5.66);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);  

                direction.change_direction(env.program_info(), env.console, context, 5.24);
                pbf_move_left_joystick(context, 128, 0, 600, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);  

                direction.change_direction(env.program_info(), env.console, context, 5.45);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);                 
                  
                
            },
            {dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "checkpoint_48(): Failed to kill 30 pokemon with Let's go.",
                env.console
            );            
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle the Team Star (Fire) boss.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

    });

}


void checkpoint_49(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();
        // marker 1
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 128, 30},
            {ZoomChange::ZOOM_IN, 0, 90, 90}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );  

        // marker 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 128, 0, 30},
            {ZoomChange::ZOOM_IN, 128, 255, 30}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );   
        
        // marker 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 180, 50},
            {ZoomChange::ZOOM_IN, 0, 70, 175}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );      
        
        // marker 4
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 180, 50},
            {ZoomChange::ZOOM_IN, 0, 50, 185}
        );  
        // walk until you run into the wall
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 10, 10, false);        
  
        
        // marker 5. put marker on other side of bridge
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 128, 128, 0},
            {ZoomChange::ZOOM_IN, 128, 0, 10}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );         
        

        // marker 6. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 255, 30);
        });      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
          

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);         


       
    });

}



}
}
}
