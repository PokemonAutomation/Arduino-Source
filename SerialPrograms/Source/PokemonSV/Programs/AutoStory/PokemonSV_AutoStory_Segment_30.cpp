/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_30.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_30::name() const{
    return "30: Glaseado Gym (Ice)";
}

std::string AutoStory_Segment_30::start_text() const{
    return "Start: Beat Montenevera Gym (Ghost). At Glaseado gym Pokecenter.";
}

std::string AutoStory_Segment_30::end_text() const{
    return "End: Beat Glaseado Gym (Ice). At North Province Area One Pokecenter.";
}

void AutoStory_Segment_30::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_75().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_76().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_77().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


std::string AutoStory_Checkpoint_75::name() const{ return "075 - " + AutoStory_Segment_30().name(); }
std::string AutoStory_Checkpoint_75::start_text() const{ return "At Glaseado gym Pokecenter.";}
std::string AutoStory_Checkpoint_75::end_text() const{ return "Battled Nemona. Spoke to Glaseado Gym receptionist";}
void AutoStory_Checkpoint_75::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_75(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_76::name() const{ return "076 - " + AutoStory_Segment_30().name(); }
std::string AutoStory_Checkpoint_76::start_text() const{ return AutoStory_Checkpoint_75().end_text();}
std::string AutoStory_Checkpoint_76::end_text() const{ return "Beat Glaseado Gym Challenge. Beat Glaseado Gym (Ice).";}
void AutoStory_Checkpoint_76::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_76(env, context, options.notif_status_update, stats);
}


std::string AutoStory_Checkpoint_77::name() const{ return "077 - " + AutoStory_Segment_30().name(); }
std::string AutoStory_Checkpoint_77::start_text() const{ return AutoStory_Checkpoint_76().end_text();}
std::string AutoStory_Checkpoint_77::end_text() const{ return "At North Province Area One Pokecenter";}
void AutoStory_Checkpoint_77::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_77(env, context, options.notif_status_update, stats);
}



void checkpoint_75(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        DirectionDetector direction;
        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            // fly_to_overworld_from_map() may fail since the snowy background on the map will false positive the destinationMenuItemWatcher (MapDestinationMenuDetector at box {0.523000, 0.680000, 0.080000, 0.010000}), which causes the fly to fail
            // we can get around this by either placing down a marker, or by zooming out so that that section isn't white snow.
            // place down marker in this case

            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 210, 255, 40);
            // place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            //     {ZoomChange::ZOOM_IN, 255, 200, 300}, 
            //     FlyPoint::POKECENTER, 
            //     {0.404687, 0.261111}
            // );
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 128, 0, 240ms});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 200, 255, 880ms});
        }
        
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

            direction.change_direction(env.program_info(), env.console, context, 3.855289);
            pbf_move_left_joystick(context, 128, 0, 200, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.056395);
            pbf_move_left_joystick(context, 128, 0, 250, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.749788);
            pbf_move_left_joystick(context, 128, 0, 680, 50);

            direction.change_direction(env.program_info(), env.console, context, 1.589021);
            pbf_move_left_joystick(context, 128, 0, 600, 50);
            
            direction.change_direction(env.program_info(), env.console, context, 1.343606);   //1.327724

            handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
                [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                    walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 30000ms);
                }, 
                [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                    pbf_move_left_joystick(context, 0, 0, 150, 50); // move left
                    pbf_move_left_joystick(context, 255, 0, 150, 50);  // move right
                }
            );
        });

        // speak to Nemona, inside the gym
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});

        env.console.log("Battle Nemona.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // speak to gym receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 20000ms);
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});



    });   
}

