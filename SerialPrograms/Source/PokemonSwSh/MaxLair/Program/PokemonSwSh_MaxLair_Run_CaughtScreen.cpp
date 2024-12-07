/*  Max Lair Run Caught Screen
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/Tools/InterruptableCommands.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
//#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
//#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_CatchScreenTracker.h"
#include "PokemonSwSh_MaxLair_Run_CaughtScreen.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


StateMachineAction mash_A_to_entrance(
    AdventureRuntime& runtime,
    ConsoleHandle& console, BotBaseContext& context,
    const ImageViewRGB32& entrance
){
    EntranceDetector entrance_detector(entrance);

    int result = run_until(
        console, context,
        [&](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 60 * TICKS_PER_SECOND);
        },
        {{entrance_detector}},
        INFERENCE_RATE
    );

    if (result < 0){
        console.log("Failed to detect entrance.", COLOR_RED);
        runtime.session_stats.add_error();
        dump_image(MODULE_NAME, console, "FailedToDetectEntrance");
        return StateMachineAction::RESET_RECOVER;
    }
    return StateMachineAction::KEEP_GOING;
}


void synchronize_caught_screen(
    ConsoleHandle& console, BotBaseContext& context,
    GlobalStateTracker& state_tracker
){
    context.wait_for_all_requests();
    state_tracker.synchronize(console, console.index(), std::chrono::seconds(60));
}


StateMachineAction run_caught_screen(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& decider,
    const ImageViewRGB32& entrance
){
    size_t console_index = console.index();
    bool is_host = console_index == runtime.host_index;

    pbf_wait(context, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    CaughtPokemonScreen tracker(console, context);
    runtime.session_stats.add_run(tracker.total());
    if (is_host){
        runtime.path_stats.add_run(tracker.total() >= 4);
//        cout << runtime.path_stats.to_str() << endl;
    }

    //  Scroll over everything. This checks them for shinies.
    tracker.enter_summary();
    for (size_t c = 0; c < tracker.total(); c++){
        tracker.scroll_to(c);
    }

    //  Get all the shinies.
    bool boss_is_shiny = false;
    std::vector<size_t> shinies;
    for (size_t c = 0; c < tracker.total(); c++){
        if (!tracker[c].shiny){
            continue;
        }
        shinies.emplace_back(c);
        runtime.session_stats.add_shiny();
        if (c == 3){
            boss_is_shiny = true;
            runtime.session_stats.add_shiny_legendary();
        }
    }

    //  If anything is shiny, take a video.
    if (!shinies.empty()){
        tracker.scroll_to(shinies.back());
        tracker.leave_summary();
        context.wait_for(std::chrono::seconds(1));
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }

    //  Screencap all the shinies and send notifications.
    for (size_t index : shinies){
        tracker.scroll_to(index);
        tracker.leave_summary();
        VideoSnapshot screen = console.video().snapshot();

        WriteSpinLock lg(runtime.m_lock);
        send_shiny_notification(
            env, console,
            runtime.notification_shiny,
            console_index, shinies.size(),
            nullptr,
            runtime.path_stats,
            runtime.session_stats,
            screen
        );
    }


    const std::string& boss = state_tracker[console_index].boss;
    CaughtScreenAction action =
        decider.end_adventure_action(
            console_index, boss,
            runtime.path_stats,
            !shinies.empty(), boss_is_shiny
        );

    switch (action){
    case CaughtScreenAction::STOP_PROGRAM:
        console.log("Stopping program...", COLOR_PURPLE);
        synchronize_caught_screen(console, context, state_tracker);
        return StateMachineAction::STOP_PROGRAM;

    case CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE:
        if (is_host){
            runtime.path_stats.clear();
        }
        if (shinies.empty() || shinies[0] == 3){
            console.log("Quitting back to entrance.", COLOR_PURPLE);
            tracker.leave_summary();
            synchronize_caught_screen(console, context, state_tracker);
            pbf_press_dpad(context, DPAD_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
            return mash_A_to_entrance(runtime, console, context, entrance);
        }else{
            console.log("Taking non-shiny boss and returning to entrance...", COLOR_BLUE);
            tracker.scroll_to(shinies[0]);
            tracker.enter_summary();    //  Enter summary to verify you're on the right mon.
            tracker.leave_summary();
            synchronize_caught_screen(console, context, state_tracker);
            StateMachineAction state = mash_A_to_entrance(runtime, console, context, entrance);
            if (state == StateMachineAction::RESET_RECOVER){
                throw_and_log<FatalProgramException>(
                    console,
                    ErrorReport::SEND_ERROR_REPORT,
                    "Unable to take " + Pokemon::STRING_POKEMON + ". Did you forget to disable nicknames?",
                    console
               );
            }
            return state;
        }

    case CaughtScreenAction::RESET:
        console.log("Resetting game...", COLOR_BLUE);
        synchronize_caught_screen(console, context, state_tracker);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
        reset_game_from_home_with_inference(console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        return StateMachineAction::DONE_WITH_ADVENTURE;
    }

    throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Invalid enum.");
}


}
}
}
}
