/*  Pokemon Sword/Shield Pokeball Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_PokeballSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const SpriteDatabase& ALL_POKEBALL_SPRITES(){
    static const SpriteDatabase database("PokemonSwSh/PokeballSprites.png", "PokemonSwSh/PokeballSprites.json");
    return database;
}



}
}
}
