/*  Max Lair AI Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_AI_Tools_H
#define PokemonAutomation_PokemonSwSh_MaxLair_AI_Tools_H

#include <memory>
#include "CommonFramework/Logging/Logger.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


std::unique_ptr<papkmnlib::Pokemon> convert_player_to_pkmnlib(const PlayerState& player);


std::vector<const papkmnlib::Pokemon*> get_rental_candidates_on_path_pkmnlib(const GlobalState& state);
std::vector<const papkmnlib::Pokemon*> get_boss_candidates(const GlobalState& state);


double evaluate_hypothetical_team(
    const GlobalState& state,
    const papkmnlib::Pokemon* team[4],
    const std::vector<const papkmnlib::Pokemon*>& rental_candidates_on_path,
    const std::vector<const papkmnlib::Pokemon*>& boss_candidates_on_path
);



bool swap_if_least_hp(
    const GlobalState& state,
    size_t player_index
);





}
}
}
}
#endif
