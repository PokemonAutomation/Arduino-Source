/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ML/Inference/ML_YOLOv5Detector.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_35.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


using namespace ML;

std::string AutoStory_Segment_35::name() const{
    return "35: Area Zero Gate, Station 1";
}

std::string AutoStory_Segment_35::start_text() const{
    return "Start: Beat Nemona, Penny, and Arven. At Los Platos Pokecenter.";
}

std::string AutoStory_Segment_35::end_text() const{
    return "End: Inside Area Zero Station 1. Deactivated the locks.";
}

void AutoStory_Segment_35::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_93().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_94().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_95().run_checkpoint(env, context, options, stats);


    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_93::name() const{ return "093 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_93::start_text() const{ return "Beat Arven. At Los Platos Pokecenter.";}
std::string AutoStory_Checkpoint_93::end_text() const{ return "Entered Area Zero Gate. Flew down to Area Zero from the platform.";}
void AutoStory_Checkpoint_93::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_93(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_94::name() const{ return "094 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_94::start_text() const{ return AutoStory_Checkpoint_93().end_text();}
std::string AutoStory_Checkpoint_94::end_text() const{ return "Outside Area Zero Station 1. Defeated Glimmora.";}
void AutoStory_Checkpoint_94::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_94(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_95::name() const{ return "095 - " + AutoStory_Segment_35().name(); }
std::string AutoStory_Checkpoint_95::start_text() const{ return AutoStory_Checkpoint_94().end_text();}
std::string AutoStory_Checkpoint_95::end_text() const{ return "Inside Area Zero Station 1. Deactivated the locks.";}
void AutoStory_Checkpoint_95::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_95(env, context, options.notif_status_update, stats);
}


void checkpoint_93(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // fly to Medali West from Los Platos Pokecenter
        // this clears Pokemon in minimap
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 95, 0, 170}, FlyPoint::POKECENTER);

        // marker 1     {0.580729, 0.286111}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.580729, 0.286111}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 255, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 2    {0.475, 0.4}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.475, 0.4}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 3    {0.473958, 0.260185}      {0.479687, 0.250926}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 255, 30}, 
            FlyPoint::POKECENTER, 
            {0.479687, 0.250926}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 4     {0.425, 0.289815}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.425, 0.289815}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 5     {0.465104, 0.292593}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 255, 0, 20}, 
            FlyPoint::POKECENTER, 
            {0.465104, 0.292593}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 6     {0.439583, 0.274074}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_OUT, 255, 255, 20}, 
            FlyPoint::POKECENTER, 
            {0.439583, 0.274074}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 255, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 120, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX});
        pbf_move_left_joystick(context, {0, +1}, 800ms, 400ms);
        pbf_move_left_joystick(context, 255, 70, 300, 50);
        mash_button_till_overworld(env.console, context, BUTTON_A);
        // clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 120, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX, CallbackEnum::PROMPT_DIALOG});

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30000ms);
        mash_button_till_overworld(env.console, context, BUTTON_A); // prompt, black dialog. random A press

    }); 

}

