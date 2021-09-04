/*  Max Lair AI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_AI_H
#define PokemonAutomation_PokemonSwSh_MaxLair_AI_H

#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  0 for top Pokemon
//  1 for middle Pokemon
//  2 for bottom Pokemon
int8_t select_starter(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index,
    const std::string options[3]
);

//  0 for left-most path.
//  1 for 2nd from left.
//  2 ...
uint8_t select_path(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
);

//  Return -1 if no selecting an item.
int8_t select_item(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
);

//  Professor offers to exchange a Pokemon.
bool should_swap_pokemon(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
);

//  Return 0 - 3 for move position. 2nd parameter is dmax.
std::pair<uint8_t, bool> select_move(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
);

//  Swap with newly caught Pokemon.
bool should_swap_pokemon(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index,
    const std::string options[2]
);


}
}
}
}
#endif
