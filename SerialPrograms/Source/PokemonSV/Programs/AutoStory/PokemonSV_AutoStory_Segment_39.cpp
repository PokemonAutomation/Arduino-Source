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
    return "";
}

std::string AutoStory_Segment_39::start_text() const{
    return "Start: ";
}

std::string AutoStory_Segment_39::end_text() const{
    return "End: ";
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

    // AutoStory_Checkpoint_10().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_102::name() const{ return "0102 - " + AutoStory_Segment_39().name(); }
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

void checkpoint_102(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    // checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    // [&](size_t attempt_number){

        // YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-station-4b.onnx");

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
    // });
}

void checkpoint_103(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    // checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    // [&](size_t attempt_number){


    // });
}

void checkpoint_104(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

void checkpoint_105(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}



}
}
}