void checkpoint_94(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){


        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/A0-station-1.onnx");


        move_player_forward(env, context, 6,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            }
        );

        // confirm we can see tree-tera
        move_player_until_yolo_object_detected(env, context, yolo_detector, "tree-tera", 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            },
            5
        );
        // align to tree-tera. center-y: 0.294444   center-x: 0.604688
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "tree-tera", 0.294444, 
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-tera", 0.277344);  // x-position of tree-tera prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "tree-tera", 0.604688,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-tera", 0.6);  // 0.277344  // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );
        
        // move towards tree-tera until {0.721875, 0.277778, 0.146875, 0.180556}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "tree-tera",
            0.146, 0.180,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-tera", 0.604688);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        // align to rock-1. center-y: 0.191667   center-x: 0.583594
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-1", 0.191667,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-1", 0.578);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-1", 0.583594,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-1", 0.58);  // 0.578  // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards rock-1 until {0.626563, 0.113889, 0.279688, 0.230556}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-1",
            0.279, 0.230,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-1", 0.583594);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );
        

        // align to rock-2. center-y: 0.194444   center-x: 0.508594
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-2", 0.194444,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo_with_recovery(env, context, yolo_detector, "rock-2", 0.25, 
                    [&](){
                        move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-1", 0.7);
                    }
                );
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-2", 0.5);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-2", 0.508594,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo_with_recovery(env, context, yolo_detector, "rock-2", 0.5, 
                //     [&](){
                //         move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-1", 0.7);
                //     }
                // );
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-2", 0.5);  // 0.223   // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        

        // move towards rock-2 until {0.471875, 0.244444, 0.103125, 0.266667}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-2",
            0.103, 0.266,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-2", 0.508594);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        // align to rock-3. center-y: 0.190278   center-x: 0.566406
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-3", 0.190278,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3", 0.724);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-3", 0.566406,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3", 0.6);  // 0.724   // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards rock-3 until   {0.598438, 0.186111, 0.121875, 0.136111}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-3",
            0.121, 0.1,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3", 0.566406);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );
        

        // align to stream.  center-y: 0.481944   center-x: 0.528906
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "stream", 0.481944,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "stream", 0.404);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "stream", 0.528906,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "stream", 0.5);  // 0.404   // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );


        // move forward until stream not detected
        move_forward_until_yolo_object_not_detected(env, context, yolo_detector, "stream", 
            3,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "stream", 0.5);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        pbf_press_button(context, BUTTON_L, 160ms, 400ms);

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "rock-3", 255, 80ms);

        // align to rock-3.  center-y: 0.291667   center-x: 0.501563
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-3", 0.291667,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3", 0.8);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-3", 0.501563,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3", 0.5015);  // 0.8   // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );


        // move towards rock-3 until {0.245313, 0.001000, 0.404688, 0.394444}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-3",
            0.404, 0.25,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-3", 0.501563);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        


        // align to rock-4.  center-y: 0.200000   center-x: 0.550781
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-4", 0.550781,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4", 0.55);  // 0.8   // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-4", 0.200000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4", 0.55);  // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards rock-4 until {0.621875, 0.202778, 0.262500, 0.177778}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-4",
            0.25, 0.177,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-4", 0.550781);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "tree-1", 0, 560ms);

        // align to tree-1.   center-y: 0.390278   center-x: 0.5
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "tree-1", 0.390278,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-1", 0.3);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "tree-1", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-1", 0.5);  // 0.3  // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards tree-1 until  {0.064063, 0.002778, 0.523438, 0.305556} 
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "tree-1",
            0.52, 0.27,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-1", 0.5);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );


        // align to tree-1.  center-y: 0.341667   center-x: 0.500000
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "tree-1", 0.341667,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-1", 0.5);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "tree-1", 0.500000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-1", 0.5);  // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards tree-1 until   {-0.064063, -0.008333, 1.073438, 0.602778}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "tree-1",
            0.8, 0.500,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "tree-1", 0.500000);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "rock-5", 255, 400ms);

        // align to rock-5.  center-y: 0.206944   center-x: 0.600000
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-5", 0.206944,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-5", 0.7);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-5", 0.600000,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-5", 0.6);  // 0.7  // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards rock-5 until   {0.548438, -0.002778, 0.271875, 0.327778}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-5",
            0.271875, 0.327778,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-5", 0.600000);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );

        move_camera_until_yolo_object_detected(env, context, yolo_detector, "rock-6", 255, 400ms);

        

        // align to rock-6.  center-y: 0.333333   center-x: 0.372656
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "rock-6", 0.333333,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-6", 0.66);  // 0.7  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "rock-6", 0.372656,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-6", 0.37);  // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        // move towards rock-6 until   {0.148438, 0.313889, 0.225000, 0.119444}
        move_forward_until_yolo_object_above_min_size(env, context, yolo_detector, "rock-6",
            0.22, 0.119,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "rock-6", 0.372656);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }
        );


        // align to station-1.  center-y: 0.286111   center-x: 0.483594
        move_camera_yolo(env, context, CameraAxis::Y, yolo_detector, "station-1", 0.286111,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                move_player_to_realign_via_yolo(env, context, yolo_detector, "station-1", 0.64);  // x-position of target object prior to camera move
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-1", 0.483594,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                // move_player_to_realign_via_yolo(env, context, yolo_detector, "station-1", 0.5);  // 0.64   // set close to target x-position of target object
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );

        do_action_until_dialog(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                move_player_forward(env, context, 10,
                    [&](){
                        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-1", 0.5,
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

void checkpoint_95(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        

        YOLOv5Detector yolo_detector(RESOURCE_PATH() + "PokemonSV/YOLO/station-door-1.onnx");
        move_player_forward(env, context, 4,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
            }, 
            true
        );
        
        move_camera_until_yolo_object_detected(env, context, yolo_detector, "station-door-1", 0, 400ms);
        move_camera_yolo(env, context, CameraAxis::X, yolo_detector, "station-door-1", 0.5,
            [&](){
                run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);
                pbf_move_left_joystick(context, {0, +1}, 80ms, 400ms); // move forward to align with camera
            }        
        );
        

        // enter Station 1
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20000ms);
        mash_button_till_overworld(env.console, context, BUTTON_A);  // black dialog

        // disable Lock at Station 1
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20000ms);
        mash_button_till_overworld(env.console, context, BUTTON_A);     // prompt, black dialog, 
        
        

    }, false);     
}




}
}
}
