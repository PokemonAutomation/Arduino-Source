/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_25.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_25::name() const{
    return "25: Team Star (Poison)";
}

std::string AutoStory_Segment_25::start_text() const{
    return "Start: Beat Orthworm Titan. At East Province (Area Three) Pokecenter.";
}

std::string AutoStory_Segment_25::end_text() const{
    return "End: Beat Team Star (Poison). At Medali Pokecenter.";
}

void AutoStory_Segment_25::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_58(env, context, options.notif_status_update, stats);
    checkpoint_59(env, context, options.notif_status_update, stats);
    checkpoint_60(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

void checkpoint_58(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();
        
        // marker 1  {0.795312, 0.626852}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::FAST_TRAVEL, 
            {0.795312, 0.626852}
        );

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 2   {0.672396, 0.532407}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.672396, 0.532407}
        );

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // speak to Clive
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // marker 3   {0.685417, 0.571296}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.685417, 0.571296}
        );

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // battle Team Star Grunt
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW, CallbackEnum::BATTLE});
        env.console.log("Battle team star grunt.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

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

        context.wait_for_all_requests();
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 255, 50);
                walk_forward_while_clear_front_path(env.program_info(), env.console, context, 100);
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
            }
        );
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        AdvanceDialogWatcher    dialog(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){

                DirectionDetector direction;
                uint16_t seconds_wait = 8; // todo: change back to 6-8
 
                direction.change_direction(env.program_info(), env.console, context, 2.618972);
                pbf_move_left_joystick(context, 128, 0, 100, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 0, 200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);

                direction.change_direction(env.program_info(), env.console, context, 3.265261);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 2.411131);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                pbf_move_left_joystick(context, 128, 0, 300, 50);

                direction.change_direction(env.program_info(), env.console, context, 2.828856);
                pbf_move_left_joystick(context, 128, 0, 200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 2.306254);
                pbf_move_left_joystick(context, 128, 0, 500, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 2.565727);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 3.036098);
                pbf_move_left_joystick(context, 128, 0, 400, 50);

                direction.change_direction(env.program_info(), env.console, context, 3.525225);
                pbf_move_left_joystick(context, 128, 0, 200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 4.051901);
                pbf_move_left_joystick(context, 128, 0, 700, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 4.573712);
                pbf_move_left_joystick(context, 128, 0, 500, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


                // wait 30 seconds to let the trainers cooldown
                pbf_wait(context, 30 * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 1.310148);
                pbf_move_left_joystick(context, 128, 0, 800, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);
                

                direction.change_direction(env.program_info(), env.console, context, 0.572754);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_move_left_joystick(context, 128, 0, 700, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 6.036137);
                pbf_move_left_joystick(context, 128, 0, 200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 5.779599);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                pbf_move_left_joystick(context, 128, 0, 600, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


                pbf_wait(context, 10 * TICKS_PER_SECOND);
                
            },
            {dialog}
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Team Star Poison: Failed to kill 30 pokemon with Let's go.",
                env.console
            );            
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle the Team Star (Poison) boss.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

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
        // fly back to Porto Marinada Pokecenter
        // move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0}, FlyPoint::FAST_TRAVEL);

    });    

}





}
}
}
