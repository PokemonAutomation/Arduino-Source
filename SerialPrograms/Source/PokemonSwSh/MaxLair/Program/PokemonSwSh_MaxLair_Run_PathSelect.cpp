/*  Max Lair Run Path Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_Run_PathSelect.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_path_select(
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker
){
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);

    PathReader reader(stream.overlay(), player_index);
    context.wait_for(std::chrono::milliseconds(500));

    VideoSnapshot screen = stream.video().snapshot();
    reader.read_sprites(stream.logger(), state, screen);
    reader.read_hp(stream.logger(), state, screen);

    if (state.wins == 0){
        reader.read_path(env, stream, context, state);
    }else{
        reader.read_side(stream.logger(), state, screen);
    }


    GlobalState inferred = state_tracker.synchronize(stream.logger(), console_index);


    //  Select the path.
    std::vector<PathNode> path = select_path(stream.logger(), inferred, player_index);
    uint8_t slot;
    if (path.empty()){
        stream.log("No available paths due to read errors. Picking left-most path.", COLOR_RED);
        slot = 0;
    }else{
        slot = path[0].path_slot;
    }
    state.last_best_path = std::move(path);

    stream.log("Choosing path " + std::to_string((int)slot) + ".", COLOR_PURPLE);

    for (uint8_t c = 0; c < slot; c++){
        pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    }
    pbf_mash_button(context, BUTTON_A, 1000ms);
    pbf_wait(context, 5000ms);
    context.wait_for_all_requests();
}



}
}
}
}
