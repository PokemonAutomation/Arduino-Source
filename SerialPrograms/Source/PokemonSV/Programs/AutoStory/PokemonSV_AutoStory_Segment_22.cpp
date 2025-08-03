/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
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
    return "22: Team Star (Fire): Beat Team Star";
}

std::string AutoStory_Segment_22::start_text() const{
    return "Start: At East Province (Area One) Pokecenter.";
}

std::string AutoStory_Segment_22::end_text() const{
    return "End: ";
}

void AutoStory_Segment_22::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options
) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_47(env, context, options.notif_status_update);
    checkpoint_48(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_47(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }

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

       
        break;
    }catch(OperationFailedException&){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}


void checkpoint_48(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }

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

        break;
    }catch(OperationFailedException&){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}


// todo: uncomment checkpoint_save
void checkpoint_49(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            // checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        context.wait_for_all_requests();


       
        break;
    }catch(OperationFailedException&){
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
