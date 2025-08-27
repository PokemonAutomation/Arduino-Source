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
#include "PokemonSV_AutoStory_Segment_03.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





std::string AutoStory_Segment_03::name() const{
    return "03: Catch Tutorial";
}

std::string AutoStory_Segment_03::start_text() const{
    return "Start: Battled Nemona on the beach.";
}

std::string AutoStory_Segment_03::end_text() const{
    return "End: Finished catch tutorial. Walked to the cliff and heard mystery cry. Standing in front of Nemona near the cliff.";
}

void AutoStory_Segment_03::run_segment(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment 03: Catch Tutorial", COLOR_ORANGE);

    checkpoint_05(env, context, options.notif_status_update, stats);
    checkpoint_06(env, context, options.notif_status_update, stats);
    checkpoint_07(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment 03: Catch Tutorial", COLOR_GREEN);

}


void checkpoint_05(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         
        context.wait_for_all_requests();

        DirectionDetector direction;
        direction.change_direction(env.program_info(), env.console, context, 1.92);
        pbf_move_left_joystick(context, 128, 0, 7 * TICKS_PER_SECOND, 50);        
        direction.change_direction(env.program_info(), env.console, context, 1.13);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);
        
        context.wait_for_all_requests();
        env.console.log("Get mom's sandwich");
        env.console.overlay().add_log("Get mom's sandwich", COLOR_WHITE);
        mash_button_till_overworld(env.console, context);
    
    });   
}

void checkpoint_06(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){        

        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 40, 82, 60);
        pbf_move_left_joystick(context, 128, 0, 6 * TICKS_PER_SECOND, 20);
        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 110, 10, 60);
        env.console.log("overworld_navigation: Go to Nemona.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20, 20, true, true);
        
        context.wait_for_all_requests();
        env.console.log("clear_dialog: Talk with Nemona to start catch tutorial. Stop when detect battle.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, 
            {CallbackEnum::WHITE_A_BUTTON, CallbackEnum::TUTORIAL, CallbackEnum::BATTLE});
        
        // can die in catch tutorial, and the story will continue. so need to detect wipeout
        env.console.log("Battle Lechonk in catch tutorial. Stop when detect dialog.");
        run_wild_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);

        env.console.log("clear_dialog: Talk with Nemona to finish catch tutorial. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, 
            {CallbackEnum::TUTORIAL, CallbackEnum::OVERWORLD});

        context.wait_for_all_requests();
        env.console.log("Finished catch tutorial");
        env.console.overlay().add_log("Finished catch tutorial", COLOR_WHITE);

    });

}

void checkpoint_07(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){         

        context.wait_for_all_requests();
        env.console.log("Move to cliff");
        env.console.overlay().add_log("Move to cliff", COLOR_WHITE);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 70, 100);
        env.console.log("overworld_navigation: Go to cliff.");
        overworld_navigation(env.program_info(), env.console, context, 
            NavigationStopCondition::STOP_TIME, NavigationMovementMode::DIRECTIONAL_ONLY, 
            135, 0, 24, 12, true, true);

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 80);
        handle_when_stationary_in_overworld(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 24, 12, true, true);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 128, 40, 50);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );               

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        context.wait_for_all_requests();
        env.console.log("Mystery cry");
        env.console.overlay().add_log("Mystery cry", COLOR_WHITE);
        
    });
       

}




}
}
}