void checkpoint_76(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        pbf_move_left_joystick(context, 128, 255, 300, 100);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);


        env.console.log("Fly back to Glaseado Gym Pokecenter");
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap. End up in Glaseado Gym Pokecenter.");
        // fly_to_overworld_from_map() may fail since the snowy background on the map will false positive the destinationMenuItemWatcher (MapDestinationMenuDetector at box {0.523000, 0.680000, 0.080000, 0.010000}), which causes the fly to fail
        // we can get around this by either placing down a marker, or by zooming out so that that section isn't white snow.
        // place down a marker in this case.

        // remove old marker, then place new one
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 0, 0);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 180, 45);

        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 128, 0, 240ms});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 200, 255, 880ms});
        
        DirectionDetector direction;
        
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

            direction.change_direction(env.program_info(), env.console, context, 0.943392);
            pbf_move_left_joystick(context, 128, 0, 200, 50);

            direction.change_direction(env.program_info(), env.console, context, 2.513925);

            // talk to 
            walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30000ms);

        });

        mash_button_till_overworld(env.console, context, BUTTON_A);

        // start the Snow Slope Run
        NoMinimapWatcher no_minimap(env.console, COLOR_RED, Milliseconds(5000));
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                pbf_move_left_joystick(context, 128, 0, 2412ms, 0ms);
                pbf_wait(context, 3612ms);
                pbf_move_left_joystick(context, 128, 0, 16ms, 0ms);
                pbf_move_left_joystick(context, 0, 0, 2861ms, 0ms);
                pbf_move_left_joystick(context, 128, 0, 309ms, 0ms);
                pbf_wait(context, 6431ms);
                pbf_move_left_joystick(context, 0, 128, 5081ms, 0ms);
                pbf_wait(context, 239ms);
                pbf_move_left_joystick(context, 128, 255, 961ms, 0ms);
                pbf_wait(context, 1615ms);
                pbf_move_left_joystick(context, 128, 0, 753ms, 0ms);
                pbf_wait(context, 79ms);
                pbf_move_left_joystick(context, 128, 0, 75ms, 0ms);
                pbf_wait(context, 10485ms);
                pbf_move_left_joystick(context, 128, 0, 7930ms, 0ms);
                pbf_wait(context, 12000ms);
            },
            {no_minimap}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "checkpoint_76(): Failed to Snow Slope Run.",
                env.console
            );
        }
        env.log("No minimap seen. Likely finished the Snow Slope Run. But no guarantee we did it within time.");

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        // pokemon are cleared from minimap after the Snow Slope run

        // fly back to Glaseado Gym Pokecenter
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 0, 0, 0ms}, FlyPoint::POKECENTER);


        // walk back to gym building
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

            direction.change_direction(env.program_info(), env.console, context, 3.855289);
            pbf_move_left_joystick(context, 128, 0, 200, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.056395);
            pbf_move_left_joystick(context, 128, 0, 250, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.749788);
            pbf_move_left_joystick(context, 128, 0, 680, 50);

            direction.change_direction(env.program_info(), env.console, context, 1.589021);
            pbf_move_left_joystick(context, 128, 0, 600, 50);
            
            direction.change_direction(env.program_info(), env.console, context, 1.343606);   //1.327724

            handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
                [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                    walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30000ms);
                }, 
                [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){           
                    pbf_move_left_joystick(context, 0, 0, 150, 50); // move left
                    pbf_move_left_joystick(context, 255, 0, 150, 50);  // move right
                }
            );
        });

        // speak to receptionist. if we fail to detect a battle, then we know we failed the Gym test. we then reset.
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::PROMPT_DIALOG, CallbackEnum::BATTLE, CallbackEnum:: DIALOG_ARROW});
        env.console.log("Battle Ice Gym leader.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A);

    });   
}

void checkpoint_77(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        pbf_move_left_joystick(context, 128, 255, 300, 100);
        pbf_wait(context, 3000ms);
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        env.console.log("Go to Glaseado Gym Pokecenter");
        env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
        // fly_to_overworld_from_map() may fail since the snowy background on the map will false positive the destinationMenuItemWatcher (MapDestinationMenuDetector at box {0.523000, 0.680000, 0.080000, 0.010000}), which causes the fly to fail
        // we can get around this by either placing down a marker, or by zooming out so that that section isn't white snow.
        // place down a marker in this case.

        // remove old marker, then place new one
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 0, 0);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 180, 45);

        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_OUT, 128, 0, 240ms});
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::ZOOM_IN, 200, 255, 880ms});

        DirectionDetector direction;

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){

            direction.change_direction(env.program_info(), env.console, context, 3.855289);
            pbf_move_left_joystick(context, 128, 0, 200, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.056395);
            pbf_move_left_joystick(context, 128, 0, 250, 50);

            direction.change_direction(env.program_info(), env.console, context, 3.749788);
            pbf_move_left_joystick(context, 128, 0, 680, 50);

            direction.change_direction(env.program_info(), env.console, context, 1.589021);
            pbf_move_left_joystick(context, 128, 0, 1200, 50);
            
            // direction.change_direction(env.program_info(), env.console, context, 1.343606);   //1.327724
           
        });

        move_from_glaseado_gym_to_north_province_area_one(env, context);

    });   
}




void move_from_glaseado_gym_to_north_province_area_one(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    // marker 0      {0.460938, 0.310185}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.460938, 0.310185}
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


    // marker 1     {0.33125, 0.314815}      {0.297396, 0.322222}        {0.313021, 0.322222}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.313021, 0.322222}
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

    // marker 2         {0.222396, 0.413889}         {0.253646, 0.386111}
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::ZOOM_IN, 0, 0, 0ms}, 
        FlyPoint::POKECENTER, 
        {0.253646, 0.386111}
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
        
 

    // marker 3   {0.80625, 0.574074}        {0.803125, 0.615741}       {0.794792, 0.614815}        (0.794792, 0.609259).
    place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 0, 400ms},
        FlyPoint::POKECENTER, 
        {0.794792, 0.609259}
    );
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 50, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    );

    // marker 4. set marker to pokecenter
    realign_player_from_landmark(
        env.program_info(), env.console, context, 
        {ZoomChange::KEEP_ZOOM, 255, 128, 640ms},
        {ZoomChange::KEEP_ZOOM, 0, 0, 0ms}
    );  
    handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 110, 10, false);
        }, 
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            pbf_move_left_joystick(context, 0, 255, 40, 50);
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        }
    ); 

    // marker 5. set marker past pokecenter
    handle_unexpected_battles(env.program_info(), env.console, context,
    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 255, 50);
    });      
    overworld_navigation(env.program_info(), env.console, context, 
        NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
        128, 15, 12, 12, false);           // can't wrap in handle_when_stationary_in_overworld(), since we expect to be stationary when walking into the pokecenter
        

    fly_to_overlapping_flypoint(env.program_info(), env.console, context); 


}



}
}
}
