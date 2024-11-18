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
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_14.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




std::string AutoStory_Segment_14::name() const{
    return "11.2: Bombirdier Titan: Battle Bombirdier";
}

std::string AutoStory_Segment_14::start_text() const{
    return "Start: At West Province Area One Central Pokecenter";
}

std::string AutoStory_Segment_14::end_text() const{
    return "End: Defeated Bombirder. At West Province Area One North Pokecenter";
}

void AutoStory_Segment_14::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.overlay().add_log("Start Segment 11.2: Bombirdier Titan: Battle Bombirdier", COLOR_ORANGE);

    checkpoint_30(env, context, options.notif_status_update);
    checkpoint_31(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment 11.2: Bombirdier Titan: Battle Bombirdier", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats();

}



void checkpoint_30(
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
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 128, 17);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);    


        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 200, 200},
            {ZoomChange::KEEP_ZOOM, 0, 65, 220}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 200, 200},
            {ZoomChange::KEEP_ZOOM, 0, 80, 235}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        // section 4. walk until Arven dialog
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 200, 200},
            {ZoomChange::KEEP_ZOOM, 0, 60, 280}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 30, {CallbackEnum::OVERWORLD, CallbackEnum::BLACK_DIALOG_BOX});

        // after Arven dialog. section 5
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 18, 6, false);        

        // section 6
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 0},
            {ZoomChange::KEEP_ZOOM, 0, 20, 65}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        get_on_ride(env.program_info(), env.console, context);

        // section 7
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 0, 0, 0},
            {ZoomChange::KEEP_ZOOM, 0, 30, 80}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        // section 8. enter left side of boulder field
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 255, 50},
            {ZoomChange::KEEP_ZOOM, 0, 40, 95}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 18, 6, false); 

        // section 8.1. move up
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 0, 50);
        pbf_move_left_joystick(context, 128, 0, 100, 100);

        // section 9. go to middle-right of boulder field
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 255, 50},
            {ZoomChange::KEEP_ZOOM, 0, 15, 110}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 5, false);  

        // // section 9.1. go to right edge of boulder field
        // realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 140, 0, 50);        
        // pbf_move_left_joystick(context, 128, 0, 200, 100);

        // section 10. walk up right edge
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 255, 80},
            {ZoomChange::KEEP_ZOOM, 0, 12, 130}
        );      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 5, false);  

        // section 10.1 walk up right edge. until hit rock
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 15, 50);          

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            140, 0, 10, 5, false);        

        // section 10.2. move away from rock.
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 80, 255, 50);  
        pbf_move_left_joystick(context, 128, 0, 200, 100);

        // section 11
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 255, 100},
            {ZoomChange::KEEP_ZOOM, 0, 5, 150}
        );      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 5, false);  

        // section 12. reach the top. battle Bombirdier
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 255, 100},
            {ZoomChange::KEEP_ZOOM, 50, 0, 170}
        );
        try{
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_BATTLE, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 0, 40, 5, false);   
        }catch (OperationFailedException& e){ 
            (void) e;
            // likely attempted to open/close phone to realign, but failed
            // likely already reached cutscene to battle Bombirdeier.

            // keep waiting until battle detected.
            overworld_navigation(env.program_info(), env.console, context, 
                NavigationStopCondition::STOP_BATTLE, NavigationMovementMode::DIRECTIONAL_ONLY, 
                128, 128, 30, 30, false);          

        }       

        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 30, {CallbackEnum::BATTLE});
        // round 2 of battle 
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
        // get ride upgrade
        mash_button_till_overworld(env.console, context, BUTTON_A);

       
        break;
    }catch (...){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }              
    }

}


void checkpoint_31(
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
        // section 1. fall down the mountain
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 80, 180},
            {ZoomChange::KEEP_ZOOM, 0, 170, 120}
        );        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 50, 10, false); 
        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 80, 100},
            {ZoomChange::KEEP_ZOOM, 0, 255, 55}
        );  
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 10, false);         
        // section 3. align to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, 255, 0, 40},
            {ZoomChange::KEEP_ZOOM, 0, 0, 0}
        );  
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 10, false);      

        // section 4. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){                        
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 60, 40);
        });
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);   

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
        break;
    }catch (...){
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
