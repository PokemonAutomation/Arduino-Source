/*  Pokemon Sword/Shield Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_PokemonSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const SpriteDatabase& ALL_POKEMON_SPRITES(){
    static const SpriteDatabase database("PokemonSwSh/PokemonSprites.png", "PokemonSwSh/PokemonSprites.json");
    return database;
}
const SpriteDatabase& ALL_POKEMON_SILHOUETTES(){
    static const SpriteDatabase database("PokemonSwSh/PokemonSilhouettes.png", "PokemonSwSh/PokemonSprites.json");
    return database;
}



}
}
}
