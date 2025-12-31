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
#include "PokemonSV_AutoStory_Segment_26.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_26::name() const{
    return "26: Medali Gym (Normal)";
}

std::string AutoStory_Segment_26::start_text() const{
    return "Start: Beat Team Star (Poison). At Medali Pokecenter.";
}

std::string AutoStory_Segment_26::end_text() const{
    return "End: Beat Medali Gym (Normal). At Glaseado Mountain Pokecenter.";
}

void AutoStory_Segment_26::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_61().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_62().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_63().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_61::name() const{ return "061 - " + AutoStory_Segment_26().name(); }
std::string AutoStory_Checkpoint_61::start_text() const{ return "At Medali Pokecenter.";}
std::string AutoStory_Checkpoint_61::end_text() const{ return "At Medali Gym";}
void AutoStory_Checkpoint_61::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_61(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_62::name() const{ return "062 - " + AutoStory_Segment_26().name(); }
std::string AutoStory_Checkpoint_62::start_text() const{ return AutoStory_Checkpoint_61().end_text();}
std::string AutoStory_Checkpoint_62::end_text() const{ return "Beat Medali Gym";}
void AutoStory_Checkpoint_62::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_62(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_63::name() const{ return "063 - " + AutoStory_Segment_26().name(); }
std::string AutoStory_Checkpoint_63::start_text() const{ return AutoStory_Checkpoint_62().end_text();}
std::string AutoStory_Checkpoint_63::end_text() const{ return "At Glaseado Mountain Pokecenter";}
void AutoStory_Checkpoint_63::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_63(env, context, options.notif_status_update, stats);
}




void checkpoint_61(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats, 
    [&](size_t attempt_number){

        // first, clear Pokemon in Minimap.
        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, +1, 0ms});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, +1, 0ms});
        }

        // marker 1   keep{0.490625, 0.594444}  in{0.589583, 0.569444} 
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, +1, 0ms},
            FlyPoint::POKECENTER, 
            {0.589583, 0.569444}
        );
        
        DirectionDetector direction;

        direction.change_direction(env.program_info(), env.console, context, 0.278620);
        pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 60, 20, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        direction.change_direction(env.program_info(), env.console, context, 3.566068);

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20000ms);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, +1}, 800ms, 400ms); // if to the left of the door, will move right and enter
                pbf_move_left_joystick(context, {-1, 0}, 2400ms, 400ms); // if to the right of the door, will move left
                pbf_move_left_joystick(context, {+1, +1}, 800ms, 400ms); // then move right and enter
            }
        );

        // speak to Nemona, inside the gym
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // speak to gym receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

    });    

}

void checkpoint_62(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        pbf_move_left_joystick(context, {0, -1}, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);


        DirectionDetector direction;
        env.console.log("Fly back to Medali East Pokecenter");
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
        env.console.log("Fly to Cascaraffa north to clear minimap. Then Medali West. End up in Medal East Pokecenter.");
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, -0.409, 800ms}, FlyPoint::POKECENTER);
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, +1, +0.375, 680ms});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, -1, +1, 0ms});


        pbf_press_button(context, BUTTON_L, 400ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 1.971173);
        pbf_move_left_joystick(context, {0, +1}, 4800ms, 400ms);

        direction.change_direction(env.program_info(), env.console, context, 3.191172);
        pbf_move_left_joystick(context, {0, +1}, 2400ms, 400ms);

         
        direction.change_direction(env.program_info(), env.console, context, 5.114177);  // old 4.975295
        pbf_move_left_joystick(context, {0, +1}, 2400ms, 400ms);

        pbf_wait(context, 3000ms);
        // wait for overworld after entering Eatery
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_left_joystick(context, {+1, 0}, 400ms, 400ms);
        
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});

        // grilled rice balls
        pbf_press_button(context, BUTTON_A, 400ms, 400ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        // medium serving
        pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
        pbf_press_button(context, BUTTON_A, 400ms, 400ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        // extra crispy
        pbf_press_dpad(context, DPAD_UP, 104ms, 160ms);
        pbf_press_button(context, BUTTON_A, 400ms, 400ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_PROMPT, 60, {CallbackEnum::PROMPT_DIALOG});
        // lemon
        pbf_press_dpad(context, DPAD_DOWN, 104ms, 160ms);
        pbf_press_button(context, BUTTON_A, 400ms, 400ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD}, false);


        pbf_mash_button(context, BUTTON_A, 1000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});

        env.console.log("Battle Normal Gym leader.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        // We see Dialog right before the final pokemon terastalizes, but after you select your move. So, technically, we see dialog before the battle officially ends.
        // it's fine as long as you OHKO Larry's final Pokemon. If you can't OHKO the final Staraptor, they you probably won't be able to beat the Elite Four.
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 60);
        pbf_mash_button(context, BUTTON_A, 1000ms);

        env.console.log("Finish up with Larry, then speak to Geeta and Nemona.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::WHITE_A_BUTTON, CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});
        env.console.log("Battle Nemona.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);
        

    });    

}

void checkpoint_63(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        // Gym leader defeated. Standing in Gym building
        pbf_move_left_joystick(context, {0, -1}, 2400ms, 800ms);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving Gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        // fly to Medali East Pokecenter
        env.console.log("Fly back to Medali East Pokecenter");
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
        env.console.log("Fly to Cascaraffa north to clear minimap. Then Medali West. End up in Medal East Pokecenter.");
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, -0.409, 800ms}, FlyPoint::POKECENTER);
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, +1, +0.375, 680ms});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, -1, +1, 0ms});

        
        move_from_medali_to_glaseado_mountain(env, context);

    });    

}


void move_from_medali_to_glaseado_mountain(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();


    // marker 1.    x=0.399479, y=0.713889
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.399479, 0.713889}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );
    

    // marker 2.     x=0.410417, y=0.760185
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, +1, +1, 240ms},
        FlyPoint::POKECENTER, 
        {0.410417, 0.760185}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 3.     : x=0.3875, y=0.712037
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.3875, 0.712037}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 4. cross creek     x=0.502083, y=0.255556
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, 0, +1, 160ms},
        FlyPoint::POKECENTER, 
        {0.502083, 0.255556}
    );        

    get_on_ride(env.program_info(), env.console, context);

    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 20, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {0, +1}, 500ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 1000ms);
            pbf_move_left_joystick(context, {0, +1}, 500ms, 0ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    get_off_ride(env.program_info(), env.console, context);

    

    // marker 5.     x=0.461458, y=0.297222
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_OUT, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.461458, 0.297222}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 6.     x=0.451562, y=0.288889
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
        FlyPoint::POKECENTER, 
        {0.451562, 0.288889}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 7.    : x=0.623958, y=0.35463
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, -1, +1, 400ms},
        FlyPoint::POKECENTER, 
        {0.623958, 0.35463}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 30, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 8.     x=0.544271, y=0.5
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.544271, 0.5}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {-1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 9.    : x=0.417708, y=0.388889
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, -1, +1, 0ms},
        FlyPoint::POKECENTER, 
        {0.417708, 0.388889}
    );        
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );


    // marker 10. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, -1, +1, 0ms},
        {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}
    );  
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 20, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 11. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 20, 0, 40);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 

}




}
}
}
