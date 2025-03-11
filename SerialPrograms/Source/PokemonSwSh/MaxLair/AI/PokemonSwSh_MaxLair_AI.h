/*  Max Lair AI
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_AI_H
#define PokemonAutomation_PokemonSwSh_MaxLair_AI_H

#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


int random(int min, int max);



//  0 for top Pokemon
//  1 for middle Pokemon
//  2 for bottom Pokemon
int8_t select_starter(
    Logger& logger,
    const GlobalState& state,
    size_t player_index,
    const std::string options[3]
);

//  0 for left-most path.
//  1 for 2nd from left.
//  2 ...
std::vector<PathNode> select_path(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
);

//  Return -1 if no selecting an item.
int8_t select_item(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
);

//  Professor offers to exchange a Pokemon.
bool should_swap_with_professor(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
);

//  Return 0 - 3 for move position. 2nd parameter is dmax.
std::pair<uint8_t, bool> select_move(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
);

//  Swap with newly caught Pokemon.
bool should_swap_with_newly_caught(
    Logger& logger,
    const GlobalState& state,
    size_t player_index,
    const std::string options[2]
);




}
}
}
}
#endif
