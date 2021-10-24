/*  Pokemon Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_Types_H
#define PokemonAutomation_Pokemon_Types_H

#include <string>
#include <map>

namespace PokemonAutomation{
namespace Pokemon{


enum class MoveCategory{
    STATUS,
    PHYSICAL,
    SPECIAL,
    UNKNOWN,
};

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

extern const std::map<PokemonType, std::string> TYPE_ENUM_TO_SLUG;

const std::string& get_type_slug(PokemonType type);


}
}
#endif
