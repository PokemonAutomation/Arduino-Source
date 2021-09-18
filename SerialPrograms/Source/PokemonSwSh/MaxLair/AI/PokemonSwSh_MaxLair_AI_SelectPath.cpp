/*  Max Lair AI Select Path
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  0 for left-most path.
//  1 for 2nd from left.
//  2 ...
uint8_t select_path(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Choosing a path... State =>\n" + state.dump(),
        "purple"
    );

    //  Can't do anything since there's no path detection yet.
    return random(0, 5);
}



}
}
}
}
