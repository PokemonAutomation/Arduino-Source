/*  Max Lair AI Swap with Professor
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  Professor offers to exchange a Pokemon.
bool should_swap_with_professor(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Deciding whether to swap with professor... State =>\n" + state.dump(),
        "purple"
    );


    double pc_least_hp = 1.0;
    int pc_player = -1;

    double npc_least_hp = 1.0;
    int npc_player = -1;

    for (int c = 0; c < 4; c++){
        double hp = state.players[c].health.value.hp;
        if (hp < 0){
            continue;
        }

        if (state.players[c].console_id < 0){
            //  NPC
            if (npc_least_hp > hp){
                npc_least_hp = hp;
                npc_player = c;
            }
        }else{
            //  Player Character
            if (pc_least_hp > hp){
                pc_least_hp = hp;
                pc_player = c;
            }
        }
    }

    //  Player character should take it. Take it if you have the least HP.
    if (pc_player >= 0 && pc_least_hp < 0.5){
        return player_index == (size_t)pc_player;
    }

    //  And NPC needs it. Don't take it and hope they do.
    if (npc_player >= 0 && npc_least_hp < 0.5){
        return false;
    }

    //  Nobody has less than 50%. Take it if you have the least HP.
    return player_index == (size_t)pc_player;
}



}
}
}
}
