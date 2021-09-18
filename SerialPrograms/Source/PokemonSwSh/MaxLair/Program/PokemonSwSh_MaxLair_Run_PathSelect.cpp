/*  Max Lair Run Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh_MaxLair_Run_ItemSelect.h"
#include "PokemonSwSh_MaxLair_Run_PathSelect.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_path_select(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);

    PathPartyReader reader(console, player_index);
    env.wait_for(std::chrono::milliseconds(500));

    reader.read_sprites(console, console, state, console.video().snapshot());
    reader.read_hp(console, console, state, console.video().snapshot());

    if (state.wins == 0){
        Path path;
        if (read_path(path, console)){
            console.log("Path Detection:\n" + path.dump());
            state.path = path;
        }else{
            console.log("Path Detection: Failed", Qt::red);
        }
    }


    GlobalState inferred = state_tracker.synchronize(env, console, console_index);


    //  Select the path.
    uint8_t path = select_path(console, inferred, player_index);
    console.log("Choosing path " + std::to_string((int)path) + ".", "purple");

    for (uint8_t c = 0; c < path; c++){
        pbf_press_dpad(console, DPAD_RIGHT, 10, 50);
    }
    pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();
}



}
}
}
}
