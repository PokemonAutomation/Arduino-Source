/*  Max Lair Run Pokemon Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh_MaxLair_Run_PokemonSelect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_select_pokemon(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const ConsoleSpecificOptions& settings
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];

    state.adventure_started = true;

    console.log("Switch " + std::to_string(console.index()) + "'s turn to select.");

    //  Wait for the screen to finish loading.
    env.wait_for(std::chrono::milliseconds(500));


    PokemonSelectMenuReader reader(
        console,
        console.overlay(),
        settings.language
    );


    std::string options[3];

//    //  Wait for bottom row to reload.
//    pbf_wait(console, 50);
//    console.botbase().wait_for_all_requests();

    //  Read the bottom two options first.
    QImage screen = console.video().snapshot();
    options[1] = reader.read_option(screen, 1);
    options[2] = reader.read_option(screen, 2);

//    reader.read_options(screen, options);
    int8_t player_index = reader.who_is_selecting(screen);
    if (player_index >= 0){
        state.players[player_index].console_id = (int8_t)console_index;
    }else{
        player_index = 0;
    }

    //  Scroll down one to move the arrow off the top row. Then we can read it.
    pbf_press_dpad(console, DPAD_DOWN, 10, 80);
    console.botbase().wait_for_all_requests();
    screen = console.video().snapshot();
    options[0] = reader.read_option(screen, 0);

    state.add_seen(options[0]);
    state.add_seen(options[1]);
    state.add_seen(options[2]);


//    GlobalState inferred = state_tracker.synchronize(env, console, console_index);
    state_tracker.push_update(console_index);
    GlobalState inferred = state_tracker.infer_actual_state(console_index);


    //  Make your selection.
    int8_t selection = select_starter(console, inferred, player_index, options);
    console.log("Choosing option " + std::to_string((int)selection) + ".", COLOR_PURPLE);
    switch (selection){
    case 0:
        pbf_press_dpad(console, DPAD_UP, 10, 50);
        break;
    case 1:
        break;
    case 2:
        pbf_press_dpad(console, DPAD_DOWN, 10, 50);
        break;
    }
    pbf_press_button(console, BUTTON_A, 10, 50);
    console.botbase().wait_for_all_requests();

    //  Update state.
    if (player_index >= 0){
        state.players[console_index].pokemon = std::move(options[selection]);
    }


}





}
}
}
}
