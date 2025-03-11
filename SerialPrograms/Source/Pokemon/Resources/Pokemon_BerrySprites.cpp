/*  Pokemon Berry Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_BerrySprites.h"

namespace PokemonAutomation{
namespace Pokemon{



const SpriteDatabase& ALL_BERRY_SPRITES(){
    static const SpriteDatabase database("Pokemon/BerrySprites.png", "Pokemon/BerrySprites.json");
    return database;
}




}
}
