/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteTriangleDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "CommonTools/Async/InferenceRoutines.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_22.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_22::name() const{
    return "22: Levincia Gym (Electric)";
}

std::string AutoStory_Segment_22::start_text() const{
    return "Start: Defeated Team Star (Fire). At East Province (Area Two) Pokecenter.";
}

std::string AutoStory_Segment_22::end_text() const{
    return "End: Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.";
}

void AutoStory_Segment_22::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_50(env, context, options.notif_status_update, stats);
    checkpoint_51(env, context, options.notif_status_update, stats);
    checkpoint_52(env, context, options.notif_status_update, stats);
    checkpoint_53(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

void checkpoint_50(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        
        context.wait_for_all_requests();
        // set down marker 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 60, 50);

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 0);
        pbf_move_left_joystick(context, 128, 0, 150, 100);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 60, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, 0, 128, 128, 255);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );
        
        get_off_ride(env.program_info(), env.console, context);

        // marker 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 50, 50},
            {ZoomChange::ZOOM_IN, 0, 100, 60}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );      

        get_on_ride(env.program_info(), env.console, context);
        
        // marker 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 128, 20},
            {ZoomChange::ZOOM_IN, 0, 0, 0}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                // jump over the fence when stationary
                context.wait_for_all_requests();
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 100);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );          

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);


    });

}


void checkpoint_51(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 128, 50);
        pbf_move_left_joystick(context, 128, 0, 400, 100);        
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 170, 0, 60);
        pbf_move_left_joystick(context, 128, 0, 1800, 100);       
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 85, 60);
        
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 0, 100, 50);   // move forward/left
            },
            5, 5
        );         

        // enter gym building. talk to Hassel, meet Rika
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::OVERWORLD}); 


    });    

}

void checkpoint_52(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // talk to receptionist
        env.console.log("Talk to Levincia gym receptionist.");
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});    
        
        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for dialog after leaving gym
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20, 128, 255);
        
        WhiteTriangleWatcher white_triangle(COLOR_RED, ImageFloatBox(0.948773, 0.034156, 0.013874, 0.024668));
        // mash A until detect top right white triangle 1
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {white_triangle}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect white triangle in top right, which is an indicator of the Levincia Hide-and-Seek gym challenge.",
                env.console
            );
        }
        env.console.log("Detected white triangle in top right. Assume we are in the Levincia Hide-and-Seek gym challenge.");

        // select Clavell 1
        pbf_move_left_joystick(context, 255, 0, 2000ms, 480ms);
        pbf_move_left_joystick(context, 10, 255, 100, 100);
        pbf_mash_button(context, BUTTON_A, 1000ms);

        // mash B until detect battle 1
        NormalBattleMenuWatcher battle(COLOR_BLUE);
        ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 100000ms); // press B so we don't mash past the Battle menu
            },
            {battle}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect white triangle in top right, which is an indicator of the Levincia Hide-and-Seek gym challenge.",
                env.console
            );
        }        

        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        // mash B until detect top right white triangle 2
        ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 100000ms); // press B so we don't accidentally select random guy
            },
            {white_triangle}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect white triangle in top right, which is an indicator of the Levincia Hide-and-Seek gym challenge.",
                env.console
            );
        }        
        env.console.log("Detected white triangle in top right. Assume we are in the Levincia Hide-and-Seek gym challenge.");

        // select Clavell 2
        pbf_move_left_joystick(context, 0, 0, 3000ms, 480ms);
        pbf_move_left_joystick(context, 255, 250, 100, 100);
        pbf_mash_button(context, BUTTON_A, 1000ms);

        // mash B until detect battle 2
        ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 100000ms); // press B so we don't mash past the Battle menu
                // for (size_t i = 0; i < 200; i++){
                //     pbf_press_button(context, BUTTON_A, 100, 100);
                // }
            },
            {battle}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect white triangle in top right, which is an indicator of the Levincia Hide-and-Seek gym challenge.",
                env.console
            );
        }        

        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);        

        // mash B until detect top right white triangle 3
        ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 100000ms);
            },
            {white_triangle}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect white triangle in top right, which is an indicator of the Levincia Hide-and-Seek gym challenge.",
                env.console
            );
        }        

        env.console.log("Detected white triangle in top right. Assume we are in the Levincia Hide-and-Seek gym challenge.");

        // select Clavell 3
        pbf_move_left_joystick(context, 255, 0, 2000ms, 480ms);       
        pbf_move_left_joystick(context, 85, 255, 80, 100); 

        mash_button_till_overworld(env.console, context, BUTTON_A);



    });

}

void checkpoint_53(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        // realign camera. 
        pbf_press_button(context, BUTTON_L, 30, 30);

        // walk backwards into the Gym building
        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        

        // talk to Gym receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30);

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Electric Gym leader.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // Gym leader defeated. Standing in Gym building
        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);
        // wait for overworld after leaving Gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        // fly to Levincia (North) Pokecenter
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 100, 0, 50});

    });

}


}
}
}
