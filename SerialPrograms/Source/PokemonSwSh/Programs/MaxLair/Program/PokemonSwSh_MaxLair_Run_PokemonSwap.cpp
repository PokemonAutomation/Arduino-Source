/*  Max Lair Run Pokemon Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh_MaxLair_Run_PokemonSwap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_swap_pokemon(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const MaxLairPlayerOptions& settings
){
    static std::mutex lock;

    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);
    PlayerState& player = state.players[player_index];
    state.clear_battle_state();


    //  Wait for bottom row to reload.
    pbf_wait(console, 50);
    console.botbase().wait_for_all_requests();

    PokemonSwapMenuReader reader(
        console,
        console.overlay(),
        settings.language
    );
    PokemonSwapMenuDetector menu(console, false);

    //  Now read the options.
    std::string options[2];
    QImage screen = console.video().snapshot();
    reader.read_options(screen, options);
    reader.read_pp(screen, player.pp);


    GlobalState inferred = state_tracker.synchronize(console_index);


    bool swap;
    {
//        console.log("Acquiring lock...");
        std::lock_guard<std::mutex> lg(lock);
//        console.log("Lock acquired...");


        //  Make your selection.
        swap = should_swap_pokemon(console, inferred, player_index, options);
        if (swap){
            console.log("Choosing to swap for: " + options[1], "purple");
        }else{
            console.log("Choosing not to swap.", "purple");
        }

        //  Update state.
        if (swap){
            pbf_mash_button(console, BUTTON_A, 3 * TICKS_PER_SECOND);
//            player.pokemon = options[1];
        }
        console.botbase().wait_for_all_requests();
    }
//    console.log("Lock released...");
    if (!swap){
        pbf_mash_button(console, BUTTON_B, 3 * TICKS_PER_SECOND);
//        player.pokemon = options[0];
        console.botbase().wait_for_all_requests();
    }

    //  Wait until we exit the window.
    InterruptableCommandSession commands(console);

    PokemonSwapMenuDetector detector(console, true);
    detector.register_command_stop(commands);

    AsyncVisualInferenceSession inference(env, console);
    inference += detector;

    commands.run([&](const BotBaseContext& context){
        pbf_wait(context, 480 * TICKS_PER_SECOND);
        context->wait_for_all_requests();
    });

    pbf_wait(console, 1 * TICKS_PER_SECOND);
}




}
}
}
}
