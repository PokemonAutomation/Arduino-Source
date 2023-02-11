/*  Pokemon Scarlet/Violet Sandwich Sprites
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV_SandwichSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



const SpriteDatabase& ALL_SANDWICH_FILLINGS() {
    static const SpriteDatabase database("PokemonSV/Picnic/SandwichFillingSprites.png", "PokemonSV/Picnic/SandwichFillingSprites.json");
    return database;
}

const SpriteDatabase& ALL_SANDWICH_CONDIMENTS() {
    static const SpriteDatabase database("PokemonSV/Picnic/SandwichCondimentSprites.png", "PokemonSV/Picnic/SandwichCondimentSprites.json");
    return database;
}



}
}
}
