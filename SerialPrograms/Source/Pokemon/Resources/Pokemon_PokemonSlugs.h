/*  Pokemon Pokemon Slugs
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonSlugs_H
#define PokemonAutomation_Pokemon_PokemonSlugs_H

#include <vector>
#include <string>
#include <set>
#include <map>

namespace PokemonAutomation{
namespace Pokemon{


const std::set<std::string>& ALL_POKEMON_SLUGS();
const std::vector<std::string>& NATIONAL_DEX_SLUGS();
const std::map<std::string, size_t>& SLUGS_TO_NATIONAL_DEX();


}
}
#endif
