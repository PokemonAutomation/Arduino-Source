/*  Trainer Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

// #include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_MoveEffectivenessSymbol.h"
#include "PokemonLZA_TrainerBattle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


TrainerBattleState::TrainerBattleState(BattleAIOption& settings)
    : m_settings(settings)
    , m_consecutive_failures(0)
{}


std::string move_effectiveness_detection(MoveEffectivenessSymbol symbol){
    const char* str = to_string(symbol);
    if (str){
        return str;
    }
    return "(Unable to Detect)";
}


bool TrainerBattleState::attempt_one_attack(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    ProControllerContext& context
){
    AsyncCommandSession<ProController> command(
        context,
        console.logger(),
        env.realtime_dispatcher(),
        context
    );

    MoveEffectivenessSymbolWatcher move_watcher(COLOR_RED, &console.overlay(), 20ms);
    command.dispatch([](ProControllerContext& context){
        pbf_press_button(context, BUTTON_ZL, 5000ms, 0ms);
    });

    console.log("Begin looking for type symbols.");

    int ret = wait_until(
        console, context, 1000ms,
        {move_watcher}
    );
    if (ret < 0){
        command.stop_session_and_rethrow();
        context.wait_for(250ms);
        // No move effectiveness symbol found
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            run_lock_recovery(console, context);
        }
        return false;
    }

    std::string move_results = "Move Effectiveness:";
    move_results += "\n- Top: " + move_effectiveness_detection(move_watcher[0]);
    move_results += "\n- Left: " + move_effectiveness_detection(move_watcher[1]);
    move_results += "\n- Right: " + move_effectiveness_detection(move_watcher[2]);
    move_results += "\n- Bottom: " + move_effectiveness_detection(move_watcher[3]);
    console.log(move_results);

    static const Button BUTTON_INDEX[] = {
        BUTTON_X,
        BUTTON_Y,
        BUTTON_A,
        BUTTON_B,
        BUTTON_NONE,
    };
    static const char* STRING_INDEX[] = {
        "Picking Move: Top",
        "Picking Move: Left",
        "Picking Move: Right",
        "Picking Move: Bottom",
        "No moves available.",
    };

    int best_index = 4;
    for (int index = 0; index < 4; index++){
//        cout << "index: " << index << endl;
        if (move_watcher[index] == MoveEffectivenessSymbol::None){
//            cout << "skip due to none" << endl;
            continue;
        }
        if (m_settings.MODE == BattleAIMode::BlindMash){
            best_index = index;
            break;
        }
        if (best_index == 4 || move_watcher[best_index] < move_watcher[index]){
//            cout << "better move found: " << index << endl;
            best_index = index;
        }
    }

    console.log(STRING_INDEX[best_index], COLOR_BLUE);
    console.overlay().add_log(STRING_INDEX[best_index]);

    command.dispatch([&](ProControllerContext& context){
        ssf_press_button(context, BUTTON_ZL, 0ms, 800ms, 200ms);
        if (m_settings.USE_PLUS_MOVES){
            ssf_press_button(context, BUTTON_PLUS, 320ms, 840ms);
//            pbf_wait(context, 104ms);
        }
        pbf_press_button(context, BUTTON_INDEX[best_index], 160ms, 320ms);
    });

    command.wait();
    command.stop_session_and_rethrow();
    m_consecutive_failures = 0;
    return true;
}




void TrainerBattleState::run_lock_recovery(ConsoleHandle& console, ProControllerContext& context){
    console.log("Failed to lock on. Rotating camera...", COLOR_RED);

    ssf_press_right_joystick(context, {-1, 0},  0ms, 1000ms, 0ms);
    pbf_mash_button(context, BUTTON_ZL, 1000ms);
    context.wait_for_all_requests();
}


















}
}
}
