/*  Pokemon LA Icons
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon/Resources/Pokemon_PokemonIcons.h"
#include "PokemonLA_PokemonIcons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const std::map<std::string, QIcon>& ALL_POKEMON_ICONS(){
    static std::map<std::string, QIcon> icons = Pokemon::load_pokemon_icons("PokemonLA/PokemonSprites.png", "PokemonLA/PokemonSprites.json");
    return icons;
}

const std::map<std::string, QIcon>& ALL_MMO_POKEMON_ICONS(){
    static std::map<std::string, QIcon> icons = Pokemon::load_pokemon_icons("PokemonLA/MMOSprites.png", "PokemonLA/MMOSprites.json");
    return icons;
}


}
}
}
