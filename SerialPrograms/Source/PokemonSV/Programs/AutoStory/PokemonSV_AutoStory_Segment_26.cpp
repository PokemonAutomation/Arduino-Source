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
#include "PokemonSV_AutoStory_Segment_26.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_26::name() const{
    return "26: Medali Gym (Normal)";
}

std::string AutoStory_Segment_26::start_text() const{
    return "Start: Beat Team Star (Poison). At Medali Pokecenter.";
}

std::string AutoStory_Segment_26::end_text() const{
    return "End: Beat Medali Gym (Normal).";
}

void AutoStory_Segment_26::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_61(env, context, options.notif_status_update, stats);
    checkpoint_62(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

void checkpoint_61(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats, 
    [&](size_t attempt_number){

        // marker 1   keep{0.490625, 0.594444}  in{0.589583, 0.569444} 
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.589583, 0.569444}
        );

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 0.278620);
        pbf_move_left_joystick(context, 128, 0, 400, 50);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 60, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        direction.change_direction(env.program_info(), env.console, context, 3.566068);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, 255, 0, 100, 50); // if to the left of the door, will move right and enter
                pbf_move_left_joystick(context, 0, 128, 300, 50); // if to the right of the door, will move left
                pbf_move_left_joystick(context, 255, 0, 100, 50); // then move right and enter
            }
        );

        // speak to Nemona, inside the gym
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // speak to gym receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

    });    

}

void checkpoint_62(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        // fly to Medali East Pokecenter
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 255, 128, 20}, FlyPoint::POKECENTER);

        pbf_press_button(context, BUTTON_L, 50, 50);

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 1.971173);
        pbf_move_left_joystick(context, 128, 0, 600, 50);

        direction.change_direction(env.program_info(), env.console, context, 3.191172);
        pbf_move_left_joystick(context, 128, 0, 300, 50);

         
        direction.change_direction(env.program_info(), env.console, context, 5.114177);  // old 4.975295
        pbf_move_left_joystick(context, 128, 0, 300, 50);

        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after entering Eatery
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_left_joystick(context, 255, 128, 50, 50);
        
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});

        // grilled rice balls
        pbf_press_button(context, BUTTON_A, 50, 50);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        // medium serving
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);
        pbf_press_button(context, BUTTON_A, 50, 50);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        // extra crispy
        pbf_press_dpad(context, DPAD_UP, 13, 20);
        pbf_press_button(context, BUTTON_A, 50, 50);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        // lemon
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);
        pbf_press_button(context, BUTTON_A, 50, 50);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});


        pbf_mash_button(context, BUTTON_A, 1000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});

        env.console.log("Battle Normal Gym leader.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 60);
        pbf_mash_button(context, BUTTON_A, 1000ms);

        env.console.log("Finish up with Larry, then speak to Geeta and Nemona.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::WHITE_A_BUTTON, CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});
        env.console.log("Battle Nemona.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);
        

    });    

}

}
}
}
