/*  Max Lair Run Pokemon Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
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


    //  Wait for bottom row to reload.
    env.wait_for(std::chrono::milliseconds(100));


    PokemonSwapMenuReader reader(
        console,
        console.overlay(),
        settings.language
    );
    PokemonSwapMenuDetector menu(false);

    //  Now read the options.
    std::string options[2];
    QImage screen = console.video().snapshot();
    reader.read_options(screen, options);
    reader.read_pp(screen, player.pp);

    //  Read HP of party.
    double hp[4];
    reader.read_hp(screen,hp);
    if (hp[0] >= 0) state.players[0].health = Health{hp[0], 0};
    if (hp[1] >= 0) state.players[1].health = Health{hp[1], 0};
    if (hp[2] >= 0) state.players[2].health = Health{hp[2], 0};
    if (hp[3] >= 0) state.players[3].health = Health{hp[3], 0};


    GlobalState inferred = state_tracker.synchronize(env, console, console_index);


    bool swap;
    {
//        console.log("Acquiring lock...");
        std::lock_guard<std::mutex> lg(lock);
//        console.log("Lock acquired...");


        //  Make your selection.
        swap = should_swap_with_newly_caught(console, inferred, player_index, options);
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
    BlackScreenDetector detector;
    wait_until(
        env, console,
        std::chrono::seconds(30),
        { &detector }
    );

    pbf_wait(console, 1 * TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    PathPartyReader path_reader(console, player_index);
    path_reader.read_sprites(console, console, state, console.video().snapshot());
    path_reader.read_hp(console, console, state, console.video().snapshot());
}




}
}
}
}
