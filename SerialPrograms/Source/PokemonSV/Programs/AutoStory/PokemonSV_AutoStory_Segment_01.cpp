/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_01.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





std::string AutoStory_Segment_01::name() const{
    return "01: Pick Starter";
}


std::string AutoStory_Segment_01::start_text() const{
    return "Start: Finished cutscene. Stood up from chair. Walked to left side of room.";
}

std::string AutoStory_Segment_01::end_text() const{
    return "End: Picked the starter. Changed move order.";
}

void AutoStory_Segment_01::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_01().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_02().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_03().run_checkpoint(env, context, options, stats);
    
    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_01::name() const{ return "001 - " + AutoStory_Segment_01().name(); }
std::string AutoStory_Checkpoint_01::start_text() const{ return "Done cutscene. Stood up from chair. Walked to left side of room.";}
std::string AutoStory_Checkpoint_01::end_text() const{ return "Standing in room. Updated settings";}
void AutoStory_Checkpoint_01::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_01(env, context, options.notif_status_update, stats, options.language);
}

std::string AutoStory_Checkpoint_02::name() const{ return "002 - " + AutoStory_Segment_01().name(); }
std::string AutoStory_Checkpoint_02::start_text() const{ return AutoStory_Checkpoint_01().end_text();}
std::string AutoStory_Checkpoint_02::end_text() const{ return "Standing in front of the 'power of science' NPC. Cleared map tutorial.";}
void AutoStory_Checkpoint_02::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_02(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_03::name() const{ return "003 - " + AutoStory_Segment_01().name(); }
std::string AutoStory_Checkpoint_03::start_text() const{ return  AutoStory_Checkpoint_02().end_text();}
std::string AutoStory_Checkpoint_03::end_text() const{ return "Received starter Pokemon. Changed move order. Cleared autoheal tutorial.";}
void AutoStory_Checkpoint_03::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_03(env, context, options.notif_status_update, stats, options.language, options.starter_choice);
}


void checkpoint_01(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats, 
    Language language
){
    checkpoint_reattempt_loop_tutorial(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        
        context.wait_for_all_requests();
        // set settings
        enter_menu_from_overworld(env.program_info(), env.console, context, 0, MenuSide::RIGHT, false);
        change_settings(env, context, language, attempt_number==0);
        pbf_mash_button(context, BUTTON_B, 2000ms);
        context.wait_for_all_requests();

    });
}

void checkpoint_02(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop_tutorial(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        
        context.wait_for_all_requests();
        env.console.log("Go downstairs, get stopped by Skwovet");
        env.console.overlay().add_log("Go downstairs, get stopped by Skwovet", COLOR_WHITE);
        pbf_move_left_joystick(context, 128,   0, 3000ms, 160ms);
        pbf_move_left_joystick(context,   0, 128, 3000ms, 160ms);
        pbf_move_left_joystick(context, {0, -1}, 3000ms, 160ms);
        pbf_wait(context, 5000ms);
        // clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go to the kitchen, talk with mom");
        env.console.overlay().add_log("Go to the kitchen, talk with mom", COLOR_WHITE);
        pbf_move_left_joystick(context, {0, -1}, 2000ms, 160ms);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 0, 128);

        env.console.log("clear_dialog: Talk with Mom.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go to the front door, talk with Clavell");
        env.console.overlay().add_log("Go to the front door, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context, 230, 200, 2000ms, 160ms);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 255, 128);
        
        env.console.log("clear_dialog: Talk with Clavell at front door.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});

        context.wait_for_all_requests();
        env.console.log("Go upstairs, dress up");
        env.console.overlay().add_log("Go upstairs, dress up", COLOR_WHITE);
        pbf_move_left_joystick(context, {+1, 0}, 2000ms, 160ms);
        pbf_move_left_joystick(context, 185,  10, 1000ms, 160ms);
        pbf_move_left_joystick(context, 128,   0, 4000ms, 160ms);
        pbf_move_left_joystick(context, {+1, 0}, 4000ms, 160ms);
        pbf_move_left_joystick(context, 110, 200, 3000ms, 160ms);
        pbf_move_left_joystick(context, {+1, 0}, 2000ms, 160ms);
        pbf_mash_button(context, BUTTON_A, 20000ms);

        context.wait_for_all_requests();
        env.console.log("Go to the living room, talk with Clavell");
        env.console.overlay().add_log("Go to the living room, talk with Clavell", COLOR_WHITE);
        pbf_move_left_joystick(context,   0,   0, 3000ms, 160ms);
        pbf_move_left_joystick(context,   0, 128, 3000ms, 160ms);
        pbf_move_left_joystick(context, {0, -1}, 4000ms, 160ms);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 0, 128);        
        
        env.console.log("clear_dialog: Talk with Clavell at living room.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});

        context.wait_for_all_requests();
        env.console.log("Go outside, receive Rotom Phone");
        env.console.overlay().add_log("Go outside, receive Rotom Phone", COLOR_WHITE);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 245, 230);

        env.console.log("clear_dialog: Talk with Clavell outside. Receive Rotom phone. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::WHITE_A_BUTTON});

        context.wait_for_all_requests();
        env.console.log("Clear map tutorial");
        open_map_from_overworld(env.program_info(), env.console, context, true);
        leave_phone_to_overworld(env.program_info(), env.console, context);

    });
}

void checkpoint_03(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    Language language,
    StarterChoice starter_choice
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        
        context.wait_for_all_requests();
        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 4.62);
        pbf_move_left_joystick(context, 128, 0, 3600, 50);
        pbf_move_left_joystick(context, {-1, 0}, 240ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 4.62);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);
        
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
        pbf_press_button(context, BUTTON_A, 160ms, 840ms); // choose the starter
        env.console.log("clear_dialog: Choose starter. Stop when detect prompt to receive starter.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20, {CallbackEnum::PROMPT_DIALOG});

        pbf_press_button(context, BUTTON_A, 160ms, 840ms); // accept the pokemon
        env.console.log("clear_dialog: Stop when detect prompt to give nickname to starter.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 20, {CallbackEnum::PROMPT_DIALOG});

        pbf_mash_button(context, BUTTON_B, 100);  // Don't give a nickname
        env.console.log("clear_dialog: Talk to Nemona and Clavell. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 20, {CallbackEnum::OVERWORLD});
        
        context.wait_for_all_requests();
        env.console.log("Clear auto heal tutorial.");
        // Press X until Auto heal tutorial shows up
        TutorialWatcher tutorial;
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_button(context, BUTTON_X, 20, 250);
                }
            },
            {tutorial}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Stuck trying to clear auto heal tutorial.",
                env.console
            );  
        }
        clear_tutorial(env.console, context);

        env.console.log("Change move order.");
        swap_starter_moves(env, context, language);
        press_Bs_to_back_to_overworld(env.program_info(), env.console, context);

    }, false);
     
}




}
}
}
