/*  Max Lair Run Pokemon Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_Run_PokemonSwap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_swap_pokemon(
    AdventureRuntime& runtime,
    ConsoleHandle& console, BotBaseContext& context,
    GlobalStateTracker& state_tracker,
    const ConsoleSpecificOptions& settings
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);
    PlayerState& player = state.players[player_index];


    //  Wait for bottom row to reload.
    context.wait_for(std::chrono::milliseconds(100));


    PokemonSwapMenuReader reader(
        console,
        console.overlay(),
        settings.language
    );
    PokemonSwapMenuDetector menu(false);

    //  Now read the options.
    std::string options[2];
    VideoSnapshot screen = console.video().snapshot();
    reader.read_options(screen, options);
    reader.read_pp(screen, player.pp);

    //  Read HP of party.
    double hp[4];
    reader.read_hp(screen,hp);
    if (hp[0] >= 0) state.players[0].health = Health{hp[0], 0};
    if (hp[1] >= 0) state.players[1].health = Health{hp[1], 0};
    if (hp[2] >= 0) state.players[2].health = Health{hp[2], 0};
    if (hp[3] >= 0) state.players[3].health = Health{hp[3], 0};

    state.add_seen(options[0]);
    state.add_seen(options[1]);


    GlobalState inferred = state_tracker.synchronize(console, console_index);


    //  Make your selection.
    bool swap = should_swap_with_newly_caught(console, inferred, player_index, options);
    if (swap){
        console.log("Choosing to swap for: " + options[1], COLOR_PURPLE);
        std::lock_guard<std::mutex> lg(runtime.m_delay_lock);
        pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }else{
        console.log("Choosing not to swap.", COLOR_PURPLE);
        pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }

#if 1
    //  Wait until we exit the window.
    {
        BlackScreenWatcher detector;
        int result = run_until(
            console, context,
            [&](BotBaseContext& context){
                pbf_mash_button(context, swap ? BUTTON_A : BUTTON_B, 30 * TICKS_PER_SECOND);
            },
            {{detector}}
        );
        if (result < 0){
            console.log("Timed out waiting for black screen.", COLOR_RED);
        }else{
            console.log("Found path screen. Reading party...");
        }
    }
#endif
    {
        PathScreenDetector detector;
        int result = wait_until(
            console, context,
            std::chrono::seconds(30),
            {{detector}},
            INFERENCE_RATE
        );
        if (result < 0){
            console.log("Timed out waiting for path screen.", COLOR_RED);
        }else{
            console.log("Found path screen. Reading party...");
        }
    }

    context.wait_for(std::chrono::milliseconds(100));

    PathReader path_reader(console, player_index);
    auto snapshot = console.video().snapshot();
    path_reader.read_sprites(console, state, snapshot);
    path_reader.read_hp(console, state, snapshot);
}




}
}
}
}
