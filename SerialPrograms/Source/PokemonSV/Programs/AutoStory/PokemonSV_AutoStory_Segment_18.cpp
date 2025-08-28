/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_18.h"

// #include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_18::name() const{
    return "18: Great Tusk/Iron Treads titan";
}

std::string AutoStory_Segment_18::start_text() const{
    return "Start: Defeated Cascarrafa Gym (Water). At Porto Marinada Pokecenter.";
}

std::string AutoStory_Segment_18::end_text() const{
    return "End: Defeated Great Tusk/Iron Treads. At South Province (Area Three) Pokecenter.";
}

void AutoStory_Segment_18::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_39(env, context, options.notif_status_update, stats);
    checkpoint_40(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_39(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();

        DirectionDetector direction;
        
        // section 1
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                direction.change_direction(env.program_info(), env.console, context, 4.677921);
                pbf_move_left_joystick(context, 128, 0, 150, 100);
        });        

        // section 2
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 230, 120);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 60, 10, false);         

        // section 3. enter circle
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 0, 50},
            {ZoomChange::ZOOM_IN, 175, 255, 120}
        );        

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 60, 10, false);         

        DialogBoxWatcher        dialog(COLOR_RED, true);

        // section 4
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                // run around in circles until you run into the titan
                WallClock start = current_time();

                while (true){
                    if (current_time() - start > std::chrono::minutes(30)){
                        break;
                    }
                    // std::cout << "1:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
                        {ZoomChange::ZOOM_IN, 160, 255, 90}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 36, 12, false);  


                    // std::cout << "11:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
                        {ZoomChange::ZOOM_IN, 135, 255, 90}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 30, 10, false);       


                    // std::cout << "10:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
                        {ZoomChange::ZOOM_IN, 120, 255, 105}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 30, 10, false);            

                    // std::cout << "7:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
                        {ZoomChange::ZOOM_IN, 115, 255, 127}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 48, 12, false);            

                    // std::cout << "6:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
                        {ZoomChange::ZOOM_IN, 135, 255, 137}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 30, 10, false);    

                    // std::cout << "2:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 128, 0, 50},
                        {ZoomChange::ZOOM_IN, 200, 255, 120}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 50, 10, false);    
                }                 

            },
            {dialog}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "checkpoint_39(): Failed to run into Great Tusk/Iron Treads.",
                env.console
            );            
        }

        // battle the titan phase 1
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::BLACK_DIALOG_BOX});
        env.console.log("Battle Great Tusk/Iron Treads Titan phase 1.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);

        // section 5
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 128, 0, 40},
            {ZoomChange::ZOOM_IN, 122, 255, 130}
        );           
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);  

        // section 6
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 128, 0, 50},
            {ZoomChange::ZOOM_IN, 90, 255, 170}
        );       

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 40, false);        

        // battle the titan phase 2
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});  
        env.console.log("Battle Great Tusk/Iron Treads Titan phase 2.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
       
    });

}

void checkpoint_40(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        // fly to Mesagoza East
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 185, 440});

        // place down marker, for section 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 180, 90);

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 5.60);

        get_on_ride(env.program_info(), env.console, context);

        // jump over the fence to exit Mesagoza
        pbf_move_left_joystick(context, 128, 0, 200, 50);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 200);

        wait_for_overworld(env.program_info(), env.console, context);

        // section 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
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
        
        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 128, 40},
            {ZoomChange::ZOOM_IN, 255, 140, 100}
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

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, 150, 50},
            {ZoomChange::ZOOM_IN, 255, 90, 120}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 0, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );               

        // section 4. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 50, 30},
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

        // section 5. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 160, 40);
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
