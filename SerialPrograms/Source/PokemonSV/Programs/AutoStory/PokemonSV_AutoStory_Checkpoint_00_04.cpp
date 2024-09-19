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


void checkpoint_00(SingleSwitchProgramEnvironment& env, BotBaseContext& context){


    // Mash A through intro cutscene, until the L stick button is detected
    WhiteButtonWatcher leftstick(COLOR_GREEN, WhiteButton::ButtonLStick, {0.435, 0.912, 0.046, 0.047});
    context.wait_for_all_requests();
    run_until(
        env.console, context,
        [](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 240 * TICKS_PER_SECOND);
        },
        {leftstick}
    );
    
    // Stand up from chair and walk to left side of room
    pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
    pbf_move_left_joystick(context,   0, 128, 6 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);

}

void checkpoint_01(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update, 
    Language language
){
AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if(first_attempt){
            save_game_tutorial(env.program_info(), env.console, context);
            stats.m_checkpoint++;
            env.update_stats();
            send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");     
            first_attempt = false;
        }
        
        context.wait_for_all_requests();
        // set settings
        enter_menu_from_overworld(env.program_info(), env.console, context, 0, MenuSide::RIGHT, false);
        change_settings(env, context, language, first_attempt);
        pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);

        break;  
    }catch(...){
        // (void)e;
        first_attempt = false;
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }
    }
}

void checkpoint_02(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if(!first_attempt){
            save_game_tutorial(env.program_info(), env.console, context);
            stats.m_checkpoint++;
            env.update_stats();
            send_program_status_notification(env, notif_status_update, "Saved at checkpoint.");     
            first_attempt = false;
        }
        
        context.wait_for_all_requests();
        env.console.log("Go downstairs, get stopped by Skwovet");
        env.console.overlay().add_log("Go downstairs, get stopped by Skwovet", COLOR_WHITE);
        pbf_move_left_joystick(context, 128,   0, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128, 255, 3 * TICKS_PER_SECOND, 20);
        pbf_wait(context, 5 * TICKS_PER_SECOND);
        // clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go to the kitchen, talk with mom");
        env.console.overlay().add_log("Go to the kitchen, talk with mom", COLOR_WHITE);
        pbf_move_left_joystick(context, 128, 255, 2 * TICKS_PER_SECOND, 20);
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128);

        env.console.log("clear_dialog: Talk with Mom.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go to the front door, talk with Clavell");
        env.console.overlay().add_log("Go to the front door, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context, 230, 200, 2 * TICKS_PER_SECOND, 20);
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 255, 128);
        
        env.console.log("clear_dialog: Talk with Clavell at front door.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go upstairs, dress up");
        env.console.overlay().add_log("Go upstairs, dress up", COLOR_WHITE);
        pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 185,  10, 1 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128,   0, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 255, 128, 4 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 110, 200, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 255, 128, 2 * TICKS_PER_SECOND, 20);
        pbf_mash_button(context, BUTTON_A, 20 * TICKS_PER_SECOND);

        context.wait_for_all_requests();
        env.console.log("Go to the living room, talk with Clavell");
        env.console.overlay().add_log("Go to the living room, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context,   0,   0, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context,   0, 128, 3 * TICKS_PER_SECOND, 20);
        pbf_move_left_joystick(context, 128, 255, 4 * TICKS_PER_SECOND, 20);
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 0, 128);
        
        env.console.log("clear_dialog: Talk with Clavell at living room.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});

        context.wait_for_all_requests();
        env.console.log("Go outside, receive Rotom Phone");
        env.console.overlay().add_log("Go outside, receive Rotom Phone", COLOR_WHITE);
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 245, 230);

        env.console.log("clear_dialog: Talk with Clavell outside. Receive Rotom phone. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::OVERWORLD, ClearDialogCallback::WHITE_A_BUTTON});

        context.wait_for_all_requests();
        env.console.log("Bump into power of science NPC");
        // console.overlay().add_log("Bump into power of science NPC", COLOR_WHITE);
        pbf_move_left_joystick(context, 128,   0, 33 * TICKS_PER_SECOND, 20);

        context.wait_for_all_requests();
        env.console.log("Clear map tutorial");
        // console.overlay().add_log("Clear map tutorial", COLOR_WHITE);
        open_map_from_overworld(env.program_info(), env.console, context, true);
        leave_phone_to_overworld(env.program_info(), env.console, context);

        break;  
    }catch(...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }
    }
}

void checkpoint_03(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update,
    Language language,
    StarterChoice starter_choice
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
        
        pbf_move_left_joystick(context, 255, 0, 1 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 156, 1 * TICKS_PER_SECOND);
        env.console.log("overworld_navigation(): Go to Nemona's house.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0);
        
        context.wait_for_all_requests();
        env.console.log("Entered Nemona's house");
        mash_button_till_overworld(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Picking a starter...");
        switch(starter_choice){
        case StarterChoice::SPRIGATITO:
            env.console.log("Picking Sprigatito...");
            pbf_move_left_joystick(context, 75, 0, 80, 20);
            break;
        case StarterChoice::FUECOCO:
            env.console.log("Picking Fuecoco...");
            pbf_move_left_joystick(context, 180, 0, 80, 20);
            break;
        case StarterChoice::QUAXLY:
            env.console.log("Picking Quaxly...");
            pbf_move_left_joystick(context, 128, 0, 80, 20);
            break;
        }
        pbf_press_button(context, BUTTON_A, 20, 105); // choose the starter
        env.console.log("clear_dialog: Choose starter. Stop when detect prompt to receive starter.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20, {ClearDialogCallback::PROMPT_DIALOG});

        pbf_press_button(context, BUTTON_A, 20, 105); // accept the pokemon
        env.console.log("clear_dialog: Stop when detect prompt to give nickname to starter.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20, {ClearDialogCallback::PROMPT_DIALOG});

        pbf_mash_button(context, BUTTON_B, 100);  // Don't give a nickname
        env.console.log("clear_dialog: Talk to Nemona and Clavell. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 20, {ClearDialogCallback::OVERWORLD});
        
        context.wait_for_all_requests();
        env.console.log("Clear auto heal tutorial.");
        // Press X until Auto heal tutorial shows up
        TutorialWatcher tutorial;
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_button(context, BUTTON_X, 20, 250);
                }
            },
            {tutorial}
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Stuck trying to clear auto heal tutorial.",
                true
            );  
        }
        clear_tutorial(env.console, context);

        env.console.log("Change move order.");
        swap_starter_moves(env.program_info(), env.console, context, language);
        leave_box_system_to_overworld(env.program_info(), env.console, context);

        break;
    }catch(...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }
    }

}

void checkpoint_04(
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

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 220, 245, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 128, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 1 * TICKS_PER_SECOND);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 60, 50);
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
        env.console.log("overworld_navigation: Go to Nemona at the beach.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 8);
        
        context.wait_for_all_requests();
        env.console.overlay().add_log("Found Nemona", COLOR_WHITE);

        context.wait_for_all_requests();
        env.console.log("Starting battle...");
        env.console.overlay().add_log("Starting battle...", COLOR_WHITE);
        // TODO: Battle start prompt detection
        // can lose this battle, and story will continue
        mash_button_till_overworld(env.console, context);
        context.wait_for_all_requests();
        env.console.log("Finished battle.");
        env.console.overlay().add_log("Finished battle.", COLOR_WHITE);        

        break;
    }catch(...){
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
