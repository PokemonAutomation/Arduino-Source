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

    AutoStory_Checkpoint_39().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_40().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_39::name() const{ return "039 - " + AutoStory_Segment_18().name(); }
std::string AutoStory_Checkpoint_39::start_text() const{ return "Defeated Cascarrafa Gym (Water). At Porto Marinada Pokecenter.";}
std::string AutoStory_Checkpoint_39::end_text() const{ return "Defeated Great Tusk/Iron Treads.";}
void AutoStory_Checkpoint_39::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_39(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_40::name() const{ return "040 - " + AutoStory_Segment_18().name(); }
std::string AutoStory_Checkpoint_40::start_text() const{ return AutoStory_Checkpoint_39().end_text();}
std::string AutoStory_Checkpoint_40::end_text() const{ return "At South Province (Area Three) Pokecenter.";}
void AutoStory_Checkpoint_40::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_40(env, context, options.notif_status_update, stats);
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
        // recently flew a significant distance, so minimap should be clear of Pokemon
        // flew from Cascaraffa gym building to Porto Marinada Pokecenter
        
        // section 1
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                direction.change_direction(env.program_info(), env.console, context, 4.677921);
                pbf_move_left_joystick(context, {0, +1}, 1200ms, 800ms);
        });        

        // section 2
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, +1, -0.803, 960ms);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 60, 10, false);         

        // section 3. enter circle
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::KEEP_ZOOM, -1, +1, 400ms},
            {ZoomChange::ZOOM_IN, +0.37, -1, 960ms}
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
                        {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
                        {ZoomChange::ZOOM_IN, +0.252, -1, 720ms}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 36, 12, false);  


                    // std::cout << "11:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
                        {ZoomChange::ZOOM_IN, +0.055, -1, 720ms}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 30, 10, false);       


                    // std::cout << "10:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
                        {ZoomChange::ZOOM_IN, -0.062, -1, 840ms}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 30, 10, false);            

                    // std::cout << "7:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
                        {ZoomChange::ZOOM_IN, -0.102, -1, 1016ms}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 48, 12, false);            

                    // std::cout << "6:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
                        {ZoomChange::ZOOM_IN, +0.055, -1, 1096ms}
                    );           
                    overworld_navigation(env.program_info(), env.console, context, 
                        NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                        128, 0, 30, 10, false);    

                    // std::cout << "2:00" << std::endl;
                    realign_player_from_landmark(
                        env.program_info(), env.console, context, 
                        {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
                        {ZoomChange::ZOOM_IN, +0.567, -1, 960ms}
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

        confirm_titan_battle(env, context);

        env.console.log("Battle Great Tusk/Iron Treads Titan phase 1.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_OVERWORLD);

        // section 5
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, +1, 320ms},
            {ZoomChange::ZOOM_IN, -0.047, -1, 1040ms}
        );           
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);  

        // section 6
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, 0, +1, 400ms},
            {ZoomChange::ZOOM_IN, -0.297, -1, 1360ms}
        );       

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 40, false);        

        // battle the titan phase 2
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE});  

        confirm_titan_battle(env, context);
        
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
        // fly to Mesagoza East from Great Tusk/Iron Treads
        // this clears Pokemon in minimap
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, +1, -0.449, 3520ms});

        // place down marker, for section 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, +1, -0.409, 720ms);

        DirectionDetector direction;
        // recently flew a significant distance, so minimap should be clear of Pokemon, or at least the Pokemon have been reset
        
        direction.change_direction(env.program_info(), env.console, context, 5.60);

        get_on_ride(env.program_info(), env.console, context);

        // jump over the fence to exit Mesagoza
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 400ms);
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 1600ms);

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
                pbf_move_left_joystick(context, {+1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        ); 
        
        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, 0, 320ms},
            {ZoomChange::ZOOM_IN, +1, -0.094, 800ms}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );   

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::KEEP_ZOOM, -1, -0.173, 400ms},
            {ZoomChange::ZOOM_IN, +1, +0.297, 960ms}
        );  
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 30, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, +1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );               

        // section 4. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, +0.609, 240ms},
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}
        );  

        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 10, false);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {+1, +1}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );             

        // section 5. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, +1, -0.252, 320ms);
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
