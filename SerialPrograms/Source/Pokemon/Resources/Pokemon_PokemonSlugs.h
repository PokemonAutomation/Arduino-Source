/*  Pokemon Pokemon Slugs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonSlugs_H
#define PokemonAutomation_Pokemon_PokemonSlugs_H

#include <vector>
#include <string>
#include <set>

namespace PokemonAutomation{
namespace Pokemon{


const std::set<std::string>& ALL_POKEMON_SLUGS();
const std::vector<std::string>& NATIONAL_DEX_SLUGS();


}
}
#endif
