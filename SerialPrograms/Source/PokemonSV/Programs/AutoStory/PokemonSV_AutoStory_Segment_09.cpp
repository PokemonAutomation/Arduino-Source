/*  AutoStory
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
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_09.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




std::string AutoStory_Segment_09::name() const{
    return "09: Complete tutorial";
}

std::string AutoStory_Segment_09::start_text() const{
    return "Start: Battled Team Star, talked to Jacq, standing in classroom.";
}

std::string AutoStory_Segment_09::end_text() const{
    return "End: Finished tutorial. Acquired all 3 questlines. Got on ride for first time.";
}

void AutoStory_Segment_09::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.overlay().add_log("Start Segment 09: Complete tutorial", COLOR_ORANGE);

    checkpoint_16(env, context, options.notif_status_update);
    checkpoint_17(env, context, options.notif_status_update);
    checkpoint_18(env, context, options.notif_status_update);
    checkpoint_19(env, context, options.notif_status_update);
    checkpoint_20(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment 09: Complete tutorial", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats();

}




void checkpoint_16(
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

        // walk left
        pbf_move_left_joystick(context, 0, 128, 400, 100);
        // walk down to classroom exit.
        pbf_move_left_joystick(context, 128, 255, 300, 100);
        env.console.log("clear_dialog: Leave classroom.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);

        // Wait for detection of school navigation menu
        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 5);

        // enter Cafeteria
        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
        pbf_wait(context, 3 * TICKS_PER_SECOND);
        context.wait_for_all_requests();

        // walk forward
        pbf_move_left_joystick(context, 128, 0, 300, 100);
        // turn left 
        pbf_move_left_joystick(context, 0, 128, 50, 100);

        // talk to Arven. stop at overworld. need prompt, overworld, white button A. and book?
        env.console.log("Talk with Arven. Receive Titan questline (Path of Legends).");
        press_A_until_dialog(env.program_info(), env.console, context, 1);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
        
       
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

void checkpoint_17(
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

        // walk backwards until dialog
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20, 255);
        env.console.log("Talk with Cassiopeia.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // re-orient camera
        pbf_press_button(context, BUTTON_L, 20, 100);
        // move backwards towards front desk
        pbf_move_left_joystick(context, 128, 255, 200, 100);
        // re-orient camera
        pbf_press_button(context, BUTTON_L, 20, 100);
        // move right towards navigation kiosk
        pbf_move_left_joystick(context, 255, 128, 100, 100);
        // open school navigation screen
        press_button_until_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078});
        // go to staff room
        basic_menu_navigation(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, {0.031, 0.193 + 0.074219, 0.047, 0.078}, DPAD_DOWN, 1);
        // enter staff room
        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
        pbf_wait(context, 3 * TICKS_PER_SECOND);

        env.console.log("clear_dialog: See Geeta. Talk to Nemona. Receive Gym/Elite Four questline (Victory Road).");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        
       
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

void checkpoint_18(
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

        // walk down
        pbf_move_left_joystick(context, 128, 255, 200, 100);
        // walk left towards door
        pbf_move_left_joystick(context, 0, 128, 100, 100);

        // wait for school navigation menu
        context.wait_for_all_requests();
        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);
        // enter Directors office
        pbf_mash_button(context, BUTTON_A, 6 * TICKS_PER_SECOND);

        env.console.log("Talk to Clavell in his office, and the professor.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 25, 
            {CallbackEnum::PROMPT_DIALOG});  // max time between dialog: 17s. set timeout to 25 seconds for buffer.
        // mash A to get through the Random A press that you need. when the professor shows you area zero.
        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});
       
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

void checkpoint_19(
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

        // walk right 
        pbf_move_left_joystick(context, 255, 128, 50, 100);
        // walk down towards door
        pbf_move_left_joystick(context, 128, 255, 200, 100);

        env.console.log("Talk to Nemona and go to dorm.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // walk forward
        pbf_move_left_joystick(context, 128, 0, 100, 100);
        // walk left towards bed
        pbf_move_left_joystick(context, 0, 128, 100, 100);

        env.console.log("Go to bed. Time passes until treasure hunt.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

       
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


void checkpoint_20(
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
       
        //walk right towards door
        pbf_move_left_joystick(context, 255, 128, 200, 100);

        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);

        env.console.log("Leave dorm for schoolyard.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 0);

        env.console.log("Talk to Nemona, Arven, Cassiopeia.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 16, 
            {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BLACK_DIALOG_BOX}); // max time between dialog: 11
        
        // mash A to get through the Random A press that you need. when the Nemona shows you a Poke Gym.
        pbf_mash_button(context, BUTTON_A, 250);

        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, 
            {CallbackEnum::TUTORIAL}); // max time between dialog: 3
        
        env.console.log("Get on ride.");
        pbf_mash_button(context, BUTTON_PLUS, 1 * TICKS_PER_SECOND);

        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

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
