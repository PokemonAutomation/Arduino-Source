/*  Available Pokemon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_AvailablePokemon_H
#define PokemonAutomation_PokemonLA_AvailablePokemon_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const std::vector<std::string>& HISUI_DEX_SLUGS();

// List of pokemon in Hisui dex order that appear in Massive Outbreak.
const std::vector<std::string>& HISUI_OUTBREAK_SLUGS();



}
}
}
#endif
