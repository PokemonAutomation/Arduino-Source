/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_20.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




std::string AutoStory_Segment_20::name() const{
    return "16: Artazon Gym (Grass)";
}

std::string AutoStory_Segment_20::start_text() const{
    return "Start: Defeated Klawf. At Artazon (West) Pokecenter.";
}

std::string AutoStory_Segment_20::end_text() const{
    return "End: Defeated Artazon Gym (Grass). Inside gym building.";
}

void AutoStory_Segment_20::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.overlay().add_log("Start Segment ", COLOR_ORANGE);

    checkpoint_43(env, context, options.notif_status_update);
    checkpoint_44(env, context, options.notif_status_update);
    checkpoint_45(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment ", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats();

}


void checkpoint_43(
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
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }
        context.wait_for_all_requests();

        // place the marker somewhere else. the current location disrupts the Stationary detector
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 128, 50);

        DirectionDetector direction;
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                direction.change_direction(env.program_info(), env.console, context, 6.198);
                pbf_move_left_joystick(context, 128, 0, 400, 100);
                direction.change_direction(env.program_info(), env.console, context, 4.693);
                pbf_move_left_joystick(context, 128, 0, 1000, 100);
        });
        // walk up right set of stairs
        direction.change_direction(env.program_info(), env.console, context, 4.276);
        pbf_move_left_joystick(context, 128, 0, 700, 100);

        // realign using lamp-post
        direction.change_direction(env.program_info(), env.console, context, 2.34);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.81);
        pbf_move_left_joystick(context, 255, 0, 600, 100);

        // move toward gym building
        direction.change_direction(env.program_info(), env.console, context, 4.26);
        pbf_move_left_joystick(context, 128, 0, 900, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.05);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        pbf_wait(context, 7 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){           
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);
            }, 
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){           
                pbf_move_left_joystick(context, 0, 0, 100, 50);
            },
            5, 5
        );          

        // enter gym building. talk go Nemona and battle her.
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BLACK_DIALOG_BOX, CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW, CallbackEnum::BATTLE});        
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::GRADIENT_ARROW});
        mash_button_till_overworld(env.console, context, BUTTON_A);

       
        break;
    }catch (...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}

