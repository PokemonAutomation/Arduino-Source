/*  Max Lair State Machine
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "Program/PokemonSwSh_MaxLair_Run_PokemonSelect.h"
#include "Program/PokemonSwSh_MaxLair_Run_PathSelect.h"
#include "Program/PokemonSwSh_MaxLair_Run_ItemSelect.h"
#include "Program/PokemonSwSh_MaxLair_Run_Battle.h"
#include "Program/PokemonSwSh_MaxLair_Run_PokemonSwap.h"
#include "PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


Action run_state_iteration(
    const MaxLairPlayerOptions& player_settings,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& global_state
){
    bool triggered_pokemon_select;
    bool triggered_path_select;
    bool triggered_item_menu;
    bool triggered_battle_menu;
    bool triggered_catch_select;
    bool triggered_pokemon_swap;
    bool triggered_caught_menu;
    {
        InterruptableCommandSession commands(console);

        PokemonSelectMenuDetector pokemon_select(console, false);
        pokemon_select.register_command_stop(commands);

        PathSelectDetector path_select(console);
        path_select.register_command_stop(commands);

        ItemSelectDetector item_menu(console, false);
        item_menu.register_command_stop(commands);

        BattleMenuDetector battle_menu(console);
        battle_menu.register_command_stop(commands);

        RaidCatchDetector catch_select(console);
        catch_select.register_command_stop(commands);

        PokemonSwapMenuDetector pokemon_swap(console, false);
        pokemon_swap.register_command_stop(commands);

        PokemonCaughtMenuDetector caught_menu(console);
        caught_menu.register_command_stop(commands);

        AsyncVisualInferenceSession inference(env, console);
        GlobalState state = global_state.infer_actual_state(console.index());
        if (state.wins == 0){
            inference += pokemon_select;
        }else{
            inference += pokemon_swap;
        }
        inference += path_select;
        inference += item_menu;
        inference += battle_menu;
        inference += catch_select;
        inference += caught_menu;

        commands.run([&](const BotBaseContext& context){
            pbf_wait(context, 120 * TICKS_PER_SECOND);
            context->wait_for_all_requests();
            context.check_cancelled();
            console.log("State timed out.", Qt::red);
        });

        triggered_pokemon_select    = pokemon_select.triggered();
        triggered_path_select       = path_select.triggered();
        triggered_item_menu         = item_menu.triggered();
        triggered_battle_menu       = battle_menu.triggered();
        triggered_catch_select      = catch_select.triggered();
        triggered_pokemon_swap      = pokemon_swap.triggered();
        triggered_caught_menu       = caught_menu.triggered();
    }

    if (triggered_pokemon_select){
        console.log("Current State: " + STRING_POKEMON + " Select");
        run_select_pokemon(env, console, global_state, player_settings);
        return Action::CONTINUE;
    }
    if (triggered_path_select){
        console.log("Current State: Path Select");
        run_path_select(console, global_state);
        return Action::CONTINUE;
    }
    if (triggered_item_menu){
        console.log("Current State: Item Select");
        run_item_select(env, console, global_state);
        return Action::CONTINUE;
    }
    if (triggered_battle_menu){
        console.log("Current State: Move Select");
        run_move_select(env, console, global_state, player_settings);
        return Action::CONTINUE;
    }
    if (triggered_catch_select){
        console.log("Current State: Catch Select");
        throw_balls(env, console, global_state, player_settings);
        return Action::CONTINUE;
    }
    if (triggered_pokemon_swap){
        console.log("Current State: " + STRING_POKEMON + " Swap");
        run_swap_pokemon(env, console, global_state, player_settings);
        return Action::CONTINUE;
    }
    if (triggered_caught_menu){
        console.log("Current State: Caught Menu");
        return Action::DONE;
    }

    return Action::DONE;
}




}
}
}
}
