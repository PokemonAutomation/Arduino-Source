/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_22.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




std::string AutoStory_Segment_22::name() const{
    return "17.1: Team Star (Fire): Beat Team Star";
}

std::string AutoStory_Segment_22::start_text() const{
    return "Start: At East Province (Area One) Pokecenter.";
}

std::string AutoStory_Segment_22::end_text() const{
    return "End: ";
}

void AutoStory_Segment_22::run_segment(SingleSwitchProgramEnvironment& env, BotBaseContext& context, AutoStoryOptions options) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    context.wait_for_all_requests();
    env.console.overlay().add_log("Start Segment ", COLOR_ORANGE);

    // checkpoint_(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment ", COLOR_GREEN);
    stats.m_segment++;
    env.update_stats();

}


// todo: uncomment checkpoint_save
void checkpoint_47(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            // checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        context.wait_for_all_requests();


       
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


// todo: uncomment checkpoint_save
void checkpoint_48(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            // checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        context.wait_for_all_requests();


       
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


// todo: uncomment checkpoint_save
void checkpoint_49(
    SingleSwitchProgramEnvironment& env, 
    BotBaseContext& context, 
    EventNotificationOption& notif_status_update
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();
    bool first_attempt = true;
    while (true){
    try{
        if (first_attempt){
            // checkpoint_save(env, context, notif_status_update);
            first_attempt = false;
        }else{
            enter_menu_from_overworld(env.program_info(), env.console, context, -1);
            // we wait 10 seconds then save, so that the initial conditions are slightly different on each reset.
            env.log("Wait 10 seconds.");
            context.wait_for(Milliseconds(10 * 1000));
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        context.wait_for_all_requests();


       
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
