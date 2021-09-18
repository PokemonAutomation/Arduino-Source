/*  Max Lair AI Select Starter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  0 for top Pokemon
//  1 for middle Pokemon
//  2 for bottom Pokemon
int8_t select_starter(
    Logger& logger,
    const GlobalState& state,
    size_t player_index,
    const std::string options[3]
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Choosing a starter... State =>\n" + state.dump(),
        "purple"
    );

    return random(0, 2);
}



}
}
}
}
