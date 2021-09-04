/*  Max Lair Run Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
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
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
){
    size_t console_index = console.index();
    GlobalState& state = state_tracker[console_index];
    size_t player_index = state.find_player_index(console_index);
    state.clear_battle_state();

    PathPartyReader reader(console);
    std::string mons[4];
    reader.read_sprites(console, console.video().snapshot(), mons);
    if (!mons[0].empty()) state.players[0].pokemon = mons[0];
    if (!mons[1].empty()) state.players[1].pokemon = mons[1];
    if (!mons[2].empty()) state.players[2].pokemon = mons[2];
    if (!mons[3].empty()) state.players[3].pokemon = mons[3];


    GlobalState inferred = state_tracker.synchronize(console_index);


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
