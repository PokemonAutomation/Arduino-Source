/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_15.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_15::name() const{
    return "15: Team Star (Dark)";
}

std::string AutoStory_Segment_15::start_text() const{
    return "Start: Defeated Bombirder. At West Province Area One North Pokecenter.";
}

std::string AutoStory_Segment_15::end_text() const{
    return "End: Defeated Team Star (Dark). At Cascarrafa (West) Pokecenter.";
}

void AutoStory_Segment_15::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_32(env, context, options.notif_status_update, stats);
    checkpoint_33(env, context, options.notif_status_update, stats);
    checkpoint_34(env, context, options.notif_status_update, stats);
   

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}



void checkpoint_32(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        // section 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 50, 0, 25);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);           

        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 0, 80},
            {ZoomChange::ZOOM_IN, 5, 230, 145}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);        

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 0, 60},
            {ZoomChange::ZOOM_IN, 5, 205, 100}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false); 

        // section 4
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 128, 255, 40},
            {ZoomChange::KEEP_ZOOM, 255, 0, 110}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);        

        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // section 5
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 255, 50);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 30, false);       

        // battle team star grunts
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Team star grunt.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX});

       
    });

}

void checkpoint_33(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();

        // enter the base
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 255, 50);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 
            128, 0, 20, 20, false);       

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG, CallbackEnum::TUTORIAL});
        AdvanceDialogWatcher    dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){

                DirectionDetector direction;
                uint16_t seconds_wait = 6;

                pbf_move_left_joystick(context, 128, 0, 250, 100);
                direction.change_direction(env.program_info(), env.console, context, 3);
                pbf_move_left_joystick(context, 128, 0, 10, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 4.1);
                pbf_move_left_joystick(context, 128, 0, 750, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 5.04);
                pbf_move_left_joystick(context, 128, 0, 250, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                pbf_move_left_joystick(context, 128, 0, 500, 100);
                direction.change_direction(env.program_info(), env.console, context, 5.39);
                pbf_move_left_joystick(context, 128, 0, 10, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 5.076);
                pbf_move_left_joystick(context, 128, 0, 500, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                pbf_move_left_joystick(context, 128, 0, 250, 100);
                direction.change_direction(env.program_info(), env.console, context, 4.800);
                pbf_move_left_joystick(context, 128, 0, 250, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                pbf_move_left_joystick(context, 128, 0, 500, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 5.32);
                pbf_move_left_joystick(context, 128, 0, 250, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);                

                direction.change_direction(env.program_info(), env.console, context, 6.16);
                pbf_move_left_joystick(context, 128, 0, 250, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);   


                direction.change_direction(env.program_info(), env.console, context, 0.541);
                pbf_move_left_joystick(context, 128, 0, 500, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);    

                direction.change_direction(env.program_info(), env.console, context, 1.41);
                pbf_move_left_joystick(context, 128, 0, 350, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);         

                direction.change_direction(env.program_info(), env.console, context, 2.34);
                pbf_move_left_joystick(context, 128, 0, 250, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);     

                direction.change_direction(env.program_info(), env.console, context, 1.556);
                pbf_move_left_joystick(context, 128, 0, 500, 100);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);   
                
            },
            {dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "checkpoint_33(): Failed to kill 30 pokemon with Let's go.",
                env.console
            );            
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle the Team Star (Dark) boss.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);


    });

}

void checkpoint_34(
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
            {ZoomChange::KEEP_ZOOM, 0, 128, 40},
            {ZoomChange::ZOOM_IN, 230, 0, 100}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 10, false);   
        
        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 100, 30},
            {ZoomChange::ZOOM_IN, 0, 240, 40}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);        

        // section 3. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0},
            {ZoomChange::ZOOM_IN, 0, 0, 0}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);    

        // section 4. set marker past pokecenter
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 40, 100);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);     

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
    });

}







}
}
}
