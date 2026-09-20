/*  Max Lair AI Rental/Boss Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_AI_RentalBossMatchup_H
#define PokemonAutomation_PokemonSwSh_MaxLair_AI_RentalBossMatchup_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


double rental_vs_boss_matchup(const std::string& rental, const std::string& boss);
double rental_vs_boss_matchup(const std::string& rental, const std::vector<std::string>& bosses);



}
}
}
}
#endif
