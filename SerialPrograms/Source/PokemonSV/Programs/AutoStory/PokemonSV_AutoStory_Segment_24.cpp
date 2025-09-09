/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_24.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_24::name() const{
    return "";
}

std::string AutoStory_Segment_24::start_text() const{
    return "Start: ";
}

std::string AutoStory_Segment_24::end_text() const{
    return "End: ";
}

void AutoStory_Segment_24::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    // checkpoint_(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_55(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        // todo: try to align to left side of tunnel. then charge at Orthworm
        // reset if caught in battle.
        /////////////////////

        // get_off_ride(env.program_info(), env.console, context);

        
        // direction.change_direction(env.program_info(), env.console, context, 0.261);
        // pbf_move_left_joystick(context, 128, 0, 500, 100);
        // pbf_move_left_joystick(context, 0, 0, 500, 100);

        // // now aligned to corner.

        // direction.change_direction(env.program_info(), env.console, context, 3.736);
        // pbf_move_left_joystick(context, 128, 0, 400, 100);

        // direction.change_direction(env.program_info(), env.console, context, 5.306);
        // pbf_move_left_joystick(context, 128, 0, 700, 100);

        
        // direction.change_direction(env.program_info(), env.console, context, 4.988);
        // pbf_move_left_joystick(context, 128, 0, 800, 100);
        // pbf_move_left_joystick(context, 255, 0, 500, 100);

        // // now aligned to the wall next to the hole/passage

        // // walk away from wall slightly
        // pbf_move_left_joystick(context, 128, 255, 50, 100);
        // get_on_ride(env.program_info(), env.console, context);

        // direction.change_direction(env.program_info(), env.console, context, 0.366);
        // pbf_move_left_joystick(context, 128, 0, 250, 100);

        // direction.change_direction(env.program_info(), env.console, context, 2.565);
        // // run at Orthworm. run into its second position as well.
        // pbf_move_left_joystick(context, 128, 0, 50, 0);
        // pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, 500);
        // pbf_move_left_joystick(context, 255, 0, 500, 500);

        // get_off_ride(env.program_info(), env.console, context);

        // direction.change_direction(env.program_info(), env.console, context, 0.261);
        // pbf_move_left_joystick(context, 128, 0, 500, 100);
        // pbf_move_left_joystick(context, 0, 0, 500, 100);

        // // now aligned to corner.

        // direction.change_direction(env.program_info(), env.console, context, 3.736);
        // pbf_move_left_joystick(context, 128, 0, 400, 100);

        // direction.change_direction(env.program_info(), env.console, context, 5.306);
        // pbf_move_left_joystick(context, 128, 0, 700, 100);

        
        // direction.change_direction(env.program_info(), env.console, context, 4.988);
        // pbf_move_left_joystick(context, 128, 0, 800, 100);
        // pbf_move_left_joystick(context, 255, 0, 500, 100);

        // // now aligned to the wall next to the hole/passage


    });    

}

void checkpoint_56(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });    

}

void checkpoint_57(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });    

}

void checkpoint_58(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });    

}

void checkpoint_59(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });    

}

void checkpoint_60(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


    });    

}





}
}
}
