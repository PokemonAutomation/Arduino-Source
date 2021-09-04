/*  Max Lair AI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



int8_t select_starter(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index,
    const std::string options[3]
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Choosing a starter... State =>\n" + global_state.dump(),
        "purple"
    );

    return 2;
}

uint8_t select_path(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Choosing a path... State =>\n" + global_state.dump(),
        "purple"
    );

    //  Can't do anything since there's no path detection yet.
    return 1;
}

int8_t select_item(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Selecting an item... State =>\n" + global_state.dump(),
        "purple"
    );

    //  Can't do anything since there's no item detection yet.
    return 10;
}

//  Professor offers to exchange a Pokemon.
bool should_swap_pokemon(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Deciding whether to swap with professor... State =>\n" + global_state.dump(),
        "purple"
    );

    return false;
}

std::pair<uint8_t, bool> select_move(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Selecting a move... State =>\n" + global_state.dump(),
        "purple"
    );

    const PlayerState& player = global_state.players[player_index];

    if (!player.move_blocked[2]){
        return {2, false};
    }
    if (!player.move_blocked[0]){
        return {0, false};
    }
    if (!player.move_blocked[1]){
        return {1, false};
    }
    if (!player.move_blocked[3]){
        return {3, false};
    }

    return {0, false};
}

bool should_swap_pokemon(
    Logger& logger,
    const GlobalState& global_state,
    size_t player_index,
    const std::string options[2]
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Deciding whether to swap with newly caught... State =>\n" + global_state.dump(),
        "purple"
    );

    return false;
}


}
}
}
}
