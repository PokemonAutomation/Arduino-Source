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
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_05().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_06().run_checkpoint(env, context, options, stats);
    AutoStory_Checkpoint_07().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_05::name() const{ return "005 - " + AutoStory_Segment_03().name(); }
std::string AutoStory_Checkpoint_05::start_text() const{ return "Battled Nemona on the beach.";}
std::string AutoStory_Checkpoint_05::end_text() const{ return "Met mom at gate. Received mom's sandwich.";}
void AutoStory_Checkpoint_05::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_05(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_06::name() const{ return "006 - " + AutoStory_Segment_03().name(); }
std::string AutoStory_Checkpoint_06::start_text() const{ return AutoStory_Checkpoint_05().end_text();}
std::string AutoStory_Checkpoint_06::end_text() const{ return "Cleared catch tutorial.";}
void AutoStory_Checkpoint_06::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_06(env, context, options.notif_status_update, stats);
}

std::string AutoStory_Checkpoint_07::name() const{ return "007 - " + AutoStory_Segment_03().name(); }
std::string AutoStory_Checkpoint_07::start_text() const{ return AutoStory_Checkpoint_06().end_text();}
std::string AutoStory_Checkpoint_07::end_text() const{ return "Moved to cliff. Heard mystery cry. Standing in front of Nemona near the cliff.";}
void AutoStory_Checkpoint_07::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_07(env, context, options.notif_status_update, stats);
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
        pbf_move_left_joystick(context, 128, 0, 7000ms, 400ms);
        direction.change_direction(env.program_info(), env.console, context, 1.13);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 20);

        context.wait_for_all_requests();
        env.console.log("Get mom's sandwich");
        env.console.overlay().add_log("Get mom's sandwich", COLOR_WHITE);
        mash_button_till_overworld(env.console, context);
    
    }, false);   
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
        pbf_move_left_joystick(context, 128, 0, 6000ms, 160ms);
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

    }, false);

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
                pbf_move_left_joystick(context, {-1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );               

        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::OVERWORLD});

        context.wait_for_all_requests();
        env.console.log("Mystery cry");
        env.console.overlay().add_log("Mystery cry", COLOR_WHITE);
        
    }, false);
       

}




}
}
}
