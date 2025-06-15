/*  Pokemon Home Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_PokemonHomeSprites.h"

namespace PokemonAutomation{
namespace Pokemon{



const SpriteDatabase& ALL_POKEMON_HOME_SPRITES(){
    static const SpriteDatabase database("Pokemon/AllHomeSprites.png", "Pokemon/AllHomeSprites.json");
    return database;
}




}
}
