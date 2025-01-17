/*  Max Lair Run Professor Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_Run_ProfessorSwap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_professor_swap(
    AdventureRuntime& runtime,
    ConsoleHandle& console, ControllerContext& context,
    GlobalStateTracker& state_tracker
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);

    PathReader reader(console, player_index);
    {
        VideoSnapshot screen = console.video().snapshot();
        reader.read_sprites(console, state, screen);
        reader.read_hp(console, state, screen);
    }


    GlobalState inferred = state_tracker.synchronize(console, console_index);


    bool swap = should_swap_with_professor(console, inferred, player_index);
    if (swap){
        console.log("Choosing to swap.", COLOR_PURPLE);
        std::lock_guard<std::mutex> lg(runtime.m_delay_lock);
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }else{
        console.log("Choosing not to swap.", COLOR_PURPLE);
        pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
    }
    context.wait_for_all_requests();

#if 1
    //  Wait until we exit the window.
    {
        BlackScreenWatcher detector;
        int result = run_until<ControllerContext>(
            console, context,
            [&](ControllerContext& context){
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

    {
        VideoSnapshot screen = console.video().snapshot();
        reader.read_sprites(console, state, screen);
        reader.read_hp(console, state, screen);
    }
}




}
}
}
}
