/*  Pokemon LA Icons
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLA_PokemonSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const SpriteDatabase& ALL_POKEMON_SPRITES(){
    static const SpriteDatabase database("PokemonLA/PokemonSprites.png", "PokemonLA/PokemonSprites.json");
    return database;
}


const SpriteDatabase& ALL_MMO_SPRITES(){
    static const SpriteDatabase database("PokemonLA/MMOSprites.png", "PokemonLA/MMOSprites.json");
    return database;
}


}
}
}
