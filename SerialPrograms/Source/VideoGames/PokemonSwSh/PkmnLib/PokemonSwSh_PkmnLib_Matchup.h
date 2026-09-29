/*  PkmnLib Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_PkmnLib_Matchup_H
#define _PokemonAutomation_PokemonSwSh_PkmnLib_Matchup_H

#include <vector>
#include "PokemonSwSh_PkmnLib_Field.h"
#include "PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh_PkmnLib_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


double calc_average_damage(
    const std::vector<const Pokemon*>& attackers,
    const std::vector<const Pokemon*>& defenders,
    const Field& field, bool multipleTargets
);
double calc_move_score(
    const Pokemon& attacker, Pokemon defender,
    const std::vector<const Pokemon*>& teammates,
    size_t moveIdx, const Field& field
);
void select_best_move(
    const Pokemon& attacker, const Pokemon& defender, const Field& field,
    const std::vector<const Pokemon*>& teammates,
    size_t& bestIndex, std::string& bestMoveName, double& bestMoveScore
);
double evaluate_matchup(
    Pokemon attacker, const Pokemon& boss,
    const std::vector<const Pokemon*>& teammates,
    uint8_t numLives
);
double evaluate_average_matchup(
    const Pokemon& attacker, const std::vector<const Pokemon*>& bosses,
    const std::vector<const Pokemon*>& teammates, uint8_t numLives
);
double get_weighted_score(double rentalScore, double rentalWeight, double bossScore, double bossWeight);



}
}
}
}
#endif
