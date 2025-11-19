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
#include "PokemonSV_AutoStory_Segment_36.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_36::name() const{
    return "36: Area Zero Station 2";
}

std::string AutoStory_Segment_36::start_text() const{
    return "Start: Inside Area Zero Station 1. Deactivated the locks.";
}

std::string AutoStory_Segment_36::end_text() const{
    return "End: Inside Area Zero Station 2. Deactivated the locks.";
}

void AutoStory_Segment_36::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_96().run_checkpoint(env, context, options, stats);
    // AutoStory_Checkpoint_97().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_96::name() const{ return "096 - " + AutoStory_Segment_36().name(); }
std::string AutoStory_Checkpoint_96::start_text() const{ return "Inside Area Zero Station 1. Deactivated the locks.";}
std::string AutoStory_Checkpoint_96::end_text() const{ return "Outside Area Zero Station 2. Defeated Scream Tail/Iron Bundle.";}
void AutoStory_Checkpoint_96::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_96(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_97::name() const{ return "097 - " + AutoStory_Segment_36().name(); }
std::string AutoStory_Checkpoint_97::start_text() const{ return AutoStory_Checkpoint_96().end_text();}
std::string AutoStory_Checkpoint_97::end_text() const{ return "Inside Area Zero Station 2. Deactivated the locks.";}
void AutoStory_Checkpoint_97::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_97(env, context, options.notif_status_update, stats);
}

// std::string AutoStory_Checkpoint_98::name() const{ return "098 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_98::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_98::end_text() const{ return "";}
// void AutoStory_Checkpoint_98::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_98(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_99::name() const{ return "099 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_99::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_99::end_text() const{ return "";}
// void AutoStory_Checkpoint_99::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_99(env, context, options.notif_status_update, stats);
// }



void checkpoint_96(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-station-2.onnx");
        
    #if 0
    #endif

        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after leaving research station
        wait_for_overworld(env.program_info(), env.console, context, 30);
        

        // direction
        // center-y: 0.326389   center-x: 0.660156
        // center-y: 0.177778   center-x: 0.765625
        // 

        // move towards tree-2-1 until box:   {0.721875, -0.005556, 0.087500, 0.366667}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "tree-2-1",
            0.0900, 0.15,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-2-1", 0.7);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "rock-2-1", 0, 30);

        // align to rock-2-1.  
        // center before: center-y: 0.604167   center-x: 0.238281
        // center after: center-y: 0.297222   center-x: 0.355469
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-2-1", 0.297222,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-2-1", 0.25);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-2-1", 0.355469,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards rock-2-1 until box:  {0.007812, 0.363889, 0.203125, 0.300000}. center-y: 0.513889   center-x: 0.109375
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-2-1",
            0.15, 0.300,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-2-1", 0.15);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );


        // align to rock-2-2.  
        // center before:  center-y: 0.426   center-x: 0.239
        // center after: center-y: 0.273611   center-x: 0.494531
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-2-2", 0.273611,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-2-2", 0.25);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-2-2", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards rock-2-2 until box:  {0.385938, 0.175000, 0.223438, 0.355556}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-2-2",
            0.223438, 0.30,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-2-2", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );


        // align to tree-2-2.  
        // center before: center-y: 0.35   center-x: 0.205
        // center after: center-y: 0.315278   center-x: 0.498437
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "tree-2-2", 0.315278,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-2-2", 0.25);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "tree-2-2", 0.498437,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards tree-2-2 until box:  {0.446875, 0.025000, 0.081250, 0.319444}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "tree-2-2",
            0.081250, 0.30,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-2-2", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // align to dirt-ramp.  
        // center before: center-y: 0.3472   center-x: 0.211
        // center after: center-y: 0.354167   center-x: 0.507031
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "dirt-ramp", 0.354167,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "dirt-ramp", 0.3);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "dirt-ramp", 0.507031,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move forward until dirt-ramp not detected
        move_forward_until_yolo_object_not_detected(env, context, yolo_detector, "dirt-ramp", 
            6,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "dirt-ramp", 0.5);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        pbf_press_button(context, BUTTON_L, 240ms, 100ms);

        // confirm we can see station-2
        move_forward_until_yolo_object_detected(env, context, yolo_detector, "station-2", 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            },
            5
        );

        // align to station-2.  
        // center before: 
        // center after: center-y: 0.388889   center-x: 0.517969
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "station-2", 0.388889,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-2", 0.5);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-2", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards station-2 until box:   {0.364063, 0.219444, 0.209375, 0.127778}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "station-2",
            0.209375, 0.127778,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-2", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        do_action_until_dialog(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                move_player_forward(env, context, 20,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-2", 0.5,
                            [&](){
                                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
                            } 
                        );
                        pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
                    }
                );
            }
        );

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        

        #if 0
        // align to rock.  
        // center before: 
        // center after: 
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock", 0.000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock", 0.000);  // x-position of target object prior to camera move
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


    });  
}

void checkpoint_97(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/station-door-1.onnx");  // we can reuse the detector for station door 1.

        do_action_until_dialog(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_press_button(context, BUTTON_R, 160ms, 0ms);
                move_player_forward(env, context, 20,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-door-1", 0.5,
                            [&](){
                                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
                            } 
                        );
                        pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
                    }
                );
            }
        );

        mash_button_till_overworld(env.console, context, BUTTON_A);  // black dialog

        // disable Lock at Station 2
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 120, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX, CallbackEnum::PROMPT_DIALOG, CallbackEnum::WHITE_A_BUTTON});


    });     
}

// void checkpoint_98(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_99(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }




}
}
}
