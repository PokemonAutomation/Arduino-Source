/*  Max Lair AI Select Item
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstddef>
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  Return -1 if no selecting an item.
int8_t select_item(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Selecting an item... State =>\n" + state.dump(),
        COLOR_PURPLE
    );

    //  Can't do anything since there's no item detection yet.
//    return random(0, 4);
    return 0;
}



}
}
}
}
