/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_10.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




std::string AutoStory_Segment_10::name() const{
    return "10.1: Cortondo Gym - Go to Cortondo city";
}

std::string AutoStory_Segment_10::start_text() const{
    return "Start: After the break, with level 100 Gardevoir. At Mesagoza West pokecenter.";
}

std::string AutoStory_Segment_10::end_text() const{
    return "End: At Cortondo East Pokecenter.";
}

void AutoStory_Segment_10::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.overlay().add_log("Start Segment 10.1: Cortondo Gym - Go to Cortondo city", COLOR_ORANGE);

    checkpoint_21(env, context, options.notif_status_update);
    checkpoint_22(env, context, options.notif_status_update);
    checkpoint_23(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment 10.1: Cortondo Gym - Go to Cortondo city", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats();

}

void checkpoint_21(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        fly_to_overlapping_flypoint(env.program_info(), env.console, context);

        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_L, 20, 20);
        // move forward
        pbf_move_left_joystick(context, 128, 0, 30, 100);
        // get on ride
        get_on_ride(env.program_info(), env.console, context);
        // turn left
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NO_MARKER, 0, 128, 50);
        // move forward
        pbf_move_left_joystick(context, 128, 0, 100, 100);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 200, 70);
        pbf_move_left_joystick(context, 128, 0, 400, 100);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 128, 70);
        pbf_move_left_joystick(context, 128, 0, 700, 100);

        // turn towards wall
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 50);
        pbf_move_left_joystick(context, 128, 0, 200, 100);
        // run and jump over wall
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 100);

        BlackScreenOverWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(30),
            { black_screen }
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "checkpoint_21(): Failed to jump the East Mesagoza wall.",
                true
            );
        }
        context.wait_for_all_requests();
        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
        break;
    }catch(...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}

void checkpoint_22(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();
        // section 1
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 80},
            {ZoomChange::KEEP_ZOOM, 255, 80, 37}            
        );
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 20);

        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 40},
            {ZoomChange::KEEP_ZOOM, 255, 255, 27}            
        );        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 15);
        
        // section 3. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 128, 128, 0},
            {ZoomChange::KEEP_ZOOM, 128, 128, 0}            
        );         
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 15);

        // section 3. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){                        
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 70, 30);
        });            
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 20, 12, 12);        

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
        break;
    }catch(...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}



void checkpoint_23(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }         
        context.wait_for_all_requests();

        // section 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 110, 0, 30);

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10);

        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 80},
            {ZoomChange::KEEP_ZOOM, 255, 95, 100}            
        );         
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10);       

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 80},
            {ZoomChange::KEEP_ZOOM, 255, 75, 65}            
        );              
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10);             

        // section 4
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 0, 128, 50},
            {ZoomChange::KEEP_ZOOM, 255, 180, 17}            
        );              
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 10);    

        // section 5. set marker to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context,
            {ZoomChange::ZOOM_IN, 128, 128, 0},
            {ZoomChange::KEEP_ZOOM, 128, 128, 0}            
        );              
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10);     

        // section 6. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){                        
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 110, 50);
        });                     
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12);         

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);             
       
        break;
    }catch(...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }             
    }

}



}
}
}
