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
#include "PokemonSV_AutoStory_Segment_31.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_31::name() const{
    return "31: Team Star (Fighting)";
}

std::string AutoStory_Segment_31::start_text() const{
    return "Start: Beat Glaseado Gym (Ice). At North Province Area One Pokecenter.";
}

std::string AutoStory_Segment_31::end_text() const{
    return "End: Beat Team Star (Fighting). At North Province Area Two Pokecenter.";
}

void AutoStory_Segment_31::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_78().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_79().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_80().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_81().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_82().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_78::name() const{ return "078 - " + AutoStory_Segment_31().name(); }
std::string AutoStory_Checkpoint_78::start_text() const{ return "At North Province Area One Pokecenter";}
std::string AutoStory_Checkpoint_78::end_text() const{ return "At North Province Area Two Pokecenter";}
void AutoStory_Checkpoint_78::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_78(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_79::name() const{ return "079 - " + AutoStory_Segment_31().name(); }
std::string AutoStory_Checkpoint_79::start_text() const{ return AutoStory_Checkpoint_78().end_text();}
std::string AutoStory_Checkpoint_79::end_text() const{ return "At North Province Area Two Pokecenter";}
void AutoStory_Checkpoint_79::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_79(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_80::name() const{ return "080 - " + AutoStory_Segment_31().name(); }
std::string AutoStory_Checkpoint_80::start_text() const{ return AutoStory_Checkpoint_79().end_text();}
std::string AutoStory_Checkpoint_80::end_text() const{ return "Beat Team Star (Fighting)";}
void AutoStory_Checkpoint_80::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_80(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_81::name() const{ return "081 - " + AutoStory_Segment_31().name(); }
std::string AutoStory_Checkpoint_81::start_text() const{ return AutoStory_Checkpoint_80().end_text();}
std::string AutoStory_Checkpoint_81::end_text() const{ return "At Alfornada Pokecenter.";}
void AutoStory_Checkpoint_81::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_81(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_82::name() const{ return "082 - " + AutoStory_Segment_31().name(); }
std::string AutoStory_Checkpoint_82::start_text() const{ return AutoStory_Checkpoint_81().end_text();}
std::string AutoStory_Checkpoint_82::end_text() const{ return "At Alfornada Pokecenter.";}
void AutoStory_Checkpoint_82::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_82(env, context, options.notif_status_update, stats);
}



void checkpoint_78(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        heal_at_pokecenter(env.program_info(), env.console, context);
        
        move_from_north_province_area_one_to_north_province_area_two(env, context, attempt_number);

    });   
}


void checkpoint_79(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // empty checkpoint

    }, false);
}

void checkpoint_80(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        beat_team_star_fighting1(env, context, attempt_number);
        beat_team_star_fighting2(env, context);
    });
}

void checkpoint_81(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 0, 170, 550}, FlyPoint::POKECENTER);
        move_from_west_province_area_one_north_to_alfornada(env, context);
    });
}

void checkpoint_82(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        // empty checkpoint

    }, false);  
}



void move_from_north_province_area_one_to_north_province_area_two(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t attempt_number){
    DirectionDetector direction;
    if (attempt_number > 0 || ENABLE_TEST){
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 128, 100});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 128, 100});
    }

    do_action_and_monitor_for_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

        direction.change_direction(env.program_info(), env.console, context, 1.798578);
        pbf_move_left_joystick(context, 128, 0, 200, 50);

        
    });

    // marker 1     {0.825, 0.361111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 255, 30}, 
        FlyPoint::POKECENTER, 
        {0.825, 0.361111}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2             {0.839062, 0.267593}  {0.834896, 0.267593}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.834896, 0.267593}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

 

    // marker 3             {0.764583, 0.244444}            (0.775000, 0.250000).
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.775000, 0.250000}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 4    {0.604167, 0.326852}    {0.597396, 0.32037}    {0.600521, 0.325}    {0.589583, 0.319444}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.597396, 0.32037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 5    enter bamboo forest      keep zoom{0.679688, 0.19537}   zoom out  {0.571875, 0.360185}   
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.571875, 0.360185}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6     {0.668229, 0.336111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 0, 20}, 
        FlyPoint::POKECENTER, 
        {0.668229, 0.336111}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 7      {0.496354, 0.20463}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 255, 100, 80}, 
        FlyPoint::POKECENTER, 
        {0.496354, 0.20463}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8      {0.428125, 0.483333}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 255, 0, 50}, 
        FlyPoint::POKECENTER, 
        {0.428125, 0.483333}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 30, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // speak to Clavell at Fighting base
    mash_button_till_overworld(env.console, context, BUTTON_A, 360);


    // place marker past pokecenter   {0.25625, 0.566667}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 80, 50}, 
        FlyPoint::POKECENTER, 
        {0.25625, 0.566667}
    );

    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 0, 20, 20, false); 

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


}


void beat_team_star_fighting1(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t attempt_number){

    if (attempt_number > 0 || ENABLE_TEST){
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 128, 80});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 128, 80});
    }

    // marker 1
    realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 190, 30);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 20, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 2. navigate to gate    {0.244792, 0.37037}     {0.244792, 0.359259}        {0.265625, 0.371296}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 255, 128, 0}, 
        FlyPoint::POKECENTER, 
        {0.265625, 0.371296}
    );

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 20, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
    env.console.log("Battle team star grunt.");
    run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
    mash_button_till_overworld(env.console, context, BUTTON_A);


}

