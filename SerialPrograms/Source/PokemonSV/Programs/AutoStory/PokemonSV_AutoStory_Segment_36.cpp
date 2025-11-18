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
std::string AutoStory_Checkpoint_97::end_text() const{ return "";}
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
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock", 0.000);
                pbf_move_left_joystick(context, 128, 0, 10, 50); // move forward to align with camera
            }
        );

        #endif


    });  
}

void checkpoint_97(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
}

// void checkpoint_98(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }

// void checkpoint_99(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }




}
}
}
