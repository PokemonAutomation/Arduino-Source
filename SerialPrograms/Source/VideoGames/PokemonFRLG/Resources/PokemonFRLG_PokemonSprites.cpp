/*  Pokemon FRLG Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_PokemonSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


const SpriteDatabase& ALL_POKEMON_SPRITES(){
    static const SpriteDatabase database("PokemonFRLG/PokemonSprites.png", "PokemonFRLG/PokemonSprites.json");
    return database;
}
const SpriteDatabase& FLIPPED_POKEMON_SPRITES(){
    static const SpriteDatabase database("PokemonFRLG/PokemonSpritesFlipped.png", "PokemonFRLG/PokemonSprites.json");
    return database;
}



}
}
}
