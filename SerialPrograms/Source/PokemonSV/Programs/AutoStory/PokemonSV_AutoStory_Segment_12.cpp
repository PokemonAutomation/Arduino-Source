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
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h"
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

using namespace Pokemon;




std::string AutoStory_Segment_12::name() const{
    return "10.3: Cortondo Gym - Gym battle";
}

std::string AutoStory_Segment_12::start_text() const{
    return "Start: Beat Cortondo Gym challenge.";
}

std::string AutoStory_Segment_12::end_text() const{
    return "End: Beat Cortondo Gym battle. At Cortondo West Pokecenter.";
}


void AutoStory_Segment_12::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.overlay().add_log("Start Segment 10.3: Cortondo Gym - Gym battle", COLOR_ORANGE);

    checkpoint_28(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment 10.3: Cortondo Gym - Gym battle", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats();

}


void checkpoint_28(
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
        DirectionDetector direction;
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
                direction.change_direction(env.program_info(), env.console, context, 2.71);
                pbf_move_left_joystick(context, 128, 0, 375, 100);
                direction.change_direction(env.program_info(), env.console, context, 1.26);
                pbf_move_left_joystick(context, 128, 0, 1750, 100);                
        });        
       
        direction.change_direction(env.program_info(), env.console, context, 2.73);

        NoMinimapWatcher no_minimap(env.console, COLOR_RED, Milliseconds(2000));
        int ret = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                handle_when_stationary_in_overworld(env.program_info(), env.console, context, 
                    [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){           
                        pbf_move_left_joystick(context, 128, 0, 10 * TICKS_PER_SECOND, 100);
                    }, 
                    [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){           
                        pbf_move_left_joystick(context, 0, 0, 100, 20);
                    },
                    5, 3
                );     
            },
            {no_minimap}
        );
        if (ret < 0){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT,
                env.logger(),
                "Failed to enter Cortondo Gym."
            );
        }

        wait_for_overworld(env.program_info(), env.console, context);

        // talk to receptionist
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 60, {CallbackEnum::BATTLE, CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW});

        // battle Katy
        run_battle_press_A(env.console, context, BattleStopCondition::STOP_DIALOG, {CallbackEnum::GRADIENT_ARROW}, true);
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
