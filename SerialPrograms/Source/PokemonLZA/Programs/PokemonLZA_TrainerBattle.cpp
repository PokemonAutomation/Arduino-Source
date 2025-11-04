/*  Trainer Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

// #include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLZA/Inference/PokemonLZA_MoveEffectivenessSymbol.h"
#include "PokemonLZA_TrainerBattle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


bool attempt_one_attack(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    bool use_move_ai, bool use_plus_moves, bool allow_button_B_press
){
    if (!use_move_ai){
        ssf_press_button(context, BUTTON_ZL, 160ms, 800ms, 200ms);
        if (use_plus_moves){
            ssf_press_button(context, BUTTON_PLUS, 320ms, 840ms);
//          pbf_wait(context, 104ms);
        }
        pbf_press_button(context, BUTTON_X, 80ms, 24ms);
        pbf_press_button(context, BUTTON_Y, 80ms, 24ms);
        if (allow_button_B_press){
            pbf_press_button(context, BUTTON_B, 80ms, 24ms);
        }
        return true;
    }

    AsyncCommandSession<ProController> command(
        context,
        env.logger(),
        env.realtime_dispatcher(),
        context
    );

    MoveEffectivenessSymbolWatcher move_watcher(COLOR_RED, &env.console.overlay(), 100ms);
    command.dispatch([](ProControllerContext& context){
        pbf_press_button(context, BUTTON_ZL, 10000ms, 0ms);
    });

    int ret = wait_until(
        env.console, context, 1000ms,
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
        return false;
    }

    MoveEffectivenessSymbol best_type = move_watcher[0];
    Button best_move = BUTTON_X;
    const char* best_string = "Picking Move: Top";
    if (best_type < move_watcher[1]){
        best_type = move_watcher[1];
        best_move = BUTTON_Y;
        best_string = "Picking Move: Left";
    }
    if (best_type < move_watcher[2]){
        best_type = move_watcher[2];
        best_move = BUTTON_A;
        best_string = "Picking Move: Right";
    }
    if (best_type < move_watcher[3]){
//        best_type = move_watcher[3];
        best_move = BUTTON_B;
        best_string = "Picking Move: Bottom";
    }

    env.log(best_string, COLOR_BLUE);
    env.console.overlay().add_log(best_string);

    command.dispatch([&](ProControllerContext& context){
        ssf_press_button(context, BUTTON_ZL, 0ms, 800ms, 200ms);
        if (use_plus_moves){
            ssf_press_button(context, BUTTON_PLUS, 320ms, 840ms);
//            pbf_wait(context, 104ms);
        }
        pbf_press_button(context, best_move, 160ms, 320ms);
    });

    command.wait();

    command.stop_session_and_rethrow();
    return true;
}


}
}
}
