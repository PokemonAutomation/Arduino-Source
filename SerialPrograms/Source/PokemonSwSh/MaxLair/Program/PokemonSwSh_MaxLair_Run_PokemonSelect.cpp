/*  Max Lair Run Pokemon Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh_MaxLair_Run_PokemonSelect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_select_pokemon(
    size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker,
    OcrFailureWatchdog& ocr_watchdog,
    const ConsoleSpecificOptions& settings
){
    GlobalState& state = state_tracker[console_index];

    state.adventure_started = true;

    stream.log("Switch " + std::to_string(console_index) + "'s turn to select.");

    //  Wait for the screen to finish loading.
    context.wait_for(std::chrono::milliseconds(500));


    PokemonSelectMenuReader reader(
        stream.logger(),
        stream.overlay(),
        settings.language,
        ocr_watchdog
    );


    std::string options[3];

//    //  Wait for bottom row to reload.
//    pbf_wait(context, 50);
//    context.wait_for_all_requests();

    //  Read the bottom two options first.
    VideoSnapshot screen = stream.video().snapshot();
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
    pbf_press_dpad(context, DPAD_DOWN, 10, 80);
    context.wait_for_all_requests();
    screen = stream.video().snapshot();
    options[0] = reader.read_option(screen, 0);

    state.add_seen(options[0]);
    state.add_seen(options[1]);
    state.add_seen(options[2]);


//    GlobalState inferred = state_tracker.synchronize(env, console, console_index);
    state_tracker.push_update(console_index);
    GlobalState inferred = state_tracker.infer_actual_state(console_index);


    //  Make your selection.
    int8_t selection = select_starter(stream.logger(), inferred, player_index, options);
    stream.log("Choosing option " + std::to_string((int)selection) + ".", COLOR_PURPLE);
    switch (selection){
    case 0:
        pbf_press_dpad(context, DPAD_UP, 10, 50);
        break;
    case 1:
        break;
    case 2:
        pbf_press_dpad(context, DPAD_DOWN, 10, 50);
        break;
    }
    pbf_press_button(context, BUTTON_A, 10, 50);
    context.wait_for_all_requests();

    //  Update state.
    if (player_index >= 0){
        state.players[console_index].pokemon = std::move(options[selection]);
    }


}





}
}
}
}
