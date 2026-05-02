/*  Pokemon Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_Types_H
#define PokemonAutomation_Pokemon_Types_H

//#include <string>
#include "Common/Cpp/EnumStringMap.h"

namespace PokemonAutomation{
namespace Pokemon{


enum class MoveCategory{
    STATUS,
    PHYSICAL,
    SPECIAL,
    UNKNOWN,
};

enum class PokemonTeraType{
    BUG,
    DARK,
    DRAGON,
    ELECTRIC,
    FAIRY,
    FIGHTING,
    FIRE,
    FLYING,
    GHOST,
    GRASS,
    GROUND,
    ICE,
    NONE,
    NORMAL,
    POISON,
    PSYCHIC,
    ROCK,
    STEEL,
    STELLAR,
    WATER
};
const EnumStringMap<PokemonTeraType>& POKEMON_TERA_TYPE_SLUGS();

enum class PokemonType{
    NONE,
    NORMAL,
    FIRE,
    FIGHTING,
    WATER,
    FLYING,
    GRASS,
    POISON,
    ELECTRIC,
    GROUND,
    PSYCHIC,
    ROCK,
    ICE,
    BUG,
    DRAGON,
    GHOST,
    DARK,
    STEEL,
    FAIRY,
};
const EnumStringMap<PokemonType>& POKEMON_TYPE_SLUGS();


}
}
#endif
