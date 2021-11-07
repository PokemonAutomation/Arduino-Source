/*  Max Lair Item
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_Run_ItemSelect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_item_select(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);

    PathReader reader(console, player_index);
    {
        QImage screen = console.video().snapshot();
        reader.read_sprites(console, state, screen);
        reader.read_hp(console, state, screen);
    }


    GlobalState inferred = state_tracker.synchronize(env, console, console_index);


    int8_t item_index = select_item(console, inferred, player_index);
    console.log("Choosing item " + std::to_string((int)item_index) + ".", "purple");

    if (item_index < 0){
        pbf_press_button(console, BUTTON_B, 10, 50);
        return;
    }
    for (int8_t c = 0; c < item_index; c++){
        pbf_press_dpad(console, DPAD_DOWN, 10, 50);
    }
    pbf_press_button(console, BUTTON_A, 10, 5 * TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    //  Wait until we exit the window.
    ItemSelectDetector item_menu(true);
    wait_until(
        env, console,
        std::chrono::seconds(480),
        { &item_menu },
        INFERENCE_RATE
    );

    pbf_wait(console, 1 * TICKS_PER_SECOND);
}



}
}
}
}
