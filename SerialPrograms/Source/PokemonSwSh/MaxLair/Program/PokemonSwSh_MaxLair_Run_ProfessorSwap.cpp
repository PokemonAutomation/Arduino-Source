/*  Max Lair Run Professor Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh_MaxLair_Run_ProfessorSwap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_professor_swap(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);

    PathPartyReader reader(console, player_index);
    reader.read_sprites(console, console, state, console.video().snapshot());
    reader.read_hp(console, console, state, console.video().snapshot());


    GlobalState inferred = state_tracker.synchronize(env, console, console_index);


    bool swap = should_swap_with_professor(console, inferred, player_index);

    if (swap){
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.log("Choosing to swap.", "purple");
    }else{
        pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
        console.log("Choosing not to swap.", "purple");
    }
    console.botbase().wait_for_all_requests();

    //  Wait until we exit the window.
    BlackScreenDetector detector;
    int result = wait_until(
        env, console,
        std::chrono::seconds(30),
        { &detector }
    );
    if (result < 0){
        console.log("Timed out waiting for black screen.", Qt::red);
    }

    pbf_wait(console, 1 * TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

    reader.read_sprites(console, console, state, console.video().snapshot());
    reader.read_hp(console, console, state, console.video().snapshot());
}




}
}
}
}
