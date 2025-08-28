/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_12.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_12::name() const{
    return "12: Cortondo Gym (Bug): Gym battle";
}

std::string AutoStory_Segment_12::start_text() const{
    return "Start: Beat Cortondo Gym challenge. At Cortondo East Pokecenter.";
}

std::string AutoStory_Segment_12::end_text() const{
    return "End: Beat Cortondo Gym battle. At Cortondo West Pokecenter.";
}


void AutoStory_Segment_12::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    checkpoint_28(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}


void checkpoint_28(
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
                direction.change_direction(env.program_info(), env.console, context, 2.71);
                pbf_move_left_joystick(context, 128, 0, 375, 100);
                direction.change_direction(env.program_info(), env.console, context, 1.26);
                pbf_move_left_joystick(context, 128, 0, 1750, 100);                
        });        
       
        direction.change_direction(env.program_info(), env.console, context, 2.73);

        NoMinimapWatcher no_minimap(env.console, COLOR_RED, Milliseconds(2000));
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
                    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                        pbf_move_left_joystick(context, 128, 0, 10 * TICKS_PER_SECOND, 100);
                    }, 
                    [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                        pbf_move_left_joystick(context, 0, 0, 100, 20);
                    },
                    5, 3
                );     
            },
            {no_minimap}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to enter Cortondo Gym.",
                env.console
            );
        }

        wait_for_overworld(env.program_info(), env.console, context);

        // talk to receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW});

        // battle Katy
        env.console.log("Battle Grass Gym.");
        run_trainer_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG);
        mash_button_till_overworld(env.console, context, BUTTON_A, 360);

        // leave gym building
        pbf_move_left_joystick(context, 128, 255, 500, 100);
        pbf_wait(context, 3 * TICKS_PER_SECOND);        
        // wait for overworld after leaving gym
        wait_for_overworld(env.program_info(), env.console, context, 30);

        pbf_move_left_joystick(context, 128, 0, 450, 100);
        direction.change_direction(env.program_info(), env.console, context, 1.26);
        pbf_move_left_joystick(context, 128, 0, 1600, 100);
        fly_to_overlapping_flypoint(env.program_info(), env.console, context);
       
    });

}




}
}
}
