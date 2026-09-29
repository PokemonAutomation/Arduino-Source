/*  Pokemon Home Pokeball Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonHome_PokeballSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


const SpriteDatabase& ALL_POKEBALL_SPRITES(){
    static const SpriteDatabase database("PokemonHome/PokeballSprites.png", "PokemonHome/PokeballSprites.json");
    return database;
}



}
}
}
