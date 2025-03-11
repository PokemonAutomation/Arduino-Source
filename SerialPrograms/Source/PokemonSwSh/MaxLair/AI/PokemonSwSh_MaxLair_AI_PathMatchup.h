/*  Max Lair AI Path Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_AI_PathMatchup_H
#define PokemonAutomation_PokemonSwSh_MaxLair_AI_PathMatchup_H

#include <vector>
#include <set>
#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Pokemon_Types.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


const std::set<std::string>& rentals_by_type(PokemonType type);
double type_vs_boss(PokemonType type, const std::string& boss_slug);
double type_vs_boss(PokemonType type, PokemonType boss_type);



std::vector<std::vector<PathNode>> generate_paths(
    const PathMap& map, uint8_t wins, int8_t side
);


std::vector<PathNode> select_path(
    Logger* logger,
    const std::string& boss,
    const PathMap& pathmap, uint8_t wins, int8_t path_side
);



}
}
}
}
#endif
