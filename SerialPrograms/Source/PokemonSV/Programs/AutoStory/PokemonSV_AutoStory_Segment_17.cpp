/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_17.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_17::name() const{
    return "17: Cascarrafa Gym (Water): Gym challenge and Gym battle";
}

std::string AutoStory_Segment_17::start_text() const{
    return "Start: Received Kofu's wallet. At Porto Marinada Pokecenter.";
}

std::string AutoStory_Segment_17::end_text() const{
    return "End: Defeated Cascarrafa Gym (Water). At Porto Marinada Pokecenter.";
}

void AutoStory_Segment_17::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_37(env, context, options.notif_status_update, stats);
    checkpoint_38(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_37(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        DirectionDetector direction;
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 255, 50);

                // section 1
                direction.change_direction(env.program_info(), env.console, context, 1.606);
                pbf_move_left_joystick(context, 128, 0, 200, 100);

                get_on_ride(env.program_info(), env.console, context);

                // section 2
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
        });

        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_MARKER, NavigationMovementMode::DIRECTIONAL_ONLY, 
            128, 0, 30, 10, false);          
        // section 3. set marker to shop/Kofu
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 0, 140, 27);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);

        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Kofu's assistant.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

       
    });

}

void checkpoint_38(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 255, 180, 170});
        DirectionDetector direction;
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                direction.change_direction(env.program_info(), env.console, context, 0.3491);
                pbf_move_left_joystick(context, 128, 0, 400, 100);
                direction.change_direction(env.program_info(), env.console, context, 5.075911);
                pbf_move_left_joystick(context, 128, 0, 525, 100);                
        });

        direction.change_direction(env.program_info(), env.console, context, 3.771252);
        get_on_ride(env.program_info(), env.console, context);
        // walk towards elevator
        pbf_move_left_joystick(context, 128, 0, 700, 100);
        // jump to ensure you get on elevator
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 200);
        pbf_wait(context, 3 * TICKS_PER_SECOND);
        // wait for overworld to reappear after stepping off elevator
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_left_joystick(context, 128, 0, 120, 100);     

        direction.change_direction(env.program_info(), env.console, context, 5.11);  
        pbf_move_left_joystick(context, 128, 0, 1600, 100); 
        direction.change_direction(env.program_info(), env.console, context, 3.2245);          
        
        
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 18);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 255, 0, 100, 50);
                pbf_move_left_joystick(context, 0, 0, 100, 50);
            }
        );         
        // talk to Nemona
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // talk to reception. Battle Kofu
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW});
        env.console.log("Battle Water Gym.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);
        
        // Gym now defeated. now in Cascaraffa gym building
        context.wait_for_all_requests();
        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);
        // wait for overworld after leaving Gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        // fly to Porto Marinada pokecenter
        move_cursor_towards_flypoint_and_go_there(env.program_info(), env.console, context, {ZoomChange::KEEP_ZOOM, 0, 80, 150});        

    });

}


}
}
}
