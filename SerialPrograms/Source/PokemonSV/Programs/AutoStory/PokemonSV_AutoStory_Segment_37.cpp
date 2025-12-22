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
#include "PokemonSV_AutoStory_Segment_37.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_37::name() const{
    return "37: Area Zero Station 3";
}

std::string AutoStory_Segment_37::start_text() const{
    return "Start: Inside Area Zero Station 2. Deactivated the locks.";
}

std::string AutoStory_Segment_37::end_text() const{
    return "End: Inside Area Zero Station 3. Deactivated the locks.";
}

void AutoStory_Segment_37::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_98().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_99().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_98::name() const{ return "098 - " + AutoStory_Segment_37().name(); }
std::string AutoStory_Checkpoint_98::start_text() const{ return "Inside Area Zero Station 2. Deactivated the locks.";}
std::string AutoStory_Checkpoint_98::end_text() const{ return "Outside Area Zero Station 3. Defeated Great Tusk/Iron Treads.";}
void AutoStory_Checkpoint_98::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_98(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_99::name() const{ return "099 - " + AutoStory_Segment_37().name(); }
std::string AutoStory_Checkpoint_99::start_text() const{ return AutoStory_Checkpoint_98().end_text();}
std::string AutoStory_Checkpoint_99::end_text() const{ return "Inside Area Zero Station 3. Deactivated the locks.";}
void AutoStory_Checkpoint_99::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_99(env, context, options.notif_status_update, stats);
}

void checkpoint_98(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-station-3.onnx");

        pbf_move_left_joystick(context, 128, 255, 1600ms, 800ms);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 10, 255, 128);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        pbf_press_dpad(context, DPAD_DOWN, 13, 20);

        pbf_mash_button(context, BUTTON_A, 50);
        wait_for_overworld(env.program_info(), env.console, context, 30);



        // arrived at research station 1

        pbf_move_left_joystick(context, 0, 255, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving research station
        wait_for_overworld(env.program_info(), env.console, context, 30);


        // left research station 1


        // align to rock-3-0.  
        // center before: center-y: 0.618056   center-x: 0.260156
        // center after: center-y: 0.248611   center-x: 0.452344
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-3-0", 0.248611,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3-0", 0.260156);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-3-0", 0.452344,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move forward until rock-3-0 not detected
        move_forward_until_yolo_object_not_detected(env, context, yolo_detector, "rock-3-0", 
            2,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3-0", 0.452344);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );


        pbf_press_button(context, BUTTON_L, 240ms, 100ms);

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "waterfall-3-1", 0, 800ms);



        // align to waterfall-3-1.  
        // center before: y:0.22         x: 0.31
        // center after: center-y: 0.220833   center-x: 0.754687
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "waterfall-3-1", 0.220833,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "waterfall-3-1", 0.31);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "waterfall-3-1", 0.754687,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // walk off the cliff


        // move forward until we see rock-3-1
        move_player_until_yolo_object_detected(env, context, yolo_detector, "rock-3-1", 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
                pbf_press_button(context, BUTTON_L, 240ms, 100ms);
                move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "waterfall-3-1", 0.754687,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
                    }        
                );
            },
            30
        );



        // align to rock-3-1.  
        // center before:  center-y: 0.538889   center-x: 0.384375
        // center after: center-y: 0.295833   center-x: 0.577344
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-3-1", 0.295833,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3-1", 0.4);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-3-1", 0.577344,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move forward until rock-3-1 not detected
        move_forward_until_yolo_object_not_detected(env, context, yolo_detector, "rock-3-1", 
            1,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3-1", 0.577344);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        // align to tree-3-1.  
        // center before: center-y: 0.189   center-x: 0.21
        // center after: center-y: 0.318056   center-x: 0.522656
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "tree-3-1", 0.318056,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-3-1", 0.3);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "tree-3-1", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards tree-3-1 until box:   {0.428125, 0.000000, 0.118750, 0.305556}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "tree-3-1",
            0.118, 0.2,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-3-1", 0.5);  // realign to target X
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        // align to tree-3-1.  
        // center before: center-y: 0.150000   center-x: 0.517969
        // center after: center-y: 0.151389   center-x: 0.358594
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "tree-3-1", 0.151389,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-3-1", 0.5);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "tree-3-1", 0.358594,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move forward until tree-3-1 not detected
        move_forward_until_yolo_object_not_detected(env, context, yolo_detector, "tree-3-1", 
            1,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-3-1", 0.5);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );



        pbf_move_right_joystick(context, {0, -1}, 1600ms, 0ms);
        move_camera_until_yolo_object_detected(env, context, yolo_detector, "beyond-cliff-3", 255, 240ms);

        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "beyond-cliff-3", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_right_joystick(context, {0, -1}, 1600ms, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards beyond-cliff-3 until it takes up most of screen
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "beyond-cliff-3",
            0.5, 0.9,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_right_joystick(context, {0, -1}, 1600ms, 0ms);
                
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        // walked off cliff. now move backwards

        pbf_move_left_joystick(context, 128, 255, 800, 50); // move forward to align with camera

        pbf_press_button(context, BUTTON_L, 240ms, 100ms);

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "station-3", 0, 800ms);


        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-3", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );


        do_action_until_dialog(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                move_player_forward(env, context, 20,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-3", 0.5,
                            [&](){
                                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
                            } 
                        );
                        pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
                    }
                );
            }
        );


        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

    }, false);   
}

void checkpoint_99(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
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
                                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
                            } 
                        );
                        pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
                    }
                );
            }
        );

        mash_button_till_overworld(env.console, context, BUTTON_A);  // black dialog

        // disable Lock at Station 3
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20);
        mash_button_till_overworld(env.console, context, BUTTON_A);  // black dialog, prompt


    }, false);     
}


}
}
}
