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

    checkpoint_29(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

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

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);

        // align for long stretch 1, part 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 70, 0, 60);


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 15, false);

        // align for long stretch 1, part 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 128, 255, 40},
            {ZoomChange::KEEP_ZOOM, 80, 0, 75}
        );        
        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 12, 12, false);            

        // align for long stretch 1, part 3
 
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 128, 255, 60},
            {ZoomChange::KEEP_ZOOM, 95, 0, 115}
        );
        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 36, 12, false);

        // align for long stretch 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 128, 255, 100},
            {ZoomChange::KEEP_ZOOM, 0, 105, 65}
        );              

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 45, 15, false);

        // align for long stretch 3, part 1
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 128, 65},
            {ZoomChange::KEEP_ZOOM, 0, 50, 87}
        ); 

        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        // align for long stretch 3, part 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 160, 65},
            {ZoomChange::KEEP_ZOOM, 20, 0, 110}
        ); 

        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 10, false);            

        // align for long stretch 3, part 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 60, 110},
            {ZoomChange::KEEP_ZOOM, 255, 128, 115}
        );


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 10, false);                

        // align for long stretch 3, part 4
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 128, 100},
            {ZoomChange::KEEP_ZOOM, 255, 67, 85} //{ZoomChange::KEEP_ZOOM, 255, 70, 90}
        );


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 36, 12, false);        

        // align to cross bridge
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 128, 90},
            {ZoomChange::KEEP_ZOOM, 255, 35, 67}
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
                pbf_mash_button(context, BUTTON_A, 250);

                // walk back to start position before bridge
                realign_player_from_landmark(
                    env.program_info(), env.console, context, 
                    {ZoomChange::ZOOM_IN, 255, 255, 180},
                    {ZoomChange::KEEP_ZOOM, 33, 0, 175}
                );

                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::CLEAR_WITH_LETS_GO, 
                    128, 0, 20, 20, false);          


                // align to cross bridge
                realign_player_from_landmark(
                    env.program_info(), env.console, context, 
                    {ZoomChange::ZOOM_IN, 0, 128, 90},
                    {ZoomChange::KEEP_ZOOM, 255, 35, 67}
                );


            }
        }

        confirm_no_overlapping_flypoint(env.program_info(), env.console, context);
        pbf_press_button(context, BUTTON_B, 20, 100);
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
        });

        env.console.log("Successfully crossed the bridge.");
        
        // align for post-bridge section 1
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 150, 60},
            {ZoomChange::KEEP_ZOOM, 255, 60, 50} // {ZoomChange::KEEP_ZOOM, 255, 60, 50}
        );


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);   

        // align for post-bridge section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 150, 60},
            {ZoomChange::KEEP_ZOOM, 255, 105, 50}
        );


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);        

        // align for post-bridge section 3. move up towards tree
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 128, 50},
            {ZoomChange::KEEP_ZOOM, 255, 90, 35}
        );


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);        

        // align for post-bridge section 4
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 128, 50},
            {ZoomChange::KEEP_ZOOM, 255, 55, 25}
        );


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);     



        // align for post-bridge section 5. set marker to pokecenter.
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 128, 50},
            {ZoomChange::KEEP_ZOOM, 128, 128, 0}
        );


        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 10, false);    


        // align for post-bridge section 6. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 200, 30);
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
