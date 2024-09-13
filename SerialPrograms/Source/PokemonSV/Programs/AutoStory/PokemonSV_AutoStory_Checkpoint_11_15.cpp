/*  AutoStoryTools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_SnapshotDumper.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonMovesReader.h"
#include "PokemonSV/Inference/Map/PokemonSV_DestinationMarkerDetector.h"
#include "PokemonSV_AutoStoryTools.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



void checkpoint_11(
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
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 210, 100);
                pbf_move_left_joystick(context, 128, 0, 187, 20);
                pbf_move_left_joystick(context, 0, 128, 30, 8 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 0, 1 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 60, 200);                
            }
        );     

        env.console.log("overworld_navigation: Go to Los Platos.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 75);

        env.console.log("clear_dialog: Talk with Nemona at Los Platos. Clear Let's go tutorial. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::TUTORIAL, ClearDialogCallback::OVERWORLD});

        context.wait_for_all_requests();

        env.console.log("Reached Los Platos");
        env.console.overlay().add_log("Reached Los Platos", COLOR_WHITE);

        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}


void checkpoint_12(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    // reset rate: ~25%. 12 resets out of 52. 
    // resets due to: getting attacked by wild pokemon, either from behind, 
    // or when lead pokemon not strong enough to clear them with Let's go
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){        
        
            if (first_attempt){
                checkpoint_save(env, context, notif_status_update);
                first_attempt = false;
            } 

            context.wait_for_all_requests();
            
            fly_to_overlapping_flypoint(env.program_info(), env.console, context);

            // re-orient camera
            pbf_press_button(context, BUTTON_L, 20, 20);

            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 35);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 500);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 0, 120);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 2000);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 0, 120);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 1250);
            
            // check we're not still at the Los Platos Pokecenter.
            confirm_no_overlapping_flypoint(env.program_info(), env.console, context);

            // not stuck at Los Platos Pokecenter
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            press_Bs_to_back_to_overworld(env.program_info(), env.console, context, 7);

            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 80, 0, 100);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 1500);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 30, 0, 50);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 1000);

            heal_at_pokecenter(env.program_info(), env.console, context);
   
        }  
        );

        env.console.log("Reached Mesagoza (South) Pokecenter.");
        
        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}

void checkpoint_13(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    // reset rate: 0%. 0 resets out of 70.
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        do_action_and_monitor_for_battles(env, env.console, context,
        [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){        
        
            if (first_attempt){
                checkpoint_save(env, context, notif_status_update);
                first_attempt = false;
            } 

            fly_to_overlapping_flypoint(env.program_info(), env.console, context);

            context.wait_for_all_requests();
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 80, 50);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 500);
            walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);
        });

        env.console.log("clear_dialog: Talk with Nemona at Mesagoza gate. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60,
            {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::DIALOG_ARROW, ClearDialogCallback::BATTLE});
        
        env.console.log("run_battle_press_A: Battle with Nemona at Mesagoza gate. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        
        env.console.log("clear_dialog: Talk with Nemona within Mesagoza. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::OVERWORLD, ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::WHITE_A_BUTTON});
        
       
        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}

void checkpoint_14(
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
        // realign diagonally to the left
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 80, 0, 100);
        // walk forward so you're off center
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
        // walk forward, while still off center
        pbf_move_left_joystick(context, 128, 0, 2000, 100);
        // realign diagonally to the right
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 178, 0, 100);
        // walk forward so you're closer to the center
        pbf_move_left_joystick(context, 128, 0, 150, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
        // walk forward until hit dialog at top of stairs
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60);
        // clear dialog until battle. with prompt, battle
        env.console.log("clear_dialog: Talk with Team Star at the top of the stairs. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::BATTLE, ClearDialogCallback::DIALOG_ARROW});
        // run battle until dialog
        env.console.log("run_battle_press_A: Battle with Team Star grunt 1. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        // clear dialog until battle, with prompt, white button, tutorial, battle
        env.console.log("clear_dialog: Talk with Team Star and Nemona. Receive Tera orb. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, 
            {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::WHITE_A_BUTTON, ClearDialogCallback::TUTORIAL, ClearDialogCallback::BATTLE, ClearDialogCallback::DIALOG_ARROW});
        // run battle until dialog
        env.console.log("run_battle_press_A: Battle with Team Star grunt 2. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        // clear dialog until overworld
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::OVERWORLD});
       
        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}

void checkpoint_15(
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
        // realign diagonally to the right
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 178, 0, 100);
        // walk forward so you're closer to the center
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
        // walk forward up stairs
        pbf_move_left_joystick(context, 128, 0, 1000, 100);
        // realign going straight
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        // walk forward until hit dialog inside the school
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60);

        env.console.log("clear_dialog: Talk with Nemona, Clavell, and Jacq inside the school. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::PROMPT_DIALOG, ClearDialogCallback::OVERWORLD});
       
        break;
    }catch(OperationFailedException& e){
        context.wait_for_all_requests();
        env.console.log(e.m_message, COLOR_RED);
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
