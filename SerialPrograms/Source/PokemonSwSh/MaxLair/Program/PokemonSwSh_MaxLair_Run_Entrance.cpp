/*  Max Lair Run Entrance
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Run_Entrance.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_entrance(
    MaxLairRuntime& runtime,
    ProgramEnvironment& env,
    ConsoleHandle& console, bool is_host,
    GlobalStateTracker& state_tracker
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];

    if (!state.adventure_started){
        console.log("Failed to start raid.", Qt::red);
        runtime.stats.add_error();
    }else if (state.wins == 0){
        console.log("Lost on first raid.", "purple");
        runtime.stats.add_run(0);
    }


    InferenceBoxScope box(console, 0.782, 0.850, 0.030, 0.050);

    pbf_wait(console, 2 * TICKS_PER_SECOND);
    while (true){
        if (is_host){
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        }else{
            pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
        }
        console.botbase().wait_for_all_requests();

        QImage screen = console.video().snapshot();
        ImageStats stats = image_stats(extract_box(screen, box));
        if (!is_grey(stats, 400, 1000)){
            break;
        }
    }
}



}
}
}
}
