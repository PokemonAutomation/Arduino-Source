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
#include "PokemonSV_AutoStory_Segment_07.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




std::string AutoStory_Segment_07::name() const{
    return "07: Go to Mesagoza South";
}

std::string AutoStory_Segment_07::start_text() const{
    return "Start: At Los Platos Pokecenter.";
}

std::string AutoStory_Segment_07::end_text() const{
    return "End: At Mesagoza South Pokecenter.";
}

void AutoStory_Segment_07::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.log("Start Segment 07: Go to Mesagoza South", COLOR_ORANGE);
    env.console.overlay().add_log("Start Segment 07: Go to Mesagoza South", COLOR_ORANGE);

    checkpoint_12(env, context, options.notif_status_update);

    // // Mystery Gift, delete later
    // enter_menu_from_overworld(env.program_info(), env.console, context, 2);
    // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
    // pbf_press_dpad(context, DPAD_UP, 20, 105);
    // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
    // pbf_press_dpad(context, DPAD_DOWN, 20, 105);
    // pbf_press_button(context, BUTTON_A, 20, 4 * TICKS_PER_SECOND);
    // pbf_press_button(context, BUTTON_A, 20, 10 * TICKS_PER_SECOND);
    // clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10);

    context.wait_for_all_requests();
    env.console.log("End Segment 07: Go to Mesagoza South", COLOR_GREEN);
    env.console.overlay().add_log("End Segment 07: Go to Mesagoza South", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats();

}

void checkpoint_12(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    // reset rate: ~25%. 12 resets out of 52. 
    // resets due to: getting attacked by wild pokemon, either from behind, 
    // or when lead pokemon not strong enough to clear them with Let's go
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
        [&](const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){        
        
            if (first_attempt){
                checkpoint_save(env, context, notif_status_update);
                first_attempt = false;
            } 

            context.wait_for_all_requests();
            
            fly_to_overlapping_flypoint(info, env.console, context);

            // re-orient camera
            pbf_press_button(context, BUTTON_L, 20, 20);

            walk_forward_while_clear_front_path(info, env.console, context, 35);
            realign_player(info, env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 100);
            walk_forward_while_clear_front_path(info, env.console, context, 500);
            realign_player(info, env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 100, 0, 120);
            walk_forward_while_clear_front_path(info, env.console, context, 2000);
            realign_player(info, env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 255, 0, 120);
            walk_forward_while_clear_front_path(info, env.console, context, 1250);
            
            // check we're not still at the Los Platos Pokecenter.
            confirm_no_overlapping_flypoint(info, env.console, context);

            // not stuck at Los Platos Pokecenter
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_B, 20, 1 * TICKS_PER_SECOND);
            press_Bs_to_back_to_overworld(info, env.console, context, 7);

            realign_player(info, env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 80, 0, 100);
            walk_forward_while_clear_front_path(info, env.console, context, 1500);
            realign_player(info, env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 30, 0, 50);
            walk_forward_while_clear_front_path(info, env.console, context, 1000);

            fly_to_overlapping_flypoint(info, env.console, context);

            // heal_at_pokecenter(info, env.console, context);
   
        }  
        );

        env.console.log("Reached Mesagoza (South) Pokecenter.");
        
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
