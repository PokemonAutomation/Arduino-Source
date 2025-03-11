/*  Max Lair Run Professor Swap
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mutex>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
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
    size_t console_index,
    AdventureRuntime& runtime,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker
){
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);

    PathReader reader(stream.overlay(), player_index);
    {
        VideoSnapshot screen = stream.video().snapshot();
        reader.read_sprites(stream.logger(), state, screen);
        reader.read_hp(stream.logger(), state, screen);
    }


    GlobalState inferred = state_tracker.synchronize(stream.logger(), console_index);


    bool swap = should_swap_with_professor(stream.logger(), inferred, player_index);
    if (swap){
        stream.log("Choosing to swap.", COLOR_PURPLE);
        std::lock_guard<std::mutex> lg(runtime.m_delay_lock);
        pbf_mash_button(context, BUTTON_A, 1000ms);
        context.wait_for_all_requests();
    }else{
        stream.log("Choosing not to swap.", COLOR_PURPLE);
        pbf_mash_button(context, BUTTON_B, 1000ms);
    }
    context.wait_for_all_requests();


    //  Wait until we exit the window.
    {
        BlackScreenWatcher detector;
        int result = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {detector}
        );
        if (result < 0){
            stream.log("Timed out waiting for black screen.", COLOR_RED);
            return;
        }
    }

    {
        PathScreenDetector detector;
        int result = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {detector},
            INFERENCE_RATE
        );
        if (result < 0){
            stream.log("Timed out waiting for path screen.", COLOR_RED);
            return;
        }
    }

    stream.log("Found path screen. Reading party...");
    context.wait_for(std::chrono::milliseconds(100));

    VideoSnapshot screen = stream.video().snapshot();
    reader.read_sprites(stream.logger(), state, screen);
    reader.read_hp(stream.logger(), state, screen);
}




}
}
}
}
