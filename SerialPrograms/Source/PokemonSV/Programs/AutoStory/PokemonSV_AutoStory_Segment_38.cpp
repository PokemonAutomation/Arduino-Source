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
#include "PokemonSV_AutoStory_Segment_38.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_38::name() const{
    return "38: Area Zero Station 4";
}

std::string AutoStory_Segment_38::start_text() const{
    return "Start: Inside Area Zero Station 2. Deactivated the locks.";
}

std::string AutoStory_Segment_38::end_text() const{
    return "End: Inside Area Zero Station 3. Deactivated the locks.";
}

void AutoStory_Segment_38::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_100().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_101().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_100::name() const{ return "0100 - " + AutoStory_Segment_38().name(); }
std::string AutoStory_Checkpoint_100::start_text() const{ return "Inside Area Zero Station 3. Deactivated the locks.";}
std::string AutoStory_Checkpoint_100::end_text() const{ return "Entered inner cave. Finished cutscene, admiring the waterfall/large crystals.";}
void AutoStory_Checkpoint_100::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_100(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_101::name() const{ return "0101 - " + AutoStory_Segment_38().name(); }
std::string AutoStory_Checkpoint_101::start_text() const{ return AutoStory_Checkpoint_100().end_text();}
std::string AutoStory_Checkpoint_101::end_text() const{ return "Inside Area Zero Station 4. Deactivated the locks.";}
void AutoStory_Checkpoint_101::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_101(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_102::name() const{ return "0102 - " + AutoStory_Segment_38().name(); }
std::string AutoStory_Checkpoint_102::start_text() const{ return "";}
std::string AutoStory_Checkpoint_102::end_text() const{ return "";}
void AutoStory_Checkpoint_102::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_102(env, context, options.notif_status_update, stats);
}

// std::string AutoStory_Checkpoint_103::name() const{ return "0103 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_103::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_103::end_text() const{ return "";}
// void AutoStory_Checkpoint_103::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_103(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_104::name() const{ return "0104 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_104::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_104::end_text() const{ return "";}
// void AutoStory_Checkpoint_104::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_104(env, context, options.notif_status_update, stats);
// }

// std::string AutoStory_Checkpoint_105::name() const{ return "0105 - " + AutoStory_Segment_3().name(); }
// std::string AutoStory_Checkpoint_105::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_105::end_text() const{ return "";}
// void AutoStory_Checkpoint_105::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_105(env, context, options.notif_status_update, stats);
// }

void checkpoint_100(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-station-4a.onnx");

        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after leaving research station
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_right_joystick(context, 128, 160, 30, 0);  // adjust camera so that head doesn't cover the rock.



        // align to rock-4-1.  
        // center before: 
        // center after: center-y: 0.295833   center-x: 0.511719
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-4-1", 0.295833,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4-1", 0.5);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-4-1", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards rock-4-1 until box:  {0.404688, 0.091667, 0.246875, 0.400000}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-4-1",
            0.24, 0.3950,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4-1", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // align to cave-entrance.  
        // center before: y: 0.35    x: 0.211719
        // center after:  center-y: 0.329167   center-x: 0.517188
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "cave-entrance", 0.329167,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "cave-entrance", 0.25);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "cave-entrance", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move forward until we see rock-4-2
        move_player_until_yolo_object_detected(env, context, yolo_detector, "rock-4-2", 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "cave-entrance", 0.5,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
                    }        
                );
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            },
            30
        );

        move_player_forward(env, context, 2,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            }
        );

        // align to rock-4-2.  
        // center before: y: 0.463     x: 0.6085
        // center after:  center-y: 0.295833   center-x: 0.550781
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-4-2", 0.295833,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4-2", 0.6);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-4-2", 0.550781,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards rock-4-2 until box:  {0.595313, 0.263889, 0.089063, 0.086111}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-4-2",
            0.08, 0.086,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4-2", 0.55);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // align to rock-4-3.  
        // center before: Y: 0.211  x: 0.789
        // center after: 0.181944   center-x: 0.503125
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-4-3", 0.181944,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4-3", 0.75);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-4-3", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards rock-4-3 until box:   {0.396875, 0.047222, 0.142187, 0.211111}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-4-3",
            0.142, 0.211,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4-3", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "cave-archway-1", 255, 60);

        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "cave-archway-1", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        do_action_until_dialog(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                move_player_forward(env, context, 20,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "cave-archway-1", 0.5,
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


        mash_button_till_overworld(env.console, context, BUTTON_A);


    });     
}

