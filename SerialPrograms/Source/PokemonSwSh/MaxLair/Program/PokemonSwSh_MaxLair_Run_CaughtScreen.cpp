/*  Max Lair Run Caught Screen
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh_MaxLair_Run_CaughtScreen.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void summary_to_caught_screen(ProgramEnvironment& env, ConsoleHandle& console){
    pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);

    PokemonCaughtMenuDetector caught_menu;

    int result = wait_until(
        env, console,
        std::chrono::seconds(10),
        { &caught_menu }
    );

    switch (result){
    case 0:
        pbf_wait(console, 125);
        console.botbase().wait_for_all_requests();
        return;
    default:
        console.log("Failed to detect caught menu.", Qt::red);
        PA_THROW_StringException("Failed to detect caught menu.");
    }
}
void mash_A_to_entrance(ProgramEnvironment& env, ConsoleHandle& console, const QImage& entrance){
    EntranceDetector entrance_detector(entrance);

    int result = run_until(
        env, console,
        [&](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 30 * TICKS_PER_SECOND);
            context->wait_for_all_requests();
        },
        { &entrance_detector }
    );

    if (result < 0){
        console.log("Failed to detect entrance.", Qt::red);
        PA_THROW_StringException("Failed to detect entrance.");
    }
}




StateMachineAction run_caught_screen(
    MaxLairRuntime& runtime,
    ProgramEnvironment& env,
    ConsoleHandle& console, bool is_host,
    const QImage& entrance
){
    pbf_wait(console, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    SummaryShinySymbolDetector detector(console, console);
    size_t count = count_catches(console, console.video().snapshot());
    console.log(STRING_POKEMON + " Caught: " + QString::number(count), "purple");
    runtime.stats.add_run(count);

    pbf_press_button(console, BUTTON_A, 10, 100);
    pbf_press_dpad(console, DPAD_DOWN, 10, 50);
    pbf_press_button(console, BUTTON_A, 10, 0);
    console.botbase().wait_for_all_requests();

    bool shiny_found = false;
    bool shiny[4] = {false, false, false, false};
    for (size_t c = 0; c < count; c++){
        if (c != 0){
            pbf_press_dpad(console, DPAD_DOWN, 10, TICKS_PER_SECOND);
            console.botbase().wait_for_all_requests();
        }

        SummaryShinySymbolDetector::Detection detection = detector.wait_for_detection(env, console);
        switch (detection){
        case SummaryShinySymbolDetector::Detection::NO_DETECTION:
            console.log("Failed to detect summary screen.", Qt::red);
            PA_THROW_StringException("Failed to detect summary screen.");
        case SummaryShinySymbolDetector::Detection::NOT_SHINY:
            shiny[c] = false;
            shiny_found |= false;
            break;
        case SummaryShinySymbolDetector::Detection::SHINY:
            console.log("Found shiny!", Qt::blue);
            shiny[c] = true;
            shiny_found |= true;
            runtime.stats.add_shiny();
            break;
        }
    }

    //  No shinies
    if (!shiny_found){
        switch (runtime.action){
        case CaughtScreenAction::ALWAYS_STOP:
            console.log("No shinies found.", "purple");
            return StateMachineAction::STOP_PROGRAM;
        case CaughtScreenAction::RESET_HOST_IF_NON_SHINY_BOSS:
            if (is_host){
                console.log("No shinies found. Resetting host.", "purple");
                reset_game_from_home_with_inference(env, console, true);
                return StateMachineAction::KEEP_GOING;
            }
            //  Intentional fall-through.
        case CaughtScreenAction::TAKE_NON_BOSS_STOP_ON_NOTHING:
        case CaughtScreenAction::TAKE_NON_BOSS_STOP_ON_SHINY_BOSS:
            console.log("Quitting back to entrance.", "purple");
            summary_to_caught_screen(env, console);
            pbf_press_dpad(console, DPAD_DOWN, 10, 50);
            pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
            mash_A_to_entrance(env, console, entrance);
            return StateMachineAction::KEEP_GOING;
        }
    }

    //  Boss is shiny
    if (shiny[3]){
        runtime.stats.add_shiny_legendary();
        summary_to_caught_screen(env, console);

        QImage screen = console.video().snapshot();
        {
            std::lock_guard<std::mutex> lg(env.lock());
            send_shiny_notification(console, runtime.program_name, nullptr, screen, runtime.stats);
        }
        pbf_press_button(console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);

        switch (runtime.action){
        case CaughtScreenAction::ALWAYS_STOP:
        case CaughtScreenAction::RESET_HOST_IF_NON_SHINY_BOSS:
        case CaughtScreenAction::TAKE_NON_BOSS_STOP_ON_SHINY_BOSS:
            console.log("Boss is shiny! Stopping program...", Qt::blue);
            return StateMachineAction::STOP_PROGRAM;
        case CaughtScreenAction::TAKE_NON_BOSS_STOP_ON_NOTHING:
            console.log("Boss is shiny! But you're not stopping...", Qt::blue);
            pbf_press_dpad(console, DPAD_DOWN, 10, 50);
            pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
            mash_A_to_entrance(env, console, entrance);
            return StateMachineAction::KEEP_GOING;
        }
    }

    //  Move to the shiny.
    size_t c = count;
    while (c > 0){
        c--;
        if (shiny[c]){
            break;
        }
        pbf_press_dpad(console, DPAD_UP, 10, TICKS_PER_SECOND);
    }

    console.botbase().wait_for_all_requests();
    QImage screen = console.video().snapshot();
    SummaryShinySymbolDetector::Detection detection = detector.detect(screen);
    switch (detection){
    case SummaryShinySymbolDetector::Detection::NO_DETECTION:
        dump_image(console, screen, "CaughtScreen-Shiny");
        PA_THROW_StringException("Unrecoverable Error: Unable to detect shiny status.");
    case SummaryShinySymbolDetector::Detection::NOT_SHINY:
        dump_image(console, screen, "CaughtScreen-Shiny");
        PA_THROW_StringException("Fatal Inconsistency: Expected to see a shiny.");
    case SummaryShinySymbolDetector::Detection::SHINY:
        break;
    }

    summary_to_caught_screen(env, console);

    screen = console.video().snapshot();
    send_shiny_notification(console, runtime.program_name, nullptr, screen, runtime.stats);

    //  Non-boss is shiny.
    if (is_host && runtime.action == CaughtScreenAction::RESET_HOST_IF_NON_SHINY_BOSS){
        console.log("Shiny found! But you're resetting to keep the path...", Qt::blue);
        reset_game_from_home_with_inference(env, console, true);
        return StateMachineAction::KEEP_GOING;
    }

    switch (runtime.action){
    case CaughtScreenAction::ALWAYS_STOP:
        console.log("Shiny found. Stopping program...", Qt::blue);
        return StateMachineAction::STOP_PROGRAM;
    case CaughtScreenAction::TAKE_NON_BOSS_STOP_ON_SHINY_BOSS:
    case CaughtScreenAction::TAKE_NON_BOSS_STOP_ON_NOTHING:
    case CaughtScreenAction::RESET_HOST_IF_NON_SHINY_BOSS:
        console.log("Shiny found. Taking it and continuing...", Qt::blue);
        mash_A_to_entrance(env, console, entrance);
        return StateMachineAction::KEEP_GOING;
    }

    return StateMachineAction::STOP_PROGRAM;
}


}
}
}
}
