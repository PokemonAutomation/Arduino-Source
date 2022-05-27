/*  Available Pokemon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonLA_AvailablePokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



const std::vector<std::string>& HISUI_DEX_SLUGS(){
    static const std::vector<std::string> database = Pokemon::load_pokemon_slug_json_list("Pokemon/Pokedex/Pokedex-Hisui.json");
    return database;
}

const std::vector<std::string>& HISUI_OUTBREAK_SLUGS(){
    static const std::vector<std::string> database = Pokemon::load_pokemon_slug_json_list("PokemonLA/OutbreakList.json");
    return database;
}


}
}
}