void checkpoint_101(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-station-4b.onnx");

        #if 0
        #endif

        // align to crystal-4-1.  
        // center before: X: 0.84
        // center after: center-y: 0.195833   center-x: 0.621875
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "crystal-4-1", 0.195833,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-1", 0.8);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "crystal-4-1", 0.621875,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards crystal-4-1 until box:   {0.804688, 0.075000, 0.187500, 0.338889}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "crystal-4-1",
            0.1, 0.338,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-1", 0.7);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // confirm we can see cave-archway-2
        move_player_until_yolo_object_detected(env, context, yolo_detector, "cave-archway-2", 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            },
            5,
            100,
            0, 128
        );


        // align to cave-archway-2.  
        // center before: x: 0.7
        // center after:  center-y: 0.173611   center-x: 0.531250
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "cave-archway-2", 0.173611,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "cave-archway-2", 0.7);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "cave-archway-2", 0.531250,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move forward until cave-archway-2 not detected
        move_forward_until_yolo_object_not_detected(env, context, yolo_detector, "cave-archway-2", 
            3,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "cave-archway-2", 0.5);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "crystal-4-3", 255, 60);

        

        // align to crystal-4-3.  
        // center before: y:   x: 0.59
        // center after: center-y: 0.390278   center-x: 0.495313
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "crystal-4-3", 0.390278,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-3", 0.59);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "crystal-4-3", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards crystal-4-3 until box:  {0.420313, 0.052778, 0.118750, 0.263889}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "crystal-4-3",
            0.118750, 0.2,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-3", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );



        // align to crystal-4-4.  
        // center before: y:  x: 0.9
        // center after: center-y: 0.184722   center-x: 0.524219
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "crystal-4-4", 0.184722,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-4", 0.9);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "crystal-4-4", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards crystal-4-4 until box:   {0.468750, 0.052778, 0.046875, 0.102778}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "crystal-4-4",
            0.046875, 0.102778,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-4", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );


        // align to crystal-4-4.  
        // center before: y:  x: 
        // center after: center-y: 0.375000   center-x: 0.492188
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "crystal-4-4", 0.375,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-4", 0.5);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );


        // move towards crystal-4-4 until box:   {0.420313, 0.013889, 0.118750, 0.283333}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "crystal-4-4",
            0.118750, 0.25,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-4", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        pbf_move_right_joystick(context, 128, 160, 30, 0);  // adjust camera so that head doesn't cover the crystal.
  
        move_camera_until_yolo_object_detected(env, context, yolo_detector, "crystal-4-5", 255, 30);


        // align to crystal-4-5.  
        // center before: y:  x: 0.75
        // center after: center-y: 0.288889   center-x: 0.564844
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "crystal-4-5", 0.288889,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-5", 0.75);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "crystal-4-5", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards crystal-4-5 until box:  {0.407813, 0.152778, 0.207813, 0.400000}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "crystal-4-5",
            0.207813, 0.400000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "crystal-4-5", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // confirm we can see station-crystal-4
        move_player_until_yolo_object_detected(env, context, yolo_detector, "station-crystal-4", 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            },
            5,
            100,
            0, 128
        );

        // align to station-crystal-4.  
        // center before: y:    x: 0.67
        // center after: center-y: 0.456944   center-x: 0.539844
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "station-crystal-4", 0.45,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-crystal-4", 0.67);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-crystal-4", 0.539844,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards station-crystal-4 until box:  {0.423438, 0.144444, 0.393750, 0.211111}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "station-crystal-4",
            0.393, 0.2111,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-crystal-4", 0.58);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // align to station-4.  
        // center before: y:   x: 0.87
        // center after: center-y: 0.231944   center-x: 0.496875
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "station-4", 0.231944,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-4", 0.87);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-4", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards station-4 until box:  {0.339063, 0.002778, 0.284375, 0.277778}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "station-4",
            0.284375, 0.2,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-4", 0.5);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // align to station-door-4.  
        // center before: y:   x: 0.53
        // center after: center-y: 0.227778   center-x: 0.354688
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "station-door-4", 0.227778,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-door-4", 0.5);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-door-4", 0.354688,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );

        // move towards station-door-4 until box:  {0.056250, 0.194444, 0.184375, 0.316667}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "station-door-4",
            0.1, 0.316,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-door-4", 0.3);  // realign to target X
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        // align to station-door-4.  
        // center before: 
        // center after: center-y: 0.318056   center-x: 0.514844
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-door-4", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "station-door-4", 0.318056,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-door-4", 0.5);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }        
        );


        do_action_until_dialog(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                move_player_forward(env, context, 20,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-door-4", 0.5,
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


        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 120, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX});
        pbf_move_left_joystick(context, 128, 0, 100, 0);
        // disable Lock at Station 4
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20);
        mash_button_till_overworld(env.console, context, BUTTON_A);  // black dialog, prompt

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
    });
}

void checkpoint_102(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_103(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_104(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_105(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

}
}
}
