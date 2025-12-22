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
#include "PokemonSV_AutoStory_Segment_28.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_28::name() const{
    return "28: Team Star (Fairy)";
}

std::string AutoStory_Segment_28::start_text() const{
    return "Start: Beat Dondozo/Tatsugiri Titan. At North Province Area Three Pokecenter.";
}

std::string AutoStory_Segment_28::end_text() const{
    return "End: Beat Team Star (Fairy). At Montenevera Pokecenter.";
}

void AutoStory_Segment_28::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_68().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_69().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_70().run_checkpoint(env, context, options, stats);    

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_68::name() const{ return "068 - " + AutoStory_Segment_28().name(); }
std::string AutoStory_Checkpoint_68::start_text() const{ return "At North Province Area Three Pokecenter";}
std::string AutoStory_Checkpoint_68::end_text() const{ return "At North Province Area Three Pokecenter";}
void AutoStory_Checkpoint_68::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_68(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_69::name() const{ return "069 - " + AutoStory_Segment_28().name(); }
std::string AutoStory_Checkpoint_69::start_text() const{ return AutoStory_Checkpoint_68().end_text();}
std::string AutoStory_Checkpoint_69::end_text() const{ return "Beat Team Star (Fairy)";}
void AutoStory_Checkpoint_69::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_69(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_70::name() const{ return "070 - " + AutoStory_Segment_28().name(); }
std::string AutoStory_Checkpoint_70::start_text() const{ return AutoStory_Checkpoint_69().end_text();}
std::string AutoStory_Checkpoint_70::end_text() const{ return "At Montenevera Pokecenter";}
void AutoStory_Checkpoint_70::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_70(env, context, options.notif_status_update, stats);
}



void checkpoint_68(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // empty checkpoint, to preserve ordering
        
    
    }, false);
    
}

void checkpoint_69(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();
        // move the marker elsewhere, away from North
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 255, 50);

        DirectionDetector direction;
        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 128, 255, 50});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 128, 0, 50});
        }

        direction.change_direction(env.program_info(), env.console, context, 0.696613);

        pbf_move_left_joystick(context, 128, 0, 200, 50);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 150, 0, 120);
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

        // talk to Clavell
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 0, 100);
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


        // battle Team Star Grunt
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW, CallbackEnum::BATTLE});
        env.console.log("Battle Mr. Harrington.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);


        context.wait_for_all_requests();
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                // don't use marker since it covers up north
                DirectionDetector direction;

                direction.change_direction(env.program_info(), env.console, context, 0.261697);
                pbf_move_left_joystick(context, 128, 0, 10, 50);
                
                walk_forward_while_clear_front_path(env.program_info(), env.console, context, 100);
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
            }
        );
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});

        NoMinimapWatcher no_minimap(env.console.logger(), COLOR_RED, Milliseconds(250));
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                
                Milliseconds wait = 6000ms;
 
                DirectionDetector direction;

                direction.change_direction(env.program_info(), env.console, context, 0.611062);
                pbf_move_left_joystick(context, 128, 0, 200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);


                direction.change_direction(env.program_info(), env.console, context, 5.550472);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 5.860926);
                pbf_move_left_joystick(context, 128, 0, 400, 50);

                direction.change_direction(env.program_info(), env.console, context, 4.920249);
                pbf_move_left_joystick(context, 128, 0, 100, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 0.402877);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 5.707655);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 5.394366);
                pbf_move_left_joystick(context, 128, 0, 400, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 0.979935);
                pbf_move_left_joystick(context, 128, 0, 500, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context,  1.552260);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                direction.change_direction(env.program_info(), env.console, context, 1.869166);
                pbf_move_left_joystick(context, 128, 0, 200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);


                direction.change_direction(env.program_info(), env.console, context, 0.769966);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);

                

                direction.change_direction(env.program_info(), env.console, context, 6.127338);
                pbf_move_left_joystick(context, 128, 0, 300, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, wait);


                try {
                    direction.change_direction(env.program_info(), env.console, context, 1.451616);
                    pbf_move_left_joystick(context, 128, 0, 500, 50);
                    pbf_press_button(context, BUTTON_R, 20, 20);
                    pbf_wait(context, wait);
                    

                    direction.change_direction(env.program_info(), env.console, context, 2.023052);
                    pbf_move_left_joystick(context, 128, 0, 500, 50);
                    pbf_press_button(context, BUTTON_R, 20, 20);
                    pbf_wait(context, wait);
                    

                    direction.change_direction(env.program_info(), env.console, context, 1.13517);
                    pbf_move_left_joystick(context, 128, 0, 1000, 50);
                    pbf_press_button(context, BUTTON_R, 20, 20);
                    pbf_wait(context, wait);
                    
                    direction.change_direction(env.program_info(), env.console, context, 0.609735);
                    pbf_move_left_joystick(context, 128, 0, 200, 50);
                    pbf_press_button(context, BUTTON_R, 20, 20);
                    pbf_wait(context, wait);
                }catch (OperationFailedException&){
                    env.console.log("Failed to change direction, but it's possibly due to clearing the challenge.");
                    pbf_wait(context, 20000ms);
                }

                // ///////////////////////
                

                // direction.change_direction(env.program_info(), env.console, context, 0);
                // pbf_move_left_joystick(context, 128, 0, 00, 50);
                // pbf_press_button(context, BUTTON_R, 20, 20);
                // pbf_wait(context, wait);


                pbf_wait(context, 20000ms);
                
            },
            {no_minimap}
        );
        context.wait_for(std::chrono::milliseconds(100));
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Team Star Fairy: Failed to kill 30 pokemon with Let's go.",
                env.console
            );            
        }
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle the Team Star (Fairy) boss.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

    });
}

void checkpoint_70(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        context.wait_for_all_requests();
        // fly back to North Province Area Three
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 0, 0}, FlyPoint::POKECENTER);

        // fly back to Glaseado Mountain Pokecenter
        // this clears Pokemon in minimap
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 0, 0, 0}, FlyPoint::POKECENTER);

        move_from_glaseado_mountain_to_montenevera(env, context);

    });
}


void move_from_glaseado_mountain_to_montenevera(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();


    // marker 1         {0.745313, 0.637037}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.745313, 0.637037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );
    
    // marker 2       x=0.225521, y=0.380556. {0.229687, 0.37037}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 0, 50}, 
        FlyPoint::POKECENTER, 
        {0.229687, 0.37037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 60, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 3       x=0.396354, y=0.69537.  x=0.396875, y=0.725926
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 200, 50}, 
        FlyPoint::POKECENTER, 
        {0.396875, 0.725926}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 50, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 4       (0.409896, 0.65)
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.409896, 0.65}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    
    // marker 5        {0.509896, 0.639815}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.509896, 0.639815}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6         {0.508333, 0.566667}   {0.521875, 0.483333}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 128, 255, 20}, 
        FlyPoint::POKECENTER, 
        {0.508333, 0.566667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 7         {0.508333, 0.566667}   {0.521875, 0.483333}  {0.529687, 0.483333}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.529687, 0.483333}
    );

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 500ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 1000ms);
            pbf_move_left_joystick(context, 128, 0, 500ms, 0ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


}





}
}
}
