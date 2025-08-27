/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_09.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_09::name() const{
    return "09: Complete tutorial";
}

std::string AutoStory_Segment_09::start_text() const{
    return "Start: Battled Team Star. Talked to Jacq, introduced self to class, standing in middle of classroom.";
}

std::string AutoStory_Segment_09::end_text() const{
    return "End: Finished tutorial. Acquired all 3 questlines. Got on ride for first time.";
}

void AutoStory_Segment_09::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment 09: Complete tutorial", COLOR_ORANGE);

    checkpoint_16(env, context, options.notif_status_update, stats);
    checkpoint_17(env, context, options.notif_status_update, stats);
    checkpoint_18(env, context, options.notif_status_update, stats);
    checkpoint_19(env, context, options.notif_status_update, stats);
    checkpoint_20(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment 09: Complete tutorial", COLOR_GREEN);

}




void checkpoint_16(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

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
        pbf_move_left_joystick(context, 128, 0, 600, 100);
        // turn left 
        pbf_move_left_joystick(context, 0, 128, 20, 100);

        // talk to Arven. stop at overworld. need prompt, overworld, white button A. and book?
        env.console.log("Talk with Arven. Receive Titan questline (Path of Legends).");
        press_A_until_dialog(env.program_info(), env.console, context, 1);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
        
       
    });

}

void checkpoint_17(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();

        // walk backwards until dialog
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20, 128, 255);
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
        navigate_school_layout_menu(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078},
            {0.031, 0.193 + 0.074219, 0.047, 0.078}, DPAD_DOWN, 1);
        // enter staff room
        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
        pbf_wait(context, 3 * TICKS_PER_SECOND);

        env.console.log("clear_dialog: See Geeta. Talk to Nemona. Receive Gym/Elite Four questline (Victory Road).");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        
       
    });

}

void checkpoint_18(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

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
       
    });

}

void checkpoint_19(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

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

       
    });

}


void checkpoint_20(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();
       
        //walk right towards door
        pbf_move_left_joystick(context, 255, 128, 200, 100);

        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.031, 0.193, 0.047, 0.078}, 10);

        env.console.log("Leave dorm for schoolyard.");
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 128, 0);

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

    });

}





}
}
}
