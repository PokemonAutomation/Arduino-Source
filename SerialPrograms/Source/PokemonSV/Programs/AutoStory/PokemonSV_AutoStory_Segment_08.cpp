/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_08.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_08::name() const{
    return "08: Beat Team Star and arrive at School";
}

std::string AutoStory_Segment_08::start_text() const{
    return "Start: At Mesagoza South Pokecenter.";
}

std::string AutoStory_Segment_08::end_text() const{
    return "End: Battled Team Star. Talked to Jacq, introduced self to class, standing in middle of classroom.";
}

void AutoStory_Segment_08::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment 08: Beat Team Star and arrive at School", COLOR_ORANGE);

    checkpoint_13(env, context, options.notif_status_update, stats);
    checkpoint_14(env, context, options.notif_status_update, stats);
    checkpoint_15(env, context, options.notif_status_update, stats);
    

    context.wait_for_all_requests();
    env.console.log("End Segment 08: Beat Team Star and arrive at School", COLOR_GREEN);

}


void checkpoint_13(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    // reset rate: 0%. 0 resets out of 70.
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){                  

            fly_to_overlapping_flypoint(info, env.console, context);

            context.wait_for_all_requests();
            realign_player(info, env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 80, 50);
            walk_forward_while_clear_front_path(info, env.console, context, 500);
            walk_forward_until_dialog(info, env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);
        });

        env.console.log("clear_dialog: Talk with Nemona at Mesagoza gate. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60,
            {CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW, CallbackEnum::BATTLE});
        
        env.console.log("Battle with Nemona at Mesagoza gate. Stop when detect dialog.");
        // story continues even if you lose, no need to detect wipeout
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        
        env.console.log("clear_dialog: Talk with Nemona within Mesagoza. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG, CallbackEnum::WHITE_A_BUTTON});
        
       
    });

}

void checkpoint_14(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

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
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        // run battle until dialog
        env.console.log("Battle with Team Star grunt 1. Stop when detect dialog.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {}, true);  // need to detect wipeouts, since you need to win, and you likely only have 1 pokemon
        // clear dialog until battle, with prompt, white button, tutorial, battle
        env.console.log("clear_dialog: Talk with Team Star and Nemona. Receive Tera orb. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, 
            {CallbackEnum::PROMPT_DIALOG, CallbackEnum::WHITE_A_BUTTON, CallbackEnum::TUTORIAL, CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        // run battle until dialog
        env.console.log("Battle with Team Star grunt 2. Stop when detect dialog.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {}, true); // need to detect wipeouts, since you need to win, and you likely only have 1 pokemon
        // clear dialog until overworld
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});
       
    });

}

void checkpoint_15(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

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
            {CallbackEnum::PROMPT_DIALOG, CallbackEnum::OVERWORLD});
       
    });

}



}
}
}
