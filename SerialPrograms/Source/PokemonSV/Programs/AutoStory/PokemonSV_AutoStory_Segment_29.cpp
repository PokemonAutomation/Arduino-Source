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
#include "PokemonSV_AutoStory_Segment_29.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_29::name() const{
    return "29: Montenevera Gym (Ghost)";
}

std::string AutoStory_Segment_29::start_text() const{
    return "Start: Beat Team Star (Fairy). At Montenevera Pokecenter.";
}

std::string AutoStory_Segment_29::end_text() const{
    return "End: Beat Montenevera Gym (Ghost). At Glaseado gym Pokecenter.";
}

void AutoStory_Segment_29::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_71().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_72().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_73().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_74().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_71::name() const{ return "071 - " + AutoStory_Segment_29().name(); }
std::string AutoStory_Checkpoint_71::start_text() const{ return "At Montenevera Pokecenter";}
std::string AutoStory_Checkpoint_71::end_text() const{ return "Spoke to Montenevera gym receptionist";}
void AutoStory_Checkpoint_71::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_71(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_72::name() const{ return "072 - " + AutoStory_Segment_29().name(); }
std::string AutoStory_Checkpoint_72::start_text() const{ return AutoStory_Checkpoint_71().end_text();}
std::string AutoStory_Checkpoint_72::end_text() const{ return "Passed gym test with MC Sledge.";}
void AutoStory_Checkpoint_72::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_72(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_73::name() const{ return "073 - " + AutoStory_Segment_29().name(); }
std::string AutoStory_Checkpoint_73::start_text() const{ return AutoStory_Checkpoint_72().end_text();}
std::string AutoStory_Checkpoint_73::end_text() const{ return "Beat Montenevera Gym. Inside gym building.";}
void AutoStory_Checkpoint_73::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_73(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_74::name() const{ return "074 - " + AutoStory_Segment_29().name(); }
std::string AutoStory_Checkpoint_74::start_text() const{ return AutoStory_Checkpoint_73().end_text();}
std::string AutoStory_Checkpoint_74::end_text() const{ return "At Glaseado gym Pokecenter.";}
void AutoStory_Checkpoint_74::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_74(env, context, options.notif_status_update, stats);
}



void checkpoint_71(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        DirectionDetector direction;

        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
        // fly_to_overworld_from_map() may fail since the snowy background on the map will false positive the destinationMenuItemWatcher (MapDestinationMenuDetector at box {0.523000, 0.680000, 0.080000, 0.010000}), which causes the fly to fail
        // we can get around this by either placing down a marker, or by zooming out so that that section isn't white snow.
        place_marker_offset_from_flypoint(env.program_info(), env.console, context,
            {ZoomChange::KEEP_ZOOM, 0, -1, 280ms},
            FlyPoint::POKECENTER, 
            {0.54375, 0.662037}
        );

        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, 0, 600ms});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, +1, 0, 600ms});


        direction.change_direction(env.program_info(), env.console, context, 1.536225);
        pbf_move_left_joystick(context, {0, +1}, 4000ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 3.786414);
        pbf_move_left_joystick(context, {0, +1}, 8800ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 4.747153);
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);


        direction.change_direction(env.program_info(), env.console, context, 4.222303);
        pbf_move_left_joystick(context, {0, +1}, 4800ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 4.572071);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30000ms);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, +1}, 800ms, 400ms); // if to the left of the door, will move right and enter
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 400ms); // if to the right of the door, will move left
                pbf_move_left_joystick(context, {+1, +1}, 800ms, 400ms); // then move right and enter
            }
        );
        
        // speak to Jacq, inside the gym
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // speak to gym receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});



    });  
}

void checkpoint_72(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        pbf_move_left_joystick(context, {0, -1}, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        DirectionDetector direction;
        // Minimap should be clear of Pokemon, since we haven't opened the map, since we cleared the Pokemon in checkpoint 71
        direction.change_direction(env.program_info(), env.console, context, 2.462858);  // 2.496149  // 2.479418
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20000ms);

        // speak to MC Sledge
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});

        env.console.log("Battle trainer 1 in the Gym challenge.");
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});
        
        env.console.log("Battle trainer 2 in the Gym challenge.");
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});

        env.console.log("Battle MC Sledge in the Gym challenge.");
        run_trainer_double_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::OVERWORLD});

    });  
}

void checkpoint_73(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        DirectionDetector direction;
        // Minimap should be clear of Pokemon, since we haven't opened the map, since we cleared the Pokemon in checkpoint 71
        direction.change_direction(env.program_info(), env.console, context, 5.478851);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30000ms);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                pbf_move_left_joystick(context, {+1, 0}, 1600ms, 400ms); // if to the left of the door, will move right
                pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
            }
        );

        mash_button_till_overworld(env.console, context, BUTTON_A, 360); // fight the Ghost Gym

    });  
}

void checkpoint_74(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){
        pbf_move_left_joystick(context, {0, -1}, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        // fly_to_overworld_from_map() may fail since the snowy background on the map will false positive the destinationMenuItemWatcher (MapDestinationMenuDetector at box {0.523000, 0.680000, 0.080000, 0.010000}), which causes the fly to fail
        // we can get around this by either placing down a marker, or by zooming out so that that section isn't white snow. 
        // We place a marker in this case
        env.console.log("fly somewhere else (Glaseado Mountain), then back to Montenevera Pokecenter");
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");

        // remove old marker, then place new one
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, -1, 0, 400ms);
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
                {ZoomChange::KEEP_ZOOM, 0, -1, 280ms},
                FlyPoint::POKECENTER, 
                {0.54375, 0.662037}
            );

        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, 0, 600ms});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, +1, 0, 600ms});
        

        move_from_montenevera_to_glaseado_gym(env, context);


    });  
}



void move_from_montenevera_to_glaseado_gym(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DirectionDetector direction;
    // minimap was cleared at the beginning of this checkpoint
    direction.change_direction(env.program_info(), env.console, context, 1.255489);
    pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);

    direction.change_direction(env.program_info(), env.console, context, 2.463760);
    pbf_move_left_joystick(context, {0, +1}, 8800ms, 400ms);

     
    // marker 1 {0.585938, 0.236111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, -1, 160ms},
        FlyPoint::POKECENTER, 
        {0.585938, 0.236111}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 2 cross bridge   {0.719271, 0.585185}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, -1, 240ms},
        FlyPoint::POKECENTER, 
        {0.719271, 0.585185}
    );

    get_on_ride(env.program_info(), env.console, context);

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

    // marker 3           {0.73125, 0.481481}    {0.7375, 0.488889}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, -1, 240ms},
        FlyPoint::POKECENTER, 
        {0.7375, 0.488889}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){ // todo: get on ride?
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 4     {0.691146, 0.347222}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.691146, 0.347222}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 5      {0.632292, 0.376852}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.632292, 0.376852}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 6     {0.61875, 0.432407}       {0.610417, 0.42037}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.610417, 0.42037}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 7          {0.613542, 0.540741}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.613542, 0.540741}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8          {0.588021, 0.578704}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.588021, 0.578704}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 9         {0.579167, 0.610185}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, -1, +1, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.579167, 0.610185}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 24, 8, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 10. end up next to Pokecenter        {0.520833, 0.443519}     {0.490625, 0.4}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, -1, +1, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.490625, 0.4}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                0, +1, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 11. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, +0.646, +1, 400ms);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        0, +0.883, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 



}





}
}
}
