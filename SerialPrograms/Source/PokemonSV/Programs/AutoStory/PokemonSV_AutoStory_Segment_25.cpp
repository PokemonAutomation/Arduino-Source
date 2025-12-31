/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_25.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_25::name() const{
    return "25: Team Star (Poison)";
}

std::string AutoStory_Segment_25::start_text() const{
    return "Start: Beat Orthworm Titan. At East Province (Area Three) Pokecenter.";
}

std::string AutoStory_Segment_25::end_text() const{
    return "End: Beat Team Star (Poison). At Medali Pokecenter.";
}

void AutoStory_Segment_25::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_58().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_59().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_60().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_58::name() const{ return "058 - " + AutoStory_Segment_25().name(); }
std::string AutoStory_Checkpoint_58::start_text() const{ return "Beat Orthworm. At East Province (Area Three) Pokecenter.";}
std::string AutoStory_Checkpoint_58::end_text() const{ return "Beat Orthworm. At East Province (Area Three) Pokecenter.";}
void AutoStory_Checkpoint_58::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_58(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_59::name() const{ return "059 - " + AutoStory_Segment_25().name(); }
std::string AutoStory_Checkpoint_59::start_text() const{ return AutoStory_Checkpoint_58().end_text();}
std::string AutoStory_Checkpoint_59::end_text() const{ return "Beat Team Star (Poison)";}
void AutoStory_Checkpoint_59::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_59(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_60::name() const{ return "060 - " + AutoStory_Segment_25().name(); }
std::string AutoStory_Checkpoint_60::start_text() const{ return AutoStory_Checkpoint_59().end_text();}
std::string AutoStory_Checkpoint_60::end_text() const{ return "At Medali Pokecenter.";}
void AutoStory_Checkpoint_60::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_60(env, context, options.notif_status_update, stats);
}

void checkpoint_58(
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

void checkpoint_59(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        context.wait_for_all_requests();

        // marker 1  {0.795312, 0.626852}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::FAST_TRAVEL, 
            {0.795312, 0.626852}
        );

        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, +1, -0.409, 1040ms});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, +0.453, 1040ms});
        }
        

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // marker 2   {0.672396, 0.532407}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.672396, 0.532407}
        );

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 40, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, {-1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // speak to Clive
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // marker 3   {0.685417, 0.571296}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
            FlyPoint::POKECENTER, 
            {0.685417, 0.571296}
        );

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    0, +1, 20, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, {-1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // battle Team Star Grunt
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW, CallbackEnum::BATTLE});
        env.console.log("Battle team star grunt.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);


        context.wait_for_all_requests();
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, -0.688, -1, 400ms);
                walk_forward_while_clear_front_path(env.program_info(), env.console, context, 800ms);
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
            }
        );
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        NoMinimapWatcher no_minimap(env.console.logger(), COLOR_RED, Milliseconds(250));
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){

                DirectionDetector direction;
                Milliseconds wait = 8000ms;
 
                direction.change_direction(env.program_info(), env.console, context, 2.618972);
                pbf_move_left_joystick(context, {0, +1}, 800ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);
                pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);

                direction.change_direction(env.program_info(), env.console, context, 3.265261);
                pbf_move_left_joystick(context, {0, +1}, 2400ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 2.411131);
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);

                pbf_move_left_joystick(context, {0, +1}, 2400ms, 400ms);

                direction.change_direction(env.program_info(), env.console, context, 2.828856);
                pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 2.306254);
                pbf_move_left_joystick(context, {0, +1}, 4000ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 2.565727);
                pbf_move_left_joystick(context, {0, +1}, 2400ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 3.036098);
                pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);

                direction.change_direction(env.program_info(), env.console, context, 3.525225);
                pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 4.051901);
                pbf_move_left_joystick(context, {0, +1}, 5600ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 4.573712);
                pbf_move_left_joystick(context, {0, +1}, 4000ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);


                // wait 30 seconds to let the trainers cooldown
                pbf_wait(context, 30000ms);

                direction.change_direction(env.program_info(), env.console, context, 1.310148);
                pbf_move_left_joystick(context, {0, +1}, 6400ms, 400ms);
                pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                pbf_wait(context, wait);
                
                try {
                    direction.change_direction(env.program_info(), env.console, context, 0.572754);
                    pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                    pbf_move_left_joystick(context, {0, +1}, 5600ms, 400ms);
                    pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                    pbf_wait(context, wait);

                    direction.change_direction(env.program_info(), env.console, context, 6.036137);
                    pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
                    pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                    pbf_wait(context, wait);

                    direction.change_direction(env.program_info(), env.console, context, 5.779599);
                    pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);
                    pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                    pbf_wait(context, wait);

                    pbf_move_left_joystick(context, {0, +1}, 4800ms, 400ms);
                    pbf_press_button(context, BUTTON_R, 160ms, 160ms);
                    pbf_wait(context, wait);


                    pbf_wait(context, 20000ms);
                }catch (OperationFailedException&){
                    env.console.log("Failed to change direction, but it's possibly due to clearing the challenge.");
                    pbf_wait(context, 20000ms);
                }
                
            },
            {no_minimap}
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Team Star Poison: Failed to kill 30 pokemon with Let's go.",
                env.console
            );            
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle the Team Star (Poison) boss.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

    });    

}

void checkpoint_60(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // fly back to Porto Marinada Pokecenter from Team Star Poison
        // this clears Pokemon in minimap
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, -1, 0, 4800ms}, FlyPoint::POKECENTER);


        move_from_porto_marinada_to_medali(env, context);
    });    

}

void move_from_porto_marinada_to_medali(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    // marker 1
    realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, +1, +0.141, 600ms);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );          

    // marker 2.   x=0.3875, y=0.60463
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.3875, 0.60463}
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

    
    // marker 3.  : x=0.316146, y=0.623148
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.316146, 0.623148}
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

    get_on_ride(env.program_info(), env.console, context);

    // marker 4. cross bridge 1  x=0.310417, y=0.712963. 
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.310417, 0.712963}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 40, 20, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 500ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 1000ms);
            pbf_move_left_joystick(context, {0, +1}, 500ms, 0ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    get_off_ride(env.program_info(), env.console, context);


    // marker 5.  : x=0.582292, y=0.692593
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.582292, 0.692593}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 60, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    get_on_ride(env.program_info(), env.console, context);

    // marker 6. cross bridge 2 :   x=0.555208, y=0.627778
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.555208, 0.627778}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 500ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 1000ms);
            pbf_move_left_joystick(context, {0, +1}, 500ms, 0ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    get_off_ride(env.program_info(), env.console, context);

    // marker 7.  :  x=0.678646, y=0.669444
    place_marker_offset_from_flypoint(env.program_info(), env.console, context,
        {ZoomChange::KEEP_ZOOM, +1, -1, 240ms},
        FlyPoint::POKECENTER, 
        {0.678646, 0.669444}
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

    // marker 8.  :  x=0.533333, y=0.640741
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, +1, -1, 400ms},
        FlyPoint::POKECENTER, 
        {0.533333, 0.640741}
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


    // marker 9. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, -1, 400ms},
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}
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

    // marker 10. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, +0.409, -1, 400ms);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        0, +0.883, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}





}
}
}
