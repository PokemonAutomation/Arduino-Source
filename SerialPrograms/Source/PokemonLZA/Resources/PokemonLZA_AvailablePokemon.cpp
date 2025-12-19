/*  Available Pokemon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonLZA_AvailablePokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



const std::vector<std::string>& LUMIOSE_DEX_SLUGS(){
    static const std::vector<std::string> database = Pokemon::load_pokemon_slug_json_list("Pokemon/Pokedex/Pokedex-Lumiose.json");
    return database;
}

const std::vector<std::string>& HYPERSPACE_DEX_SLUGS(){
    static const std::vector<std::string> database = Pokemon::load_pokemon_slug_json_list("Pokemon/Pokedex/Pokedex-Hyperspace.json");
    return database;
}



}
}
}
