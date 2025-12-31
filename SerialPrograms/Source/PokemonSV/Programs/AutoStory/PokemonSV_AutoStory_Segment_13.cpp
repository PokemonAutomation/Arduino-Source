/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_13.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_13::name() const{
    return "13: Bombirdier Titan: Go to West Province Area One Central Pokecenter";
}

std::string AutoStory_Segment_13::start_text() const{
    return "Start: At Cortondo West Pokecenter";
}

std::string AutoStory_Segment_13::end_text() const{
    return "End: At West Province Area One Central Pokecenter";
}

void AutoStory_Segment_13::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_29().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_29::name() const{ return "029 - " + AutoStory_Segment_13().name(); }
std::string AutoStory_Checkpoint_29::start_text() const{ return "At Cortondo West Pokecenter.";}
std::string AutoStory_Checkpoint_29::end_text() const{ return "At West Province Area One Central Pokecenter.";}
void AutoStory_Checkpoint_29::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_29(env, context, options.notif_status_update, stats);
}

void checkpoint_29(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();

        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, +1, 0ms});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, +1, 0ms});
        }

        // align for long stretch 1, part 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, -0.453, +1, 480ms);


        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 15, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );            

        // align for long stretch 1, part 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, -1, 320ms},
            {ZoomChange::KEEP_ZOOM, -0.375, +1, 600ms}
        );        
        
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 24, 12, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );         

        // align for long stretch 1, part 3
 
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, -1, 480ms},
            {ZoomChange::KEEP_ZOOM, -0.258, +1, 920ms}
        );
        
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 36, 12, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );  

        // align for long stretch 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, -1, 800ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.18, 520ms}
        );              

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 45, 15, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );  

        // align for long stretch 3, part 1
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, 0, 520ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.609, 696ms}
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

        // align for long stretch 3, part 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -0.252, 520ms},
            {ZoomChange::KEEP_ZOOM, -0.844, +1, 880ms}
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

        // align for long stretch 3, part 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, +0.531, 880ms},
            {ZoomChange::KEEP_ZOOM, +1, 0, 920ms}
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

        // // align for long stretch 3, part 4
        // realign_player_from_landmark(
        //     env.program_info(), env.console, context, 
        //     {ZoomChange::ZOOM_IN, 0, 128, 100},
        //     {ZoomChange::KEEP_ZOOM, 255, 67, 85} //{ZoomChange::KEEP_ZOOM, 255, 70, 90}
        // );

        //align for long stretch 3, part 4.  just prior to bridge. {0.339062, 0.612037}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms},
            FlyPoint::POKECENTER, 
            {0.339062, 0.612037}
        );


        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 36, 12, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, -1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );        

        // // align to cross bridge
        // realign_player_from_landmark(
        //     env.program_info(), env.console, context, 
        //     {ZoomChange::ZOOM_IN, 0, 128, 90},
        //     {ZoomChange::KEEP_ZOOM, 255, 35, 67}
        // );

        // align to cross bridge   {0.385937, 0.615741}
        place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms},
            FlyPoint::POKECENTER, 
            {0.385937, 0.615741}
        );


        // attempt to cross bridge. If fall into water, go back to start position (just before bridge) and try again
        WallClock start_to_cross_bridge = current_time();
        while (true){
            if (current_time() - start_to_cross_bridge > std::chrono::minutes(6)){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "checkpoint_26(): Failed to cross bridge after 6 minutes.",
                    env.console
                );
            }        

            try {
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
                    128, 0, 20, 20, false);         

                break;

            }catch(OperationFailedException&){ // try again if fall into water
                pbf_mash_button(context, BUTTON_A, 2000ms);

                // // walk back to start position before bridge
                // realign_player_from_landmark(
                //     env.program_info(), env.console, context, 
                //     {ZoomChange::ZOOM_IN, 255, 255, 180},
                //     {ZoomChange::KEEP_ZOOM, 33, 0, 175}
                // );

                // walk back to start position before bridge
                //  {0.310937, 0.580556}      {0.310937, 0.589815}      {0.310937, 0.584259}
                place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
                    {ZoomChange::KEEP_ZOOM, -1, +1, 0ms},
                    FlyPoint::POKECENTER, 
                    {0.310937, 0.584259}
                );

                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
                    128, 0, 20, 20, false);          


                // // align to cross bridge
                // realign_player_from_landmark(
                //     env.program_info(), env.console, context, 
                //     {ZoomChange::ZOOM_IN, 0, 128, 90},
                //     {ZoomChange::KEEP_ZOOM, 255, 35, 67}
                // );

                // align to cross bridge   {0.385937, 0.615741}
                place_marker_offset_from_flypoint(env.program_info(), env.console, context, 
                    {ZoomChange::KEEP_ZOOM, -1, +1, 0ms},
                    FlyPoint::POKECENTER, 
                    {0.385937, 0.615741}
                );


            }
        }

        confirm_no_overlapping_flypoint(env.program_info(), env.console, context);
        pbf_press_button(context, BUTTON_B, 160ms, 800ms);
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
        });

        env.console.log("Successfully crossed the bridge.");
        
        // align for post-bridge section 1
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, -0.173, 480ms},
            {ZoomChange::KEEP_ZOOM, +1, +0.531, 400ms} // {ZoomChange::KEEP_ZOOM, 255, 60, 50}
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
 

        // align for post-bridge section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, -0.173, 480ms},
            {ZoomChange::KEEP_ZOOM, +1, +0.18, 400ms}
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

        // align for post-bridge section 3. move up towards tree
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, 0, 400ms},
            {ZoomChange::KEEP_ZOOM, +1, +0.297, 280ms}
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

        // align for post-bridge section 4
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, 0, 400ms},
            {ZoomChange::KEEP_ZOOM, +1, +0.57, 200ms}
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



        // align for post-bridge section 5. set marker to pokecenter.
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, 0, 400ms},
            {ZoomChange::KEEP_ZOOM, 0, 0, 0ms}
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


        // align for post-bridge section 6. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, -1, -0.567, 240ms);
        });
                // realign_player_from_landmark(
                //     env.program_info(), env.console, context, 
                //     {ZoomChange::ZOOM_IN, 128, 128, 0},
                //     {ZoomChange::KEEP_ZOOM, 0, 180, 20}
                // );   

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);    



        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
              
       
    });

}





}
}
}
