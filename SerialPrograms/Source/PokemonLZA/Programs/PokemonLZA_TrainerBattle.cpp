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
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonLZA/Inference/Battles/PokemonLZA_MoveEffectivenessSymbol.h"
#include "PokemonLZA_TrainerBattle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


TrainerBattleState::TrainerBattleState()
    : m_consecutive_failures(0)
{}




bool TrainerBattleState::attempt_one_attack(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    ProControllerContext& context,
    bool use_move_ai,
    bool use_plus_moves,
    bool allow_button_B_press
){
    AsyncCommandSession<ProController> command(
        context,
        env.logger(),
        env.realtime_dispatcher(),
        context
    );

    MoveEffectivenessSymbolWatcher move_watcher(COLOR_RED, &console.overlay(), 20ms);
    command.dispatch([](ProControllerContext& context){
        pbf_press_button(context, BUTTON_ZL, 5000ms, 0ms);
    });

    env.log("Begin looking for type symbols.");

    int ret = wait_until(
        console, context, 1000ms,
        {move_watcher}
    );
    if (ret < 0){
        command.stop_session_and_rethrow();
        context.wait_for(250ms);
        // No move effectiveness symbol found
        if (allow_button_B_press){
            // It could be the game is in a transparent pre-battle dialog,
            // press B to clear it.
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
        }
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            run_lock_recovery(console, context);
        }
        return false;
    }

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
        if (move_watcher[index] == MoveEffectivenessSymbol::None){
            continue;
        }
        if (!use_move_ai){
            best_index = index;
            break;
        }
        if (move_watcher[best_index] < move_watcher[index]){
            best_index = index;
        }
    }

    env.log(STRING_INDEX[best_index], COLOR_BLUE);
    console.overlay().add_log(STRING_INDEX[best_index]);

    command.dispatch([&](ProControllerContext& context){
        ssf_press_button(context, BUTTON_ZL, 0ms, 800ms, 200ms);
        if (use_plus_moves){
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

    ssf_press_right_joystick(context, 0, 128, 0ms, 1000ms, 0ms);
    pbf_mash_button(context, BUTTON_ZL, 1000ms);
}


















}
}
}
