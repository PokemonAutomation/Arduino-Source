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
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_06.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





std::string AutoStory_Segment_06::name() const{
    return "06: Go to Los Platos";
}

std::string AutoStory_Segment_06::start_text() const{
    return "Start: Battled Arven, received Legendary's Pokeball. Talked to Nemona at roof of Lighthouse.";
}

std::string AutoStory_Segment_06::end_text() const{
    return "End: At Los Platos Pokecenter. Cleared Let's go tutorial.";
}

void AutoStory_Segment_06::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_11().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_11::name() const{ return "011 - " + AutoStory_Segment_06().name(); }
std::string AutoStory_Checkpoint_11::start_text() const{ return "Talked to Nemona at roof of the Lighthouse.";}
std::string AutoStory_Checkpoint_11::end_text() const{ return "Arrived at Los Platos pokecenter. Cleared Let's go tutorial.";}
void AutoStory_Checkpoint_11::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_11(env, context, options.notif_status_update, stats);
}



void checkpoint_11(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){ 

        context.wait_for_all_requests();
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                realign_player(info, stream, context, PlayerRealignMode::REALIGN_NEW_MARKER, -0.219, -0.646, 800ms);
                pbf_move_left_joystick(context, {0, +1}, 1496ms, 160ms);
                pbf_move_left_joystick(context, {-1, 0}, 240ms, 8000ms);
                pbf_move_left_joystick(context, {0, +1}, 1000ms, 2000ms);

                realign_player(info, stream, context, PlayerRealignMode::REALIGN_NEW_MARKER, -0.219, +0.531, 1600ms);
            }
        );     

        env.console.log("overworld_navigation: Go to Los Platos.");
        handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 75, 75, true, true);
            }, 
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, {-1, 0}, 320ms, 400ms);
                realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_OLD_MARKER);
            }
        );

        // cover for case where you fall into water. 
        // This will clear the dialog telling you that you fell into water, 
        // therefore preventing false positives on clear_dialog() assuming we're talking to Nemona.
        pbf_mash_button(context, BUTTON_A, 5000ms);
        context.wait_for_all_requests();
        

        env.console.log("clear_dialog: Talk with Nemona at Los Platos. Clear Let's go tutorial. Stop when detect overworld.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_OVERWORLD, 60, {CallbackEnum::TUTORIAL, CallbackEnum::OVERWORLD});

        context.wait_for_all_requests();

        env.console.log("Reached Los Platos");
        env.console.overlay().add_log("Reached Los Platos", COLOR_WHITE);

    }, false);

}




}
}
}
