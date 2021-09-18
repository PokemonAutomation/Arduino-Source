/*  Max Lair State Machine
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ProfessorSwap.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSelect.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PathSelect.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ItemSelect.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ProfessorSwap.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Battle.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSwap.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_CaughtScreen.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Entrance.h"
#include "PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


StateMachineAction run_state_iteration(
    MaxLairRuntime& runtime, size_t index,
    ProgramEnvironment& env,
    ConsoleHandle& console, bool is_host,
    GlobalStateTracker& global_state,
    const QImage& entrance
){
    GlobalState state = global_state.infer_actual_state(console.index());
    uint8_t wins = state.wins;

    PokemonSelectMenuDetector pokemon_select(false);
    PokemonSwapMenuDetector pokemon_swap(false);
    PathSelectDetector path_select;
    ItemSelectDetector item_menu(false);
    ProfessorSwapDetector professor_swap(console);
    BattleMenuDetector battle_menu;
    RaidCatchDetector catch_select;
    PokemonCaughtMenuDetector caught_menu;
    EntranceDetector entrance_detector(entrance);

    int result = wait_until(
        env, console,
        std::chrono::seconds(480),
        {
            wins == 0
                ? (VisualInferenceCallback*)&pokemon_select
                : (VisualInferenceCallback*)&pokemon_swap,
            &path_select,
            &item_menu,
            wins == 0 ? nullptr : &professor_swap,
            &battle_menu,
            &catch_select,
            &caught_menu,
            &entrance_detector,
        }
    );

    switch (result){
    case 0:
        if (wins == 0){
            console.log("Current State: " + STRING_POKEMON + " Select");
            run_select_pokemon(env, console, global_state, *runtime.player_settings[index]);
            return StateMachineAction::KEEP_GOING;
        }else{
            console.log("Current State: " + STRING_POKEMON + " Swap");
            run_swap_pokemon(env, console, global_state, *runtime.player_settings[index]);
            return StateMachineAction::KEEP_GOING;
        }
    case 1:
        console.log("Current State: Path Select");
        run_path_select(env, console, global_state);
        return StateMachineAction::KEEP_GOING;
    case 2:
        console.log("Current State: Item Select");
        run_item_select(env, console, global_state);
        return StateMachineAction::KEEP_GOING;
    case 3:
        console.log("Current State: Professor Swap");
        run_professor_swap(env, console, global_state);
        return StateMachineAction::KEEP_GOING;
    case 4:
        console.log("Current State: Move Select");
        run_move_select(
            env, console, global_state,
            *runtime.player_settings[index],
            battle_menu.dmaxed(),
            battle_menu.cheer()
        );
        return StateMachineAction::KEEP_GOING;
    case 5:
        console.log("Current State: Catch Select");
        return throw_balls(runtime, env, console, global_state, *runtime.player_settings[index]);
    case 6:
        console.log("Current State: Caught Menu");
        return run_caught_screen(runtime, env, console, is_host, entrance);
    case 7:
        console.log("Current State: Entrance");
        run_entrance(runtime, env, console, is_host, global_state);
        return StateMachineAction::DONE_WITH_ADVENTURE;
    default:
        global_state.mark_as_dead(index);
        console.log("Program hang. No state detected after 8 minutes. Resetting game...", Qt::red);
        runtime.stats.add_error();
        reset_game_from_home_with_inference(env, console, true);
        return StateMachineAction::DONE_WITH_ADVENTURE;
    }


//    PA_THROW_StringException("Program hang. No state detected after 8 minutes.");
}




}
}
}
}
