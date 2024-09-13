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



void checkpoint_05(
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

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 160, 60);
        pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 84, 60);
        env.console.log("overworld_navigation: Go to mom at the gate.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20);
        
        context.wait_for_all_requests();
        env.console.log("Get mom's sandwich");
        env.console.overlay().add_log("Get mom's sandwich", COLOR_WHITE);
        mash_button_till_overworld(env.console, context);
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

void checkpoint_06(
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

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 82, 60);
        pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 110, 10, 60);
        env.console.log("overworld_navigation: Go to Nemona.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20);
        
        context.wait_for_all_requests();
        env.console.log("clear_dialog: Talk with Nemona to start catch tutorial. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, 
            {ClearDialogCallback::WHITE_A_BUTTON, ClearDialogCallback::TUTORIAL, ClearDialogCallback::BATTLE});
        
        // can die in catch tutorial, and the story will continue
        env.console.log("run_battle_press_A: Battle Lechonk in catch tutorial. Stop when detect dialog.");
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        env.console.log("clear_dialog: Talk with Nemona to finish catch tutorial. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {ClearDialogCallback::TUTORIAL, ClearDialogCallback::OVERWORLD});

        context.wait_for_all_requests();
        env.console.log("Finished catch tutorial");
        env.console.overlay().add_log("Finished catch tutorial", COLOR_WHITE);

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

void checkpoint_07(
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
        env.console.log("Move to cliff");
        env.console.overlay().add_log("Move to cliff", COLOR_WHITE);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 240, 60, 80);
        env.console.log("overworld_navigation: Go to cliff.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 116, 0, 72, 24);

        env.console.log("clear_dialog: Talk to Nemona at the cliff. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {ClearDialogCallback::OVERWORLD});

        context.wait_for_all_requests();
        env.console.log("Mystery cry");
        env.console.overlay().add_log("Mystery cry", COLOR_WHITE);
        
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

void checkpoint_08(
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

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 70, 100);
        env.console.log("overworld_navigation: Go to cliff.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0);

        env.console.log("clear_dialog: Look over the injured Miraidon/Koraidon on the beach. Fall down the cliff");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 30, {});
        // long animation
        env.console.log("overworld_navigation: Go to Legendary pokemon laying on the beach.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 30);

        env.console.log("clear_dialog: Offer Miraidon/Koraidon a sandwich.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});

        // TODO: Bag menu navigation
        context.wait_for_all_requests();
        env.console.log("Feed mom's sandwich");
        env.console.overlay().add_log("Feed mom's sandwich", COLOR_WHITE);
        
        GradientArrowWatcher arrow(COLOR_RED, GradientArrowType::RIGHT, {0.104, 0.312, 0.043, 0.08});
        context.wait_for_all_requests();

        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_dpad(context, DPAD_UP, 20, 250);
                }
            },
            {arrow}
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to feed mom's sandwich.",
                true
            );  
        }

        // only press A when the sandwich is selected
        pbf_mash_button(context, BUTTON_A, 100);

        env.console.log("clear_dialog: Miraidon/Koraidon gets up and walks to cave entrance.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 35, {});
        // long animation

        // First Nemona cave conversation
        context.wait_for_all_requests();
        env.console.log("Enter cave");
        env.console.overlay().add_log("Enter cave", COLOR_WHITE);
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 20, 10 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 150, 20, 1 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 150, 20, 2 * TICKS_PER_SECOND, 20);                
            }
        );
        
        env.console.log("overworld_navigation: Go to cave.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 10);

        env.console.log("clear_dialog: Talk to Nemona yelling down, while you're down in the cave.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {ClearDialogCallback::PROMPT_DIALOG});

        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                // Legendary rock break
                context.wait_for_all_requests();
                console.log("Rock break");
                console.overlay().add_log("Rock break", COLOR_WHITE);
                pbf_move_left_joystick(context, 128, 20, 3 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 230, 25, 30);
                pbf_move_left_joystick(context, 128, 0, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

                // Houndour wave
                context.wait_for_all_requests();
                console.log("Houndour wave");
                console.overlay().add_log("Houndour wave", COLOR_WHITE);
                pbf_move_left_joystick(context, 140, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 15, 30);
                pbf_move_left_joystick(context, 128, 20, 5 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 25, 20);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 220, 25, 25);
                pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 20 * TICKS_PER_SECOND);

                // Houndoom encounter
                context.wait_for_all_requests();
                console.log("Houndoom encounter");
                console.overlay().add_log("Houndoom encounter", COLOR_WHITE);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 245, 20, 20);
                pbf_move_left_joystick(context, 128, 20, 2 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NO_MARKER, 255, 90, 20);
                pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_L, 20, 20);
            }
        );
        
        env.console.log("overworld_navigation: Go to Houndoom.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 20, 40);
        
        mash_button_till_overworld(env.console, context, BUTTON_A);

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


void checkpoint_09(
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

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 120, 100);
        env.console.log("overworld_navigation: Go to Arven at the tower.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0);
            
        context.wait_for_all_requests();
        env.console.log("Found Arven");
        env.console.overlay().add_log("Found Arven", COLOR_WHITE);
        // can lose battle, and story will still continue
        mash_button_till_overworld(env.console, context, BUTTON_A);
        context.wait_for_all_requests();
        env.console.log("Receive legendary ball");
        env.console.overlay().add_log("Receive legendary ball", COLOR_WHITE);

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

void checkpoint_10(
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
        env.console.log("Lighthouse view");
        env.console.overlay().add_log("Lighthouse view", COLOR_WHITE);
        do_action_and_monitor_for_battles(env, env.console, context,
            [&](SingleSwitchProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
                realign_player(env.program_info(), console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 110, 100);
                pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 20);
                pbf_move_left_joystick(context, 255, 128, 15, 20);
                pbf_press_button(context, BUTTON_L, 20, 20);
                pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 20);                
            }
        );

        env.console.log("overworld_navigation: Go to Nemona on the lighthouse.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 128, 0, 20);

        mash_button_till_overworld(env.console, context, BUTTON_A);

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
