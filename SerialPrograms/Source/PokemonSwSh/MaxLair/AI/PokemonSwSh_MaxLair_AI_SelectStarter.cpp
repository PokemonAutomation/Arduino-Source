/*  Max Lair AI Select Starter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Matchup.h"
#include "PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_AI_Tools.h"
#include "PokemonSwSh_MaxLair_AI_RentalBossMatchup.h"

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
        COLOR_PURPLE
    );

    using namespace papkmnlib;

    std::vector<const Pokemon*> bosses = get_boss_candidates(state);
    if (bosses.empty()){
        logger.log("Cannot pick a starter since there are no boss candidates.", COLOR_RED);
        return 0;
    }

    std::multimap<double, uint8_t, std::greater<double>> rank;
    for (uint8_t c = 0; c < 3; c++){
        if (options[c].empty()){
            continue;
        }
//        const Pokemon& rental = get_pokemon(options[c]);
        double score = 0;
        for (const Pokemon* boss : bosses){
//            score += evaluate_matchup(rental, *boss, {}, 4);
            score += rental_vs_boss_matchup(options[c], boss->name());
        }
        score /= bosses.size();
        rank.emplace(score, c);
    }
    if (rank.empty()){
        logger.log("Cannot pick a starter since none of the choices could be read.", COLOR_RED);
        return 0;
    }

    std::string dump = "Selection Score:\n";
    for (const auto& item : rank){
        dump += std::to_string(item.first) + " : " + options[item.second] + "\n";
    }
    logger.log(dump);

    return rank.begin()->second;
}



}
}
}
}
