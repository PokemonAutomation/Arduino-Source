/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_34.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_34::name() const{
    return "34: Battle Nemona, Penny, Arven";
}

std::string AutoStory_Segment_34::start_text() const{
    return "Start: Beat Clavell. Beat Elite Four. At Pokemon League Pokecenter.";
}

std::string AutoStory_Segment_34::end_text() const{
    return "End: Beat Nemona, Penny, and Arven. At Los Platos Pokecenter.";
}

void AutoStory_Segment_34::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_90().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_91().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_92().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_90::name() const{ return "090 - " + AutoStory_Segment_34().name(); }
std::string AutoStory_Checkpoint_90::start_text() const{ return "Beat Geeta. At Pokemon League Pokecenter.";}
std::string AutoStory_Checkpoint_90::end_text() const{ return "Beat Nemona. At dormitory room, next to bed.";}
void AutoStory_Checkpoint_90::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_90(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_91::name() const{ return "091 - " + AutoStory_Segment_34().name(); }
std::string AutoStory_Checkpoint_91::start_text() const{ return AutoStory_Checkpoint_90().end_text();}
std::string AutoStory_Checkpoint_91::end_text() const{ return "Beat Penny. At Academy fly point.";}
void AutoStory_Checkpoint_91::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_91(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_92::name() const{ return "092 - " + AutoStory_Segment_34().name(); }
std::string AutoStory_Checkpoint_92::start_text() const{ return AutoStory_Checkpoint_91().end_text();}
std::string AutoStory_Checkpoint_92::end_text() const{ return "Beat Arven. At Los Platos Pokecenter.";}
void AutoStory_Checkpoint_92::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_92(env, context, options.notif_status_update, stats);
}



void checkpoint_90(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // Fly to Academy
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 230, 80}, FlyPoint::FAST_TRAVEL);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 255, 100);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 30, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 120, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});

        env.console.log("Battle Nemona.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);


    });   
}

void checkpoint_91(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        
        // walk down
        pbf_move_left_joystick(context, 128, 255, 100, 100);
        // walk right towards door
        pbf_move_left_joystick(context, 255, 128, 200, 100);

        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);

        env.console.log("Leave dorm for schoolyard.");
        pbf_press_dpad(context, DPAD_UP, 13, 20);
        pbf_press_dpad(context, DPAD_LEFT, 13, 20);
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);
        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.790, 0.047, 0.078}, 10);
        pbf_mash_button(context, BUTTON_A, 1000ms);

        // clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 60, {CallbackEnum::PROMPT_DIALOG});
        // pbf_mash_button(context, BUTTON_A, 1000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW, CallbackEnum::PROMPT_DIALOG});
        env.console.log("Battle Penny.");
        SinglesMoveEntry move1{SinglesMoveType::Move1, true};  // Moonblast
        std::vector<SinglesMoveEntry> move_table1 = {move1};
        bool terastallized = false;
        bool is_won = run_pokemon(env.console, context, move_table1, true, terastallized);
        if (!is_won){// throw exception if we lose
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to beat the Ground trainer. Reset.",
                env.console
            );
        }
        
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // now back in dorm room.

        // walk right towards door
        pbf_move_left_joystick(context, 255, 128, 200, 100);

        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);

        env.console.log("Leave dorm for Director's office.");
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);
        pbf_press_dpad(context, DPAD_RIGHT, 13, 20);
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);
        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.345, 0.047, 0.078}, 10);
        pbf_mash_button(context, BUTTON_A, 1000ms);

        wait_for_overworld(env.program_info(), env.console, context);

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);

        mash_button_till_overworld(env.console, context, BUTTON_A);

         // walk right towards door
        pbf_move_left_joystick(context, 255, 128, 200, 100);

        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // now in school lobby

        context.wait_for_all_requests();
        context.wait_for(1000ms);

        pbf_move_left_joystick(context, 128, 255, 1000, 100);
        // wait for dialog when leaving school lobby
        pbf_wait(context, 3 * TICKS_PER_SECOND);    
        
        mash_button_till_overworld(env.console, context, BUTTON_A);
        
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 
                    128, 0, 60, 60, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        mash_button_till_overworld(env.console, context, BUTTON_A);

        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 0, 0, 0}, FlyPoint::FAST_TRAVEL);


    });   
}

void checkpoint_92(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 145, 255, 350}, FlyPoint::FAST_TRAVEL);
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 128, 1000ms, 100ms);
        });
        mash_button_till_overworld(env.console, context, BUTTON_A);
        // clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        // leave the inside of the lighthouse and go outside
        pbf_move_left_joystick(context, 128, 255, 100, 50);
        pbf_move_left_joystick(context, 0, 128, 300, 50);
        pbf_move_left_joystick(context, 255, 255, 100, 50);

        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after building
        wait_for_overworld(env.program_info(), env.console, context, 30);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 190, 50);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_SPAM_A, 
            128, 0, 30, 30, false);
        

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW, CallbackEnum::PROMPT_DIALOG});
        env.console.log("Battle Arven.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        mash_button_till_overworld(env.console, context, BUTTON_A);

        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0}, FlyPoint::POKECENTER);
    });   
}

void checkpoint_93(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_94(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_95(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}




}
}
}
