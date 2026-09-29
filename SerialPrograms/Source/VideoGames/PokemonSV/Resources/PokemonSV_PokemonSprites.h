/*  Pokemon Scarlet/Violet Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokemonSprites_H
#define PokemonAutomation_PokemonSV_PokemonSprites_H

#include <array>
#include "CommonTools/Resources/SpriteDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const SpriteDatabase& ALL_POKEMON_SPRITES();

const SpriteDatabase& ALL_POKEMON_SILHOUETTES();

enum class TeraType{
    Bug,
    Dark,
    Dragon,
    Electric,
    Fairy,
    Fighting,
    Fire,
    Flying,
    Ghost,
    Grass,
    Ground,
    Ice,
    Normal,
    Poison,
    Psychic,
    Rock,
    Steel,
    Water,
};
constexpr size_t NUM_TERA_TYPE = 18;
extern const std::array<std::string, NUM_TERA_TYPE> TERA_TYPE_NAMES;
const std::array<ImageRGB32, NUM_TERA_TYPE>& ALL_TERA_TYPE_ICONS();


}
}
}
#endif
