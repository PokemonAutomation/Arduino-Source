/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_23.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_23::name() const{
    return "";
}

std::string AutoStory_Segment_23::start_text() const{
    return "Start: ";
}

std::string AutoStory_Segment_23::end_text() const{
    return "End: ";
}

void AutoStory_Segment_23::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options
) const{
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    // checkpoint_(env, context, options.notif_status_update);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

// todo: uncomment checkpoint_save
void checkpoint_50(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
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
    }catch(OperationFailedException&){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}


// todo: uncomment checkpoint_save
void checkpoint_51(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
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
    }catch(OperationFailedException&){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}

// todo: uncomment checkpoint_save
void checkpoint_52(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
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
    }catch(OperationFailedException&){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}

// todo: uncomment checkpoint_save
void checkpoint_53(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
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
    }catch(OperationFailedException&){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}

// todo: uncomment checkpoint_save
void checkpoint_54(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
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
    }catch(OperationFailedException&){
        context.wait_for_all_requests();
        env.console.log("Resetting from checkpoint.");
        reset_game(env.program_info(), env.console, context);
        stats.m_reset++;
        env.update_stats();
    }         
    }

}

// todo: uncomment checkpoint_save
void checkpoint_55(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
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
    }catch(OperationFailedException&){
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
