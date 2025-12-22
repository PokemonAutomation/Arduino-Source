/*  AutoStory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.h"
#include "PokemonSV/Inference/PokemonSV_TutorialDetector.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_40.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




std::string AutoStory_Segment_40::name() const{
    return "40: Finish Main Story";
}

std::string AutoStory_Segment_40::start_text() const{
    return "Start: Entered Zero Lab. Spoke to AI Professor.";
}

std::string AutoStory_Segment_40::end_text() const{
    return "End: Battled the AI Professor. Completed the game.";
}

void AutoStory_Segment_40::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{
    

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment " + name(), COLOR_ORANGE);

    AutoStory_Checkpoint_104().run_checkpoint(env, context, options, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment " + name(), COLOR_GREEN);

}

std::string AutoStory_Checkpoint_104::name() const{ return "104 - " + AutoStory_Segment_40().name(); }
std::string AutoStory_Checkpoint_104::start_text() const{ return "Entered Zero Lab. Spoke to AI Professor.";}
std::string AutoStory_Checkpoint_104::end_text() const{ return "Battled the AI Professor. Completed the game.";}
void AutoStory_Checkpoint_104::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
    checkpoint_104(env, context, options.notif_status_update, stats);
}

// std::string AutoStory_Checkpoint_105::name() const{ return "105 - " + AutoStory_Segment_40().name(); }
// std::string AutoStory_Checkpoint_105::start_text() const{ return "";}
// std::string AutoStory_Checkpoint_105::end_text() const{ return "";}
// void AutoStory_Checkpoint_105::run_checkpoint(SingleSwitchProgramEnvironment& env, ProControllerContext& context, AutoStoryOptions options, AutoStoryStats& stats) const{
//     checkpoint_105(env, context, options.notif_status_update, stats);
// }

void checkpoint_104(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        pbf_press_button(context, BUTTON_L, 240ms, 80ms);
        pbf_move_left_joystick(context, {0, +1}, 3200ms, 400ms);
        pbf_move_left_joystick(context, {-1, 0}, 1040ms, 400ms);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);

        // now in elevator
        mash_button_till_overworld(env.console, context, BUTTON_A);

        pbf_move_left_joystick(context, {+1, 0}, 560ms, 400ms);

        // talk to the AI professor
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);
        mash_button_till_overworld(env.console, context, BUTTON_A);

        // put the book in the machine
        pbf_press_button(context, BUTTON_L, 30, 10);
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 10);


        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE, 120, {CallbackEnum::BATTLE, CallbackEnum::PROMPT_DIALOG, CallbackEnum::DIALOG_ARROW});


        env.console.log("Battle AI Professor.");
        SinglesMoveEntry move1{SinglesMoveType::Move1, false};  // Moonblast
        SinglesMoveEntry move3{SinglesMoveType::Move3, false}; // Psychic
        

        std::vector<SinglesMoveEntry> move_table1 = {move3, move1};
        bool terastallized = false;
        // start with Psychic to defeat Iron Moth for Violet, which quad resists Moonblast.
        bool is_won = run_pokemon(env.console, context, move_table1, true, terastallized);
        if (!is_won){// throw exception if we lose
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to beat the AI Professor. Reset.",
                env.console
            );
        }

        
        clear_dialog(env.console, context, ClearDialogMode::STOP_BATTLE_DIALOG_ARROW, 120, {CallbackEnum::DIALOG_ARROW}, false);

        pbf_mash_button(context, BUTTON_A, 50);


        wait_for_gradient_arrow(env.program_info(), env.console, context, {0.75, 0.62, 0.05, 0.35}, 30);

        pbf_press_dpad(context, DPAD_DOWN, 13, 20);
        pbf_mash_button(context, BUTTON_A, 20);

        pbf_wait(context, 1000ms);
        pbf_press_dpad(context, DPAD_UP, 13, 20);
        pbf_mash_button(context, BUTTON_A, 500);

        env.console.log("Battle AI Professor's Koraidon/Miraidon.");
        SinglesMoveEntry move4{SinglesMoveType::Move4, false}; // Koraidon/Miraidon: Tera Blast
        SinglesMoveEntry move4_tera{SinglesMoveType::Move4, true}; // Koraidon/Miraidon: Tera Blast
        std::vector<SinglesMoveEntry> move_table2 = {move4, move4, move4, move4, move4, move4_tera};
        is_won = run_pokemon(env.console, context, move_table2, true, terastallized);
        if (!is_won){// throw exception if we lose
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to beat the AI Professor, round 2. This shouldn't be possible. Reset.",
                env.console
            );
        }

        mash_button_till_overworld(env.console, context, BUTTON_A, 800);

        env.console.log("Clear Koraidon/Miraidon form change tutorial.");
        // Press X until tutorial shows up
        TutorialWatcher tutorial;
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_button(context, BUTTON_X, 20, 250);
                }
            },
            {tutorial}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Stuck trying to clear the Koraidon/Miraidon form change tutorial.",
                env.console
            );  
        }
        clear_tutorial(env.console, context);


    }, false);
}

// void checkpoint_105(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EventNotificationOption& notif_status_update, AutoStoryStats& stats){
// }



}
}
}
