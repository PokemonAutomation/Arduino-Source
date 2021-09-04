/*  Max Lair Run Pokemon Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh_MaxLair_Run_PokemonSelect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_select_pokemon(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker,
    const MaxLairPlayerOptions& settings
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    state.clear_battle_state();


    PokemonSelectMenuReader reader(
        console,
        console.overlay(),
        settings.language
    );

    //  Wait until it's your turn.
    auto start = std::chrono::system_clock::now();
    do{
        if (reader.my_turn(console.video().snapshot())){
            break;
        }
        if (std::chrono::system_clock::now() - start > std::chrono::seconds(120)){
            console.log("Still not your turn after 120 seconds.", Qt::red);
//            switch_ok = false;
            return;
        }
        env.wait(std::chrono::milliseconds(500));
    }while (true);
    console.log("Switch " + std::to_string(console.index()) + "'s turn to select.");

    std::string options[3];

    //  Wait for bottom row to reload.
    pbf_wait(console, 50);
    console.botbase().wait_for_all_requests();

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
    pbf_press_dpad(console, DPAD_DOWN, 10, 50);
    console.botbase().wait_for_all_requests();
    screen = console.video().snapshot();
    options[0] = reader.read_option(screen, 0);


    GlobalState inferred = state_tracker.synchronize(console_index);


    //  Make your selection.
    int8_t selection = select_starter(console, inferred, player_index, options);
    console.log("Choosing option " + std::to_string((int)selection) + ".", "purple");
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

    //  Wait until we exit the window.
    InterruptableCommandSession commands(console);

    PokemonSelectMenuDetector detector(console, true);
    detector.register_command_stop(commands);

    AsyncVisualInferenceSession inference(env, console);
    inference += detector;

    commands.run([&](const BotBaseContext& context){
        pbf_wait(context, 480 * TICKS_PER_SECOND);
        context->wait_for_all_requests();
    });

    pbf_wait(console, 5 * TICKS_PER_SECOND);
}





}
}
}
}
