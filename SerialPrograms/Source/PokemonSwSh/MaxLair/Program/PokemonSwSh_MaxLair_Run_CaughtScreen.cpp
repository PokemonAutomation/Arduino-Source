/*  Max Lair Run Caught Screen
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_CatchScreenTracker.h"
#include "PokemonSwSh_MaxLair_Run_CaughtScreen.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


StateMachineAction mash_A_to_entrance(
    MaxLairRuntime& runtime,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    const QImage& entrance
){
    EntranceDetector entrance_detector(entrance);

    int result = run_until(
        env, console,
        [&](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 60 * TICKS_PER_SECOND);
        },
        { &entrance_detector },
        INFERENCE_RATE
    );

    if (result < 0){
        console.log("Failed to detect entrance.", Qt::red);
//        PA_THROW_StringException("Failed to detect entrance.");
        runtime.stats.add_error();
        dump_image(console, MODULE_NAME, "ResetRecovery", console.video().snapshot());
        return StateMachineAction::RESET_RECOVER;
    }
    return StateMachineAction::KEEP_GOING;
}


void synchronize_caught_screen(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
){
    console.botbase().wait_for_all_requests();
    state_tracker.synchronize(env, console, console.index(), std::chrono::seconds(60));
}


StateMachineAction run_caught_screen(
    MaxLairRuntime& runtime,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const EndBattleDecider& decider,
    const QImage& entrance
){
    size_t console_index = console.index();

    pbf_wait(console, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    CaughtPokemonScreen tracker(env, console);
    runtime.stats.add_run(tracker.total());

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
        runtime.stats.add_shiny();
        if (c == 3){
            boss_is_shiny = true;
            runtime.stats.add_shiny_legendary();
        }
    }

    //  If anything is shiny, take a video.
    if (!shinies.empty()){
        tracker.scroll_to(shinies.back());
        tracker.leave_summary();
        env.wait_for(std::chrono::seconds(1));
        pbf_press_button(console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();
    }else{
        std::lock_guard<std::mutex> lg(env.lock());
        send_nonshiny_notification(
            console, runtime.notification_noshiny,
            runtime.program_name, runtime.stats
        );
    }

    //  Screencap all the shinies and send notifications.
    for (size_t index : shinies){
        tracker.scroll_to(index);
        tracker.leave_summary();
        QImage screen = console.video().snapshot();

        std::lock_guard<std::mutex> lg(env.lock());
        send_shiny_notification(
            console, runtime.notification_shiny,
            runtime.program_name,
            nullptr,
            runtime.stats,
            screen
        );
    }


    const std::string& boss = state_tracker[console_index].boss;
    EndBattleDecider::EndAdventureAction action = boss.empty()
        ? decider.end_adventure_action(console_index)
        : decider.end_adventure_action(console_index, boss, boss_is_shiny);

    switch (action){
    case EndBattleDecider::EndAdventureAction::STOP_PROGRAM:
        console.log("Stopping program...", "purple");
        synchronize_caught_screen(env, console, state_tracker);
        return StateMachineAction::STOP_PROGRAM;
    case EndBattleDecider::EndAdventureAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE:
        if (shinies.empty() || shinies[0] == 3){
            console.log("Quitting back to entrance.", "purple");
            tracker.leave_summary();
            synchronize_caught_screen(env, console, state_tracker);
            pbf_press_dpad(console, DPAD_DOWN, 10, 50);
            pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
            return mash_A_to_entrance(runtime, env, console, entrance);
        }else{
            console.log("Taking non-shiny boss and returning to entrance...", Qt::blue);
            tracker.scroll_to(shinies.back());
            tracker.enter_summary();    //  Enter summary to verify you're on the right mon.
            tracker.leave_summary();
            synchronize_caught_screen(env, console, state_tracker);
            return mash_A_to_entrance(runtime, env, console, entrance);
        }
    case EndBattleDecider::EndAdventureAction::RESET:
        console.log("Resetting game...", Qt::blue);
        synchronize_caught_screen(env, console, state_tracker);
        reset_game_from_home_with_inference(env, console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        return StateMachineAction::KEEP_GOING;
    }

    PA_THROW_StringException("Invalid enum.");
}


}
}
}
}
