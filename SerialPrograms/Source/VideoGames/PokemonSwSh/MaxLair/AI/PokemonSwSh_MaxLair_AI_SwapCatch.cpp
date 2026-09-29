/*  Max Lair AI Swap with Newly Caught
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstddef>
#include "PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_AI_Tools.h"
#include "PokemonSwSh_MaxLair_AI_RentalBossMatchup.h"

#include <iostream>
using std::cout;
using std::endl;

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
        COLOR_PURPLE
    );

    using namespace papkmnlib;

    if (options[1].empty()){
        logger.log("Unable to read replacement candidate. Taking if you have the least HP.", COLOR_RED);
        return swap_if_least_hp(state, player_index);
    }


    std::vector<const Pokemon*> rental_candidates_on_path =
        get_rental_candidates_on_path_pkmnlib(state);

    std::vector<const Pokemon*> boss_candidates_on_path =
        get_boss_candidates(state);

    std::unique_ptr<Pokemon> current_team[4];
    current_team[0] = convert_player_to_pkmnlib(state.players[0]);
    current_team[1] = convert_player_to_pkmnlib(state.players[1]);
    current_team[2] = convert_player_to_pkmnlib(state.players[2]);
    current_team[3] = convert_player_to_pkmnlib(state.players[3]);

    std::multimap<double, int8_t, std::greater<double>> rank;
    for (int8_t c = -1; c < 4; c++){
        const Pokemon* hypothetical_team[4];
        hypothetical_team[0] = current_team[0].get();
        hypothetical_team[1] = current_team[1].get();
        hypothetical_team[2] = current_team[2].get();
        hypothetical_team[3] = current_team[3].get();
        if (c >= 0){
            hypothetical_team[c] = &get_pokemon(options[1]);
        }
        double score = evaluate_hypothetical_team(
            state,
            hypothetical_team,
            rental_candidates_on_path,
            boss_candidates_on_path
        );
        rank.emplace(score, c);
    }

    if (rank.empty()){
        logger.log("Unable to compute decisions. Taking if you have the least HP.", COLOR_RED);
        return swap_if_least_hp(state, player_index);
    }

    std::string dump = "Selection Score:\n";
    for (const auto& item : rank){
        dump += std::to_string(item.first) + " : ";
        if (item.second < 0){
            dump += "Nobody";
        }else{
            dump += "Player " + std::to_string(item.second);
        }
        dump += "\n";
    }
    logger.log(dump);

    return rank.begin()->second == (int8_t)player_index;
}



}
}
}
}
