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
#include "PokemonSV_AutoStory_Segment_39.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_39::name() const{
    return "39: Zero Lab";
}

std::string AutoStory_Segment_39::start_text() const{
    return "Start: Inside Area Zero Station 4. Deactivated the locks.";
}

std::string AutoStory_Segment_39::end_text() const{
    return "End: Entered Zero Lab. Spoke to AI Professor.";
}

void AutoStory_Segment_39::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_102().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_103().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_102::name() const{ return "102 - " + AutoStory_Segment_39().name(); }
std::string AutoStory_Checkpoint_102::start_text() const{ return "Inside Area Zero Station 4. Deactivated the locks.";}
std::string AutoStory_Checkpoint_102::end_text() const{ return "Opened Zero lab. Defeated Paradox Pokemon.";}
void AutoStory_Checkpoint_102::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_102(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_103::name() const{ return "103 - " + AutoStory_Segment_39().name(); }
std::string AutoStory_Checkpoint_103::start_text() const{ return AutoStory_Checkpoint_102().end_text();}
std::string AutoStory_Checkpoint_103::end_text() const{ return "";}
void AutoStory_Checkpoint_103::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_103(env, context, options.notif_status_update, stats);
}


void checkpoint_102(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-lab.onnx");

        pbf_move_left_joystick(context, 128, 255, 200, 100);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 10, 255, 128);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);

        // go to Station 2
        pbf_mash_button(context, BUTTON_A, 50);
        wait_for_overworld(env.program_info(), env.console, context, 30);

        // heal at the bed
        pbf_move_left_joystick(context, 128, 0, 300, 100);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10, 255, 128);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::OVERWORLD});

        // leave Station 2
        pbf_move_left_joystick(context, 50, 255, 130, 20);

        pbf_move_left_joystick(context, 128, 255, 400, 100);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving research station
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_right_joystick(context, 180, 128, 30, 0);  // adjust camera so rock isn't at edge of screen.


        // align to rock-5-1.  
        // center before:   y:     x: 
        // center after: center-y: 0.347222   center-x: 0.664844
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-5-1", 0.347222,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-5-1", 0.5000);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-5-1", 0.664844,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move forward until we see rock-5-2
        move_player_until_yolo_object_detected(env, context, yolo_detector, "rock-5-2", 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            },
            30
        );

        // align to rock-5-2.  
        // center before:   y:     x: 
        // center after: center-y: 0.158333   center-x: 0.568750
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-5-2", 0.158333,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-5-2", 0.5000);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-5-2", 0.568750,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards rock-5-2 until box:  (0.2535, 0.25)  OR (0.34, 0.375)
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-5-2",
            0.25, 0.3,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-5-2", 0.000);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        pbf_move_right_joystick(context, {0, -1}, 1600ms, 0ms);
        move_camera_until_yolo_object_detected(env, context, yolo_detector, "beyond-cliff-5", 0, 480ms);

        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "beyond-cliff-5", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_right_joystick(context, {0, -1}, 1600ms, 0ms);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards beyond-cliff-5 until it takes up most of screen
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "beyond-cliff-5",
            0.5, 0.9,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_right_joystick(context, {0, -1}, 1600ms, 0ms);
                
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // walked off cliff. now move backwards
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60, 128, 255);  // move backwards until dialog detected


        mash_button_till_overworld(env.console, context, BUTTON_A);

        

        // align to gate-panel.  
        // center before:   y:     x: 
        // center after: center-y: 0.295833   center-x: 0.517969
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "gate-panel", 0.295833,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "gate-panel", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        
        do_action_until_dialog(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                move_player_forward(env, context, 10,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "gate-panel", 0.5,
                            [&](){
                                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
                            } 
                        );
                        pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
                    }, 
                    false,
                    true
                );
            }
        );


        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::PROMPT_DIALOG});

        // battle Paradox 1
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});

        // battle Paradox 2
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});

        // battle Paradox 3
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});


        #if 0
        // align to rock.  
        // center before:   y:     x: 
        // center after: 
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock", 0.000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock", 0.5000);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock", 0.000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards rock until box:  
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock",
            0.000, 0.000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock", 0.000);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        #endif  
    }, false);
}

void checkpoint_103(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_press_button(context, BUTTON_L, 30, 10);
                pbf_move_left_joystick(context, 255, 128, 70, 50);

                pbf_move_left_joystick(context, 110, 0, 10, 50);
                pbf_press_button(context, BUTTON_L, 30, 10);

                pbf_move_left_joystick(context, 128, 0, 600, 50);
                wait_for_overworld(env.program_info(), env.console, context, 30);
                
            }
        ); 

        pbf_move_left_joystick(context, 128, 0, 300, 50);
        pbf_move_left_joystick(context, 255, 128, 200, 50);

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 10);
        mash_button_till_overworld(env.console, context, BUTTON_A);

    }, false);
}




}
}
}