void checkpoint_44(
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
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        context.wait_for_all_requests();

        // talk to receptionist
        env.console.log("Talk to Artazon gym receptionist.");
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);      
        
        // talk to Sunflora NPC
        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 4.91);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        // realign to wall
        direction.change_direction(env.program_info(), env.console, context, 1.477);
        pbf_move_left_joystick(context, 128, 0, 500, 100);
        direction.change_direction(env.program_info(), env.console, context, 2.166);
        pbf_move_left_joystick(context, 0, 0, 300, 100);

        // get sunflora 1
        direction.change_direction(env.program_info(), env.console, context, 4.85);
        pbf_move_left_joystick(context, 128, 0, 300, 100);        
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 1);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );

        // get sunflora 2
        direction.change_direction(env.program_info(), env.console, context, 0.384);
        pbf_move_left_joystick(context, 128, 0, 120, 100);
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 2);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );        

        // get sunflora 3
        direction.change_direction(env.program_info(), env.console, context, 5.377);
        pbf_move_left_joystick(context, 128, 0, 120, 100);
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 3);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );              

        get_on_ride(env.program_info(), env.console, context);

        // get sunflora 4
        // align to corner 4.1
        direction.change_direction(env.program_info(), env.console, context, 1.90);
        pbf_move_left_joystick(context, 128, 0, 200, 100);    
        direction.change_direction(env.program_info(), env.console, context, 2.166);
        pbf_move_left_joystick(context, 0, 0, 300, 100);

        // align to corner 4.2
        direction.change_direction(env.program_info(), env.console, context, 6.056);
        pbf_move_left_joystick(context, 128, 0, 670, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.22);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.69);
        pbf_move_left_joystick(context, 0, 0, 500, 100);

        direction.change_direction(env.program_info(), env.console, context, 5.85);
        pbf_move_left_joystick(context, 128, 0, 60, 100);
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 4);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );        

        // get sunflora 5
        // align to corner 5.1
        direction.change_direction(env.program_info(), env.console, context, 1.59);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.79);
        pbf_move_left_joystick(context, 0, 0, 500, 100);        
        direction.change_direction(env.program_info(), env.console, context, 6.055);        
        pbf_move_left_joystick(context, 128, 0, 400, 100);
        direction.change_direction(env.program_info(), env.console, context, 5.06);   
        pbf_move_left_joystick(context, 128, 0, 600, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.38);   
        pbf_move_left_joystick(context, 0, 0, 700, 100);

        direction.change_direction(env.program_info(), env.console, context, 2.53);   
        pbf_move_left_joystick(context, 128, 0, 160, 100);
        direction.change_direction(env.program_info(), env.console, context, 0.78);   
        pbf_move_left_joystick(context, 128, 0, 90, 100);  // todo: adjust this. 80 -> 90?
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 5);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );        

        // sunflora 6
        // align to corner 6.1
        direction.change_direction(env.program_info(), env.console, context, 4.2);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.38);   
        pbf_move_left_joystick(context, 0, 0, 700, 100);        

        direction.change_direction(env.program_info(), env.console, context, 0.96);
        pbf_move_left_joystick(context, 128, 0, 300, 100);
        direction.change_direction(env.program_info(), env.console, context, 5.17);        
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.86);
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 6);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );        

        // sunflora 7
        // align to corner 7.1
        direction.change_direction(env.program_info(), env.console, context, 2.06);
        pbf_move_left_joystick(context, 128, 0, 80, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.7); 
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        direction.change_direction(env.program_info(), env.console, context, 5.01);               
        pbf_move_left_joystick(context, 128, 0, 1600, 100);        

        // align to corner 7.2. bush
        direction.change_direction(env.program_info(), env.console, context, 2.34);
        pbf_move_left_joystick(context, 128, 0, 700, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.42);
        pbf_move_left_joystick(context, 128, 0, 400, 100);

        // align to corner 7.3. lamp-post
        // todo: adjust routine to get to lampost. go behind the sculpture and goat?
        direction.change_direction(env.program_info(), env.console, context, 0);
        pbf_move_left_joystick(context, 128, 0, 120, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.75);
        pbf_move_left_joystick(context, 128, 0, 400, 100);
        direction.change_direction(env.program_info(), env.console, context, 2.95);
        pbf_move_left_joystick(context, 128, 0, 300, 100);
        direction.change_direction(env.program_info(), env.console, context, 2.22);
        pbf_move_left_joystick(context, 128, 0, 400, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.01);
        pbf_move_left_joystick(context, 128, 0, 300, 100);

        // align to corner 7.4. wall corner
        direction.change_direction(env.program_info(), env.console, context, 3.70);
        pbf_move_left_joystick(context, 128, 0, 400, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.28);
        pbf_move_left_joystick(context, 128, 0, 400, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.54);
        pbf_move_left_joystick(context, 128, 0, 600, 100);

        // align to corner 7.5. bush
        direction.change_direction(env.program_info(), env.console, context, 3.11);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 150);
        pbf_move_left_joystick(context, 128, 0, 60, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.28);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.60);
        pbf_move_left_joystick(context, 128, 0, 400, 100);

        direction.change_direction(env.program_info(), env.console, context, 1.17);
        pbf_move_left_joystick(context, 128, 0, 130, 100);
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 7);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );           

        // sunflora 8
        // align to corner 8.1. bush
        direction.change_direction(env.program_info(), env.console, context, 4.54);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 150);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.60);
        pbf_move_left_joystick(context, 128, 0, 400, 100);

        direction.change_direction(env.program_info(), env.console, context, 1.64);
        pbf_move_left_joystick(context, 128, 0, 350, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.36);
        pbf_move_left_joystick(context, 128, 0, 100, 100);

        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_press_button(context, BUTTON_A, 50, 50);
                pbf_press_button(context, BUTTON_A, 50, 50); // extra press in case one is dropped
                pbf_press_button(context, BUTTON_A, 50, 50);
                pbf_wait(context, 250);
                press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
                check_num_sunflora_found(env, context, 8);  
                pbf_wait(context, 3 * TICKS_PER_SECOND);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );        

        // // sunflora 9
        // // align to corner 9.1. bush
        pbf_move_left_joystick(context, 128, 255, 200, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.89);
        pbf_move_left_joystick(context, 128, 0, 350, 100);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 180);
        get_off_ride(env.program_info(), env.console, context);
        direction.change_direction(env.program_info(), env.console, context, 3.60);
        pbf_move_left_joystick(context, 128, 0, 600, 100);


        direction.change_direction(env.program_info(), env.console, context, 1.48);
        pbf_move_left_joystick(context, 128, 0, 50, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.11);
        pbf_move_left_joystick(context, 128, 0, 180, 100);
        direction.change_direction(env.program_info(), env.console, context, 4.75);
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        get_on_ride(env.program_info(), env.console, context);
        direction.change_direction(env.program_info(), env.console, context, 5.53);
        pbf_move_left_joystick(context, 128, 0, 600, 100);
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 9);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );

        // sunflora 10
        // align to corner 10.1. bush
        pbf_move_left_joystick(context, 0, 128, 200, 100);

        direction.change_direction(env.program_info(), env.console, context, 4.02);
        pbf_move_left_joystick(context, 128, 0, 250, 100);
        handle_failed_action(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){ 
                pbf_mash_button(context, BUTTON_A, 500);
                check_num_sunflora_found(env, context, 10);
            },
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30, 100);
            },
            3
        );

        // go back to Sunflora NPC
        // align to corner 11.1. bush
        direction.change_direction(env.program_info(), env.console, context, 0.65);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 150);
        pbf_move_left_joystick(context, 128, 0, 800, 100);

        direction.change_direction(env.program_info(), env.console, context, 4.49);
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        direction.change_direction(env.program_info(), env.console, context, 5.53);

        NoMinimapWatcher no_minimap(env.console, COLOR_RED, Milliseconds(5000));
        int ret = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 30 * TICKS_PER_SECOND, 100);
            },
            {no_minimap}
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT,
                env.logger(),
                "Failed to finish reach the Sunflora NPC."
            );
        }
        env.log("No minimap seen. Likely finished the Artazon gym challenge.");

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

       
        break;
    }catch (...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}

void checkpoint_45(
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
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }
        
        context.wait_for_all_requests();

        get_on_ride(env.program_info(), env.console, context);

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 1.52);
        pbf_move_left_joystick(context, 128, 0, 400, 100);    
        direction.change_direction(env.program_info(), env.console, context, 2.62);
        pbf_move_left_joystick(context, 128, 0, 400, 100);

        direction.change_direction(env.program_info(), env.console, context, 2.18);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 150);
        pbf_move_left_joystick(context, 128, 0, 400, 100);
        direction.change_direction(env.program_info(), env.console, context, 3.16);
        pbf_move_left_joystick(context, 0, 0, 100, 50);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){           
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20);
            }, 
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){   
                pbf_move_left_joystick(context, 0, 0, 100, 50);
                pbf_move_left_joystick(context, 255, 0, 100, 50);
            },
            5, 2, 2
        ); 

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::GRADIENT_ARROW});
        mash_button_till_overworld(env.console, context, BUTTON_A);
       
        break;
    }catch (...){
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
