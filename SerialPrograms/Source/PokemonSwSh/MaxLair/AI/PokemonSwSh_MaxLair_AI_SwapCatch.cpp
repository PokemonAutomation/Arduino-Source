/*  Max Lair AI Swap with Newly Caught
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  Swap with newly caught Pokemon.
bool should_swap_with_newly_caught(
    Logger& logger,
    const GlobalState& state,
    size_t player_index,
    const std::string options[2]
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Deciding whether to swap with newly caught... State =>\n" + state.dump(),
        "purple"
    );

//    return random(0, 4) == 0;
    return should_swap_with_professor(logger, state, player_index);
}



}
}
}
}
