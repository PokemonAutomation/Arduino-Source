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
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AutoStoryTools.h"
#include "PokemonSV_AutoStory_Segment_04.h"

//#include <iostream>
//using std::cout;
//using std::endl;
//#include <unordered_map>
//#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





std::string AutoStory_Segment_04::name() const{
    return "04: Rescue Legendary";
}

std::string AutoStory_Segment_04::start_text() const{
    return "Start: Finished catch tutorial. Walked to the cliff and heard mystery cry. Standing in front of Nemona near the cliff.";
}

std::string AutoStory_Segment_04::end_text() const{
    return "End: Saved the Legendary. Escaped from the Houndoom cave. Standing next to Koraidon/Miraidon just outside the cave exit.";
}

void AutoStory_Segment_04::run_segment(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    AutoStoryOptions options,
    AutoStoryStats& stats
) const{

    stats.m_segment++;
    env.update_stats();
    context.wait_for_all_requests();
    env.console.log("Start Segment 04: Rescue Legendary", COLOR_ORANGE);

    checkpoint_08(env, context, options.notif_status_update, stats);

    context.wait_for_all_requests();
    env.console.log("End Segment 04: Rescue Legendary", COLOR_GREEN);

}

void checkpoint_08(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    checkpoint_reattempt_loop(env, context, notif_status_update, stats,
    [&](size_t attempt_number){

        context.wait_for_all_requests();

        realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 230, 70, 100);
        env.console.log("overworld_navigation: Go to cliff.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 30, 30, true, true);

        env.console.log("Look over the injured Miraidon/Koraidon on the beach.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {});
        env.console.log("Fall down the cliff.");
        pbf_wait(context, 20 * TICKS_PER_SECOND);  // long animation
        context.wait_for_all_requests();
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});
        env.console.log("Go to Legendary pokemon laying on the beach.");
        walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_SPAM_A, 30);

        env.console.log("clear_dialog: Offer Miraidon/Koraidon a sandwich.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});

        // TODO: Bag menu navigation
        context.wait_for_all_requests();
        env.console.log("Feed mom's sandwich");
        env.console.overlay().add_log("Feed mom's sandwich", COLOR_WHITE);
        
        GradientArrowWatcher arrow(COLOR_RED, GradientArrowType::RIGHT, {0.104, 0.312, 0.043, 0.08});
        context.wait_for_all_requests();

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_dpad(context, DPAD_UP, 20, 250);
                }
            },
            {arrow}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to feed mom's sandwich.",
                env.console
            );  
        }

        // only press A when the sandwich is selected
        pbf_mash_button(context, BUTTON_A, 100);

        env.console.log("Miraidon/Koraidon eats the sandwich.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 25, {});
        env.console.log("Miraidon/Koraidon gets up and walks to cave entrance."); // long animation
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {});

        // First Nemona cave conversation
        context.wait_for_all_requests();
        env.console.log("Enter cave");
        env.console.overlay().add_log("Enter cave", COLOR_WHITE);
        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                pbf_move_left_joystick(context, 128, 0, 600, 50);
                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 150, 20, 20);
                pbf_move_left_joystick(context, 128, 0, 1000, 50);
                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 160, 20, 20);
                overworld_navigation(env.program_info(), env.console, context, 
                    NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 
                    128, 0, 20, 20, true, true);
            }
        );  

        env.console.log("clear_dialog: Talk to Nemona yelling down, while you're down in the cave.");
        clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 10, {CallbackEnum::PROMPT_DIALOG});

        do_action_and_monitor_for_battles(env.program_info(), env.console, context,
            [&](const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
                // Legendary rock break
                context.wait_for_all_requests();
                stream.log("Rock break");
                stream.overlay().add_log("Rock break", COLOR_WHITE);
                pbf_move_left_joystick(context, 128, 20, 3 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 230, 25, 30);
                pbf_move_left_joystick(context, 128, 0, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

                // Houndour wave
                context.wait_for_all_requests();
                stream.log("Houndour wave");
                stream.overlay().add_log("Houndour wave", COLOR_WHITE);
                // walk to room entrance
                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 150, 15, 30);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 210, 15, 30);
                pbf_move_left_joystick(context, 128, 20, 3 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 200, 25, 20);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);

                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 210, 25, 25);
                pbf_move_left_joystick(context, 128, 20, 6 * TICKS_PER_SECOND, 20 * TICKS_PER_SECOND);

                // Houndoom encounter
                context.wait_for_all_requests();
                stream.log("Houndoom encounter");
                stream.overlay().add_log("Houndoom encounter", COLOR_WHITE);
                pbf_move_left_joystick(context, 128, 20, 4 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 245, 20, 20);
                pbf_move_left_joystick(context, 128, 20, 2 * TICKS_PER_SECOND, 20);
                realign_player(env.program_info(), stream, context, PlayerRealignMode::REALIGN_NO_MARKER, 255, 90, 20);
                pbf_move_left_joystick(context, 128, 20, 8 * TICKS_PER_SECOND, 8 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_L, 20, 20);
            }
        );
        
        env.console.log("overworld_navigation: Go to Houndoom.");
        overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20, 20, true, true);
        
        mash_button_till_overworld(env.console, context, BUTTON_A);

    
    });


}



}
}
}
