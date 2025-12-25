/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/Farming/PokemonSV_ESPTraining.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_32.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_32::name() const{
    return "32: Alfornada Gym (Psychic)";
}

std::string AutoStory_Segment_32::start_text() const{
    return "Start: Beat Team Star (Fighting). At North Province Area Two Pokecenter.";
}

std::string AutoStory_Segment_32::end_text() const{
    return "End: Beat Alfornada Gym (Psychic). At Alfornada Pokecenter.";
}

void AutoStory_Segment_32::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_83().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_84().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}



std::string AutoStory_Checkpoint_83::name() const{ return "083 - " + AutoStory_Segment_32().name(); }
std::string AutoStory_Checkpoint_83::start_text() const{ return "At Alfornada Pokecenter.";}
std::string AutoStory_Checkpoint_83::end_text() const{ return "At Alfornada Pokecenter.";}
void AutoStory_Checkpoint_83::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_83(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_84::name() const{ return "084 - " + AutoStory_Segment_32().name(); }
std::string AutoStory_Checkpoint_84::start_text() const{ return AutoStory_Checkpoint_83().end_text();}
std::string AutoStory_Checkpoint_84::end_text() const{ return "Beat Alfornada gym challenge. Beat Alfornada gym. At Alfronada Pokecenter.";}
void AutoStory_Checkpoint_84::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_84(env, context, options.notif_status_update, stats);
}



void checkpoint_83(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // empty checkpoint


    }, false);  
}

void checkpoint_84(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        DirectionDetector direction;

        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 128, 0, 1200ms});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 128, 255, 1200ms});
        }
        
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 140, 70);

        direction.change_direction(env.program_info(), env.console, context, 3.104878);
        pbf_move_left_joystick_old(context, 128, 0, 4160ms, 400ms);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 50, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // {0.326042, 0.438889}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.326042, 0.438889}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );


        direction.change_direction(env.program_info(), env.console, context, 0.225386); //  0.225386

        handle_when_stationary_in_overworld(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30000ms);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick_old(context, 0, 0, 2400ms, 400ms); // move left
                pbf_move_left_joystick(context, {+1, 0}, 480ms, 400ms); // move right. center on door
                pbf_move_left_joystick_old(context, 128, 0, 2400ms, 400ms);  // move forward
            }
        );

        // speak to Nemona
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30000ms);
        
        // speak to receptionist
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});


        pbf_move_left_joystick(context, {0, -1}, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);


        direction.change_direction(env.program_info(), env.console, context, 4.413989);
        pbf_move_left_joystick(context, {0, +1}, 1440ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 5.516255);

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});

        //mash past other dialog
        pbf_mash_button(context, BUTTON_A, 2880ms);
            
        //wait for start
        context.wait_for(std::chrono::milliseconds(30000));
        context.wait_for_all_requests();


        // Run the ESP training mini game
        ESPTrainingStats esp_training_stats; // dummy stats
        run_esp_training(env, context, esp_training_stats);

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});

        env.console.log("Battle Gym Trainer 1.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);


        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5);
        pbf_wait(context, 10000ms);

        //mash past other dialog
        pbf_mash_button(context, BUTTON_A, 2880ms);
            
        //wait for start
        context.wait_for(std::chrono::milliseconds(8000));
        context.wait_for_all_requests();


        // Run the ESP training mini game
        run_esp_training(env, context, esp_training_stats);

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});

        env.console.log("Battle Gym Trainer 2.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        // Gym challenge now done

        direction.change_direction(env.program_info(), env.console, context, 2.336990);
        pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 0.156705);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30000ms);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick_old(context, 0, 0, 2400ms, 400ms); // move left
                pbf_move_left_joystick(context, {+1, 0}, 480ms, 400ms); // move right. center on door
                pbf_move_left_joystick_old(context, 128, 0, 2400ms, 400ms);  // move forward
            }
        );


        // speak to receptionist. if we fail to detect a battle, then we know we failed the Gym test. we then reset.
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});
        env.console.log("Battle Psychic Gym leader.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);


        pbf_move_left_joystick(context, {0, -1}, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0ms}, FlyPoint::POKECENTER);

    });  
}



}
}
}
