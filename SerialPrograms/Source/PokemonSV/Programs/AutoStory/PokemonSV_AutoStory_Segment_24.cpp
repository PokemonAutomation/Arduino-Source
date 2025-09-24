/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"

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
    return "24: Orthworm Titan: Battle Orthworm Titan";
}

std::string AutoStory_Segment_24::start_text() const{
    return "Start: At East Province (Area Three) Watchtower.";
}

std::string AutoStory_Segment_24::end_text() const{
    return "End: Beat Orthworm Titan. At East Province (Area Three) Watchtower.";
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

    checkpoint_55(env, context, options.notif_status_update, stats);
    checkpoint_56(env, context, options.notif_status_update, stats);
    checkpoint_57(env, context, options.notif_status_update, stats);

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

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            
            DirectionDetector direction;

            direction.change_direction(env.program_info(), env.console, context, 3.909067);
            pbf_move_left_joystick(context, 128, 0, 1000, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.061720);        
            pbf_move_left_joystick(context, 128, 0, 500, 100);
            pbf_move_left_joystick(context, 255, 0, 200, 100);

            // now aligned to the wall next to the hole/passage

            // walk away from wall slightly
            pbf_move_left_joystick(context, 128, 255, 50, 100);
            get_on_ride(env.program_info(), env.console, context);

            direction.change_direction(env.program_info(), env.console, context, 0.366);
            pbf_move_left_joystick(context, 128, 0, 250, 100);

            direction.change_direction(env.program_info(), env.console, context, 2.565);
            // run at Orthworm. run into its second position as well.
            pbf_move_left_joystick(context, 128, 0, 50, 0);
            pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, 500);
            pbf_move_left_joystick(context, 255, 0, 300, 500);

            get_off_ride(env.program_info(), env.console, context);

            direction.change_direction(env.program_info(), env.console, context, 0.261);
            pbf_move_left_joystick(context, 128, 0, 500, 100);
            pbf_move_left_joystick(context, 0, 0, 500, 100);

            // now aligned to corner.

            direction.change_direction(env.program_info(), env.console, context, 3.736);
            pbf_move_left_joystick(context, 128, 0, 400, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.306);
            pbf_move_left_joystick(context, 128, 0, 700, 100);

            
            direction.change_direction(env.program_info(), env.console, context, 4.988);
            pbf_move_left_joystick(context, 128, 0, 800, 100);
            pbf_move_left_joystick(context, 255, 0, 500, 100);

            // now aligned to the wall next to the hole/passage   

            // walk away from wall slightly
            pbf_move_left_joystick(context, 128, 255, 100, 100);  

            direction.change_direction(env.program_info(), env.console, context, 5.722795);
            pbf_move_left_joystick(context, 128, 0, 600, 100);

            direction.change_direction(env.program_info(), env.console, context, 0.625226);
        });

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);

        env.console.log("Battle Orthworm Titan phase 1.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);

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

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            
            DirectionDetector direction;

            direction.change_direction(env.program_info(), env.console, context, 5.042435);
            pbf_move_left_joystick(context, 128, 0, 900, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.360763);
            pbf_move_left_joystick(context, 128, 0, 500, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.85);
            pbf_move_left_joystick(context, 128, 0, 700, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.428);
            pbf_move_left_joystick(context, 128, 0, 600, 100);

            direction.change_direction(env.program_info(), env.console, context, 4.908646);
            pbf_move_left_joystick(context, 128, 0, 300, 100);

            direction.change_direction(env.program_info(), env.console, context, 1.169728);
            pbf_move_left_joystick(context, 128, 0, 200, 100);
            pbf_move_left_joystick(context, 255, 0, 200, 100);
            pbf_move_left_joystick(context, 0, 0, 200, 100);

            // now aligned to the wall next to the hole/passage   

            // walk backwards
            direction.change_direction(env.program_info(), env.console, context, 2.303077);
            pbf_move_left_joystick(context, 128, 255, 400, 100);
            direction.change_direction(env.program_info(), env.console, context, 3.908360);

            get_on_ride(env.program_info(), env.console, context);

            // charge at orthworm
            pbf_move_left_joystick(context, 128, 0, 408ms, 0ms);
            pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, 102ms);
            pbf_move_left_joystick(context, 128, 0, 1970ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, 128, 0, 0, 128, 432ms);
            pbf_move_left_joystick(context, 128, 0, 1993ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, 128, 0, 0, 128, 301ms);
            pbf_move_left_joystick(context, 128, 0, 307ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, 128, 0, 0, 128, 194ms);
            pbf_move_left_joystick(context, 128, 0, 886ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, 128, 0, 0, 128, 626ms);
            pbf_move_left_joystick(context, 128, 0, 2651ms, 0ms);
        });

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);

        // battle the titan phase 2
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});  
        env.console.log("Battle Orthworm Titan phase 2.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

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
        // fly back to East Province (Area Three) Watchtower
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0}, FlyPoint::FAST_TRAVEL);

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
