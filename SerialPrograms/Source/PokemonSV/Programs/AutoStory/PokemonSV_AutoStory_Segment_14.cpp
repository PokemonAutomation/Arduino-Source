/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
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




std::string AutoStory_Segment_14::name() const{
    return "14: Bombirdier Titan: Battle Bombirdier";
}

std::string AutoStory_Segment_14::start_text() const{
    return "Start: At West Province Area One Central Pokecenter";
}

std::string AutoStory_Segment_14::end_text() const{
    return "End: Defeated Bombirder. At West Province Area One North Pokecenter";
}

void AutoStory_Segment_14::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_30().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_31().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_30::name() const{ return "030 - " + AutoStory_Segment_14().name(); }
std::string AutoStory_Checkpoint_30::start_text() const{ return "At West Province Area One Central Pokecenter.";}
std::string AutoStory_Checkpoint_30::end_text() const{ return "Defeated Bombirdier.";}
void AutoStory_Checkpoint_30::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_30(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_31::name() const{ return "031 - " + AutoStory_Segment_14().name(); }
std::string AutoStory_Checkpoint_31::start_text() const{ return AutoStory_Checkpoint_30().end_text();}
std::string AutoStory_Checkpoint_31::end_text() const{ return "At West Province Area One North Pokecenter.";}
void AutoStory_Checkpoint_31::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_31(env, context, options.notif_status_update, stats);
}

void checkpoint_30(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        
        if (attempt_number > 0 || ENABLE_TEST){
            env.console.log("Fly to neighbouring Pokecenter, then fly back, to clear any pokemon covering the minimap.");
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, +1, -1, 800ms});
            move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, -1, +1, 800ms});
        }

        heal_at_pokecenter(env.program_info(), env.console, context);

        // section 1
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 128, 17);
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);    


        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -0.567, 1600ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.492, 1760ms}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        // section 3
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -0.567, 1600ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.375, 1880ms}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        // section 4. walk until Arven dialog
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -0.567, 1600ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.531, 2240ms}
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
            {ZoomChange::ZOOM_IN, -1, +1, 0ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.844, 520ms}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        get_on_ride(env.program_info(), env.console, context);

        // section 7
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, -1, +1, 0ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.766, 640ms}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 20, 10, false);

        // section 8. enter left side of boulder field
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -1, 400ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.688, 760ms}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 18, 6, false); 

        // section 8.1. move up
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 0, 50);
        pbf_move_left_joystick(context, {0, +1}, 800ms, 800ms);

        // section 9. go to middle-right of boulder field
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -1, 400ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.883, 880ms}
        );          
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 5, false);  

        // // section 9.1. go to right edge of boulder field
        // realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 140, 0, 50);        
        // pbf_move_left_joystick(context, {0, +1}, 1600ms, 800ms);

        // section 10. walk up right edge
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -1, 640ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.906, 1040ms}
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
        pbf_move_left_joystick(context, {0, +1}, 1600ms, 800ms);

        // section 11
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -1, 800ms},
            {ZoomChange::KEEP_ZOOM, -1, +0.961, 1200ms}
        );      
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 5, false);  

        // section 12. reach the top. battle Bombirdier
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, -1, 800ms},
            {ZoomChange::KEEP_ZOOM, -0.609, +1, 1360ms}
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

        confirm_titan_battle(env, context);

        env.console.log("Battle Bombirdier Titan phase 1.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 30, {CallbackEnum::BATTLE});
        // round 2 of battle 
        env.console.log("Battle Bombirdier Titan phase 2.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::DIALOG_ARROW});
        // get ride upgrade
        mash_button_till_overworld(env.console, context, BUTTON_A);

       
    });

}


void checkpoint_31(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        // section 1. fall down the mountain
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, +0.375, 1440ms},
            {ZoomChange::KEEP_ZOOM, -1, -0.331, 960ms}
        );        
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 50, 10, false); 
        // section 2
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, +0.375, 800ms},
            {ZoomChange::KEEP_ZOOM, -1, -1, 440ms}
        );  
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 10, false);         
        // section 3. align to pokecenter
        realign_player_from_landmark(
            env.program_info(), env.console, context, 
            {ZoomChange::ZOOM_IN, +1, +1, 320ms},
            {ZoomChange::KEEP_ZOOM, -1, +1, 0ms}
        );  
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 40, 10, false);      

        // section 4. set marker past pokecenter
        handle_unexpected_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
            realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 60, 40);
        });
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 15, 12, 12, false);   

        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
    });

}




}
}
}
