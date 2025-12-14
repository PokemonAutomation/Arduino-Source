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


// Get a set of all pokemon slugs.
// Each slug corresponds to one natioanl dex number.
const std::set<std::string>& ALL_POKEMON_SLUGS();
// Get a list of pokemon slugs in the order of national dex.
// Each slug corresponds to one natioanl dex number.
const std::vector<std::string>& NATIONAL_DEX_SLUGS();
// Get a one-to-one mapping from pokemon slugs to their national dex numbers.
const std::map<std::string, size_t>& SLUGS_TO_NATIONAL_DEX();


}
}
#endif
