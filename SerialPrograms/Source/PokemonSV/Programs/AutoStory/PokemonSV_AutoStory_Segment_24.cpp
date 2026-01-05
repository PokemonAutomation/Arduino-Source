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
    return "24: Orthworm Titan";
}

std::string AutoStory_Segment_24::start_text() const{
    return "Start: Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.";
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
std::string AutoStory_Checkpoint_55::start_text() const{ return "Defeated Levincia Gym (Electric). At Levincia (North) Pokecenter.";}
std::string AutoStory_Checkpoint_55::end_text() const{ return "Beat Orthworm phase 1 and 2";}
void AutoStory_Checkpoint_55::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_55(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_56::name() const{ return "056 - " + AutoStory_Segment_24().name(); }
std::string AutoStory_Checkpoint_56::start_text() const{ return AutoStory_Checkpoint_55().end_text();}
std::string AutoStory_Checkpoint_56::end_text() const{ return "At East Province (Area Three) Pokecenter.";}
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

        DirectionDetector direction;
        // recently flew a significant distance, so minimap should be clear of Pokemon
        // flew from Levincia Gym to Levincia North Pokecenter.
        
        direction.change_direction(env.program_info(), env.console, context, 1.341);
        pbf_move_left_joystick(context, {0, +1}, 3600ms, 800ms);

        // marker 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, +0.252, +1, 280ms);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 24, 8, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );          

        // marker 2. x=0.411979, y=0.730556
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.411979, 0.730556}
        );        
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );         
        
        // marker 3. x=0.444792, y=0.640741. zoom out
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_OUT, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.444792, 0.640741}
        ); 

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 24, 8, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 4. blind marker placement
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::KEEP_ZOOM, 0, -1, 400ms},
            {ZoomChange::ZOOM_IN, +0.173, +1, 896ms}
        ); 
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 5. blind marker placement
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, -1, 400ms},
            {ZoomChange::ZOOM_IN, +0.055, +1, 856ms}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 16, 8, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );  
        
        // marker 6. blind marker placement
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, -1, 400ms},
            {ZoomChange::ZOOM_IN, -0.062, +1, 760ms}
        ); 
        

        // walk forward until dialog
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 16, 8, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );            

        mash_button_till_overworld(env.console, context, BUTTON_A);

        // resume marker 6
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 16, 8, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        ); 


        // marker 7. x=0.505729, y=0.675926
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, -1, 240ms},
            FlyPoint::POKECENTER, 
            {0.505729, 0.675926}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 8.  x=0.591146, y=0.575926,
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.591146, 0.575926}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 9. at crossroads. x=0.723958, y=0.55463
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.723958, 0.55463}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 10. x=0.752604, y=0.643519
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.752604, 0.643519}
        );

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );


        // marker 11. x=0.752083, y=0.702778
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.752083, 0.702778}
        ); 

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 16, 8, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 12. x=0.685417, y=0.748148
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.685417, 0.748148}
        ); 

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 16, 8, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
          
            DirectionDetector direction;

            direction.change_direction(env.program_info(), env.console, context, 3.909067);
            pbf_move_left_joystick(context, {0, +1}, 8000ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 5.061720);
            pbf_move_left_joystick(context, {0, +1}, 4000ms, 800ms);
            pbf_move_left_joystick(context, {+1, +1}, 1600ms, 800ms);

            // now aligned to the wall next to the hole/passage

            // walk away from wall slightly
            pbf_move_left_joystick(context, {0, -1}, 400ms, 800ms);
            get_on_ride(env.program_info(), env.console, context);

            direction.change_direction(env.program_info(), env.console, context, 0.366);
            pbf_move_left_joystick(context, {0, +1}, 2000ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 2.565);
            // run at Orthworm. run into its second position as well.
            pbf_move_left_joystick(context, {0, +1}, 400ms, 0ms);
            pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, {0, +1}, {0, 0}, 4000ms);
            pbf_move_left_joystick(context, {+1, +1}, 2400ms, 4000ms);

            get_off_ride(env.program_info(), env.console, context);

            direction.change_direction(env.program_info(), env.console, context, 0.261);
            pbf_move_left_joystick(context, {0, +1}, 4000ms, 800ms);
            pbf_move_left_joystick(context, {-1, +1}, 4000ms, 800ms);

            // now aligned to corner.

            direction.change_direction(env.program_info(), env.console, context, 3.736);
            pbf_move_left_joystick(context, {0, +1}, 3200ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 5.306);
            pbf_move_left_joystick(context, {0, +1}, 5600ms, 800ms);

            
            direction.change_direction(env.program_info(), env.console, context, 4.988);
            pbf_move_left_joystick(context, {0, +1}, 6400ms, 800ms);
            pbf_move_left_joystick(context, {+1, +1}, 4000ms, 800ms);

            // now aligned to the wall next to the hole/passage   

            // walk away from wall slightly
            pbf_move_left_joystick(context, {0, -1}, 800ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 5.722795);
            pbf_move_left_joystick(context, {0, +1}, 4800ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 0.625226);
        });

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30000ms);

        confirm_titan_battle(env, context);
        env.console.log("Battle Orthworm Titan phase 1.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);


        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            
            DirectionDetector direction;
            // we hope minimap is clear of Pokemon, after Orthworm phase 1

            direction.change_direction(env.program_info(), env.console, context, 5.042435);
            pbf_move_left_joystick(context, {0, +1}, 7200ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 5.360763);
            pbf_move_left_joystick(context, {0, +1}, 4000ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 5.85);
            pbf_move_left_joystick(context, {0, +1}, 5600ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 5.428);
            pbf_move_left_joystick(context, {0, +1}, 4800ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 4.908646);
            pbf_move_left_joystick(context, {0, +1}, 2400ms, 800ms);

            direction.change_direction(env.program_info(), env.console, context, 1.169728);
            pbf_move_left_joystick(context, {0, +1}, 1600ms, 800ms);
            pbf_move_left_joystick(context, {+1, +1}, 1600ms, 800ms);
            pbf_move_left_joystick(context, {-1, +1}, 1600ms, 800ms);

            // now aligned to the wall next to the hole/passage   

            // walk backwards
            direction.change_direction(env.program_info(), env.console, context, 2.303077);
            pbf_move_left_joystick(context, {0, -1}, 3200ms, 800ms);
            direction.change_direction(env.program_info(), env.console, context, 3.908360);

            get_on_ride(env.program_info(), env.console, context);

            // charge at orthworm
            pbf_move_left_joystick(context, {0, +1}, 408ms, 0ms);
            pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, {0, +1}, {0, 0}, 102ms);
            pbf_move_left_joystick(context, {0, +1}, 1970ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 432ms);
            pbf_move_left_joystick(context, {0, +1}, 1993ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 301ms);
            pbf_move_left_joystick(context, {0, +1}, 307ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 194ms);
            pbf_move_left_joystick(context, {0, +1}, 886ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {-1, 0}, 626ms);
            pbf_move_left_joystick(context, {0, +1}, 2651ms, 0ms);
        });

        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30000ms);

        // battle the titan phase 2
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});  
        confirm_titan_battle(env, context);
        env.console.log("Battle Orthworm Titan phase 2.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

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
        // fly back to East Province (Area Three) Watchtower. from Orthworm
        // this clears Pokemon in minimap
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, FlyPoint::FAST_TRAVEL);

        // marker 1
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.769792, 0.725926}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 2
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.280208, 0.447222}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 3
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, 0, 480ms},
            FlyPoint::POKECENTER, 
            {0.354167, 0.375}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 40, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 4
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, 0, 400ms},
            FlyPoint::POKECENTER, 
            {0.497917, 0.274074}
        );
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 5. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, +1, 0ms},
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}
        );  

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, +1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );             

        // marker 6. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, +1, 240ms);
        });      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            0, +0.883, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
          

        fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

        
    });    

}

void checkpoint_57(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // empty checkpoint, to preserve ordering
    }, false);
}





}
}
}