void beat_team_star_fighting2(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 220, 255, 50);
            walk_forward_while_clear_front_path(env.program_info(), env.console, context, 100);
            walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
        }
    );
    clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD, CallbackEnum::PROMPT_DIALOG});


    NoMinimapWatcher no_minimap(env.console.logger(), COLOR_RED, Milliseconds(250));
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){

            DirectionDetector direction;
            uint16_t seconds_wait = 3; 

            direction.change_direction(env.program_info(), env.console, context, 3.837821);
            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 100, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 200, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.683419);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 3.577637);
            pbf_move_left_joystick(context, 128, 0, 500, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 4.204571);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);
            
            pbf_move_left_joystick(context, 128, 0, 200, 50);


            direction.change_direction(env.program_info(), env.console, context, 3.737641);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 3.368991);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 3.473130);
            pbf_move_left_joystick(context, 128, 0, 700, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 3.213118);
            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 3.787068);
            pbf_move_left_joystick(context, 128, 0, 500, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);


            direction.change_direction(env.program_info(), env.console, context,  3.997072);
            pbf_move_left_joystick(context, 128, 0, 500, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 5.200063);
            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 5.883155);
            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 6.089752);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 300, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 0.369203);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            direction.change_direction(env.program_info(), env.console, context, 5.515246);
            pbf_move_left_joystick(context, 128, 0, 200, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            pbf_move_left_joystick(context, 128, 0, 400, 50);
            pbf_press_button(context, BUTTON_R, 20, 20);
            pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            try {
                direction.change_direction(env.program_info(), env.console, context, 4.941664);
                pbf_move_left_joystick(context, 128, 0, 100, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 6.140720);
                pbf_move_left_joystick(context, 128, 0, 100, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 5.096584);
                pbf_move_left_joystick(context, 128, 0, 300, 50);


                direction.change_direction(env.program_info(), env.console, context, 5.201017);
                pbf_move_left_joystick(context, 128, 0, 1200, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

                direction.change_direction(env.program_info(), env.console, context, 0);
                pbf_move_left_joystick(context, 128, 0, 00, 50);
                pbf_press_button(context, BUTTON_R, 20, 20);
                pbf_wait(context, seconds_wait * TICKS_PER_SECOND);
            }catch (OperationFailedException&){
                env.console.log("Failed to change direction, but it's possibly due to clearing the challenge.");
                pbf_wait(context, 20000ms);
            }

            // ///////////////
            // direction.change_direction(env.program_info(), env.console, context, 0);
            // pbf_move_left_joystick(context, 128, 0, 00, 50);
            // pbf_press_button(context, BUTTON_R, 20, 20);
            // pbf_wait(context, seconds_wait * TICKS_PER_SECOND);

            

            pbf_wait(context, 20000ms);

                
            
        },
        {no_minimap}
    );
    context.wait_for(std::chrono::milliseconds(100));
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "checkpoint_48(): Failed to kill 30 pokemon with Let's go.",
            env.console
        );            
    }
    clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::DIALOG_ARROW});
    env.console.log("Battle the Team Star (Fighting) boss.");
    run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
    mash_button_till_overworld(env.console, context, BUTTON_A, 360);


}

// NOTE: This function isn't used
void move_from_fighting_base_to_north_province_area_two(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    // marker 1. set marker to pokecenter
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
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 2. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 60, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}



void move_from_west_province_area_one_north_to_alfornada(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    DirectionDetector direction;
    // recently flew here, so Minimap should be clear of Pokemon. and we should have no issues detecting direction.
    // from Team Star Fighting to West Province Area One Pokecenter

    do_action_and_monitor_for_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        // marker 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 140, 255, 30);
        direction.change_direction(env.program_info(), env.console, context, 4.047990);
        pbf_move_left_joystick(context, 128, 0, 200, 50);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
    });

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 2      zoom in{0.605729, 0.30463}, zoom out{0.684375, 0.616667}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 150, 15}, 
        FlyPoint::POKECENTER, 
        {0.684375, 0.616667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 24, 8, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 3  {0.767708, 0.45}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 255, 60}, 
        FlyPoint::POKECENTER, 
        {0.767708, 0.45}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 60, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 4   {0.763021, 0.253704}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.763021, 0.253704}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 5  {0.780729, 0.216667}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.780729, 0.216667}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6. place the marker at the top of the cliff. {0.633333, 0.304630}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, 0, 0}, 
        FlyPoint::POKECENTER, 
        {0.633333, 0.304630}
    );
    
    do_action_and_monitor_for_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        // walk towards wall
        direction.change_direction(env.program_info(), env.console, context, 2.949863);

        walk_forward_while_clear_front_path(env.program_info(), env.console, context, 700);

        // back away from wall and get on ride
        pbf_move_left_joystick(context, 128, 255, 50, 50);
        get_on_ride(env.program_info(), env.console, context);

        // move back towards wall
        pbf_move_left_joystick(context, 128, 0, 100, 50);
        pbf_move_left_joystick(context, 0, 0, 400, 50);

        direction.change_direction(env.program_info(), env.console, context, 2.575); //2.566167

        // climb the wall
        pbf_press_button(context, BUTTON_B, 528ms, 0ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 51ms);
        pbf_move_left_joystick(context, 128, 0, 5002ms, 0ms);
        pbf_wait(context, 1551ms);
        pbf_move_left_joystick(context, 0, 128, 2167ms, 0ms);
        pbf_wait(context, 745ms);
        
    });

    // continue climbing wall
    pbf_move_left_joystick(context, 128, 0, 8804ms, 0ms);

    get_off_ride(env.program_info(), env.console, context);

    // realign to marker 6
    realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 7 {0.501042, 0.738889}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, 200, 30}, 
        FlyPoint::POKECENTER, 
        {0.501042, 0.738889}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 128, 255, 30},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0}
    ); 

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 9. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 120, 255, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


    

}




}
}
}
