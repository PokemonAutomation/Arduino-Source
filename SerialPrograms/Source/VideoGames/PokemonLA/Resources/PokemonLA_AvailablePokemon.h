/*  Available Pokemon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_AvailablePokemon_H
#define PokemonAutomation_PokemonLA_AvailablePokemon_H

#include <string>
#include <vector>
#include <array>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const std::vector<std::string>& HISUI_DEX_SLUGS();

// List of pokemon in Hisui dex order that appear in Massive Outbreak.
const std::vector<std::string>& HISUI_OUTBREAK_SLUGS();

// List of pokemon sprite slugs in Hisui dex order that appear in first MMO waves.
const std::vector<std::string>& MMO_FIRST_WAVE_SPRITE_SLUGS();

// List of pokemon sprite slugs in Hisui dex order that appear in first MMO waves of each of the five regions.
const std::array<std::vector<std::string>, 5>& MMO_FIRST_WAVE_REGION_SPRITE_SLUGS();

}
}
}
#endif
