/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_24.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_24::name() const{
    return "24: Orthworm Titan: Battle Orthworm Titan";
}

std::string AutoStory_Segment_24::start_text() const{
    return "Start: At East Province (Area Three) Watchtower.";
}

std::string AutoStory_Segment_24::end_text() const{
    return "End: Beat Orthworm Titan. At East Province (Area Three) Pokecenter.";
}

void AutoStory_Segment_24::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_55().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_56().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_57().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_55::name() const{ return "055 - " + AutoStory_Segment_24().name(); }
std::string AutoStory_Checkpoint_55::start_text() const{ return "At East Province (Area Three) Watchtower.";}
std::string AutoStory_Checkpoint_55::end_text() const{ return "Beat Orthworm phase 1";}
void AutoStory_Checkpoint_55::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_55(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_56::name() const{ return "056 - " + AutoStory_Segment_24().name(); }
std::string AutoStory_Checkpoint_56::start_text() const{ return AutoStory_Checkpoint_55().end_text();}
std::string AutoStory_Checkpoint_56::end_text() const{ return "Beat Orthworm phase 2";}
void AutoStory_Checkpoint_56::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_56(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_57::name() const{ return "057 - " + AutoStory_Segment_24().name(); }
std::string AutoStory_Checkpoint_57::start_text() const{ return AutoStory_Checkpoint_56().end_text();}
std::string AutoStory_Checkpoint_57::end_text() const{ return "At East Province (Area Three) Pokecenter.";}
void AutoStory_Checkpoint_57::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_57(env, context, options.notif_status_update, stats);
}



void checkpoint_55(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            
            DirectionDetector direction;
            if (attempt_number >= 0){
                env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
                move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 128, 50});
                move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 0, 100, 130}, FlyPoint::FAST_TRAVEL);
            }

            direction.change_direction(env.program_info(), env.console, context, 3.909067);
            pbf_move_left_joystick(context, 128, 0, 1000, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.061720);        
            pbf_move_left_joystick(context, 128, 0, 500, 100);
            pbf_move_left_joystick(context, 255, 0, 200, 100);

            // now aligned to the wall next to the hole/passage

            // walk away from wall slightly
            pbf_move_left_joystick(context, 128, 255, 50, 100);
            get_on_ride(env.program_info(), env.console, context);

            direction.change_direction(env.program_info(), env.console, context, 0.366);
            pbf_move_left_joystick(context, 128, 0, 250, 100);

            direction.change_direction(env.program_info(), env.console, context, 2.565);
            // run at Orthworm. run into its second position as well.
            pbf_move_left_joystick(context, 128, 0, 50, 0);
            pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, {0, +1}, {0, 0}, 4000ms);
            pbf_move_left_joystick(context, 255, 0, 300, 500);

            get_off_ride(env.program_info(), env.console, context);

            direction.change_direction(env.program_info(), env.console, context, 0.261);
            pbf_move_left_joystick(context, 128, 0, 500, 100);
            pbf_move_left_joystick(context, 0, 0, 500, 100);

            // now aligned to corner.

            direction.change_direction(env.program_info(), env.console, context, 3.736);
            pbf_move_left_joystick(context, 128, 0, 400, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.306);
            pbf_move_left_joystick(context, 128, 0, 700, 100);

            
            direction.change_direction(env.program_info(), env.console, context, 4.988);
            pbf_move_left_joystick(context, 128, 0, 800, 100);
            pbf_move_left_joystick(context, 255, 0, 500, 100);

            // now aligned to the wall next to the hole/passage   

            // walk away from wall slightly
            pbf_move_left_joystick(context, 128, 255, 100, 100);  

            direction.change_direction(env.program_info(), env.console, context, 5.722795);
            pbf_move_left_joystick(context, 128, 0, 600, 100);

            direction.change_direction(env.program_info(), env.console, context, 0.625226);
        });

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);

        env.console.log("Battle Orthworm Titan phase 1.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);

    });    

}

void checkpoint_56(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            
            DirectionDetector direction;
            // we hope minimap is clear of Pokemon, after Orthworm phase 1

            direction.change_direction(env.program_info(), env.console, context, 5.042435);
            pbf_move_left_joystick(context, 128, 0, 900, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.360763);
            pbf_move_left_joystick(context, 128, 0, 500, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.85);
            pbf_move_left_joystick(context, 128, 0, 700, 100);

            direction.change_direction(env.program_info(), env.console, context, 5.428);
            pbf_move_left_joystick(context, 128, 0, 600, 100);

            direction.change_direction(env.program_info(), env.console, context, 4.908646);
            pbf_move_left_joystick(context, 128, 0, 300, 100);

            direction.change_direction(env.program_info(), env.console, context, 1.169728);
            pbf_move_left_joystick(context, 128, 0, 200, 100);
            pbf_move_left_joystick(context, 255, 0, 200, 100);
            pbf_move_left_joystick(context, 0, 0, 200, 100);

            // now aligned to the wall next to the hole/passage   

            // walk backwards
            direction.change_direction(env.program_info(), env.console, context, 2.303077);
            pbf_move_left_joystick(context, 128, 255, 400, 100);
            direction.change_direction(env.program_info(), env.console, context, 3.908360);

            get_on_ride(env.program_info(), env.console, context);

            // charge at orthworm
            pbf_move_left_joystick(context, 128, 0, 408ms, 0ms);
            pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, {0, +1}, {0, 0}, 102ms);
            pbf_move_left_joystick(context, 128, 0, 1970ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 432ms);
            pbf_move_left_joystick(context, 128, 0, 1993ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 301ms);
            pbf_move_left_joystick(context, 128, 0, 307ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 194ms);
            pbf_move_left_joystick(context, 128, 0, 886ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 626ms);
            pbf_move_left_joystick(context, 128, 0, 2651ms, 0ms);
        });

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30);

        // battle the titan phase 2
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});  
        env.console.log("Battle Orthworm Titan phase 2.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

    }, false);    

}

void checkpoint_57(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // fly back to East Province (Area Three) Watchtower
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0}, FlyPoint::FAST_TRAVEL);

        // marker 1
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.769792, 0.725926}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 2
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
            FlyPoint::POKECENTER, 
            {0.280208, 0.447222}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 3
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 128, 60}, 
            FlyPoint::POKECENTER, 
            {0.354167, 0.375}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 4
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 128, 50}, 
            FlyPoint::POKECENTER, 
            {0.497917, 0.274074}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 5. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 0},
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}
        );  

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 0, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );             

        // marker 6. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 30);
        });      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
          

        fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

        
    });    

}





}
}
}
