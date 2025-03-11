/*  PkmnLib Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_PkmnLib_Types_H
#define _PokemonAutomation_PokemonSwSh_PkmnLib_Types_H

#include <string>
#include <map>
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{

enum class Type{
    normal = 0,
    fighting,
    flying,
    poison,
    ground,
    rock,
    bug,
    ghost,
    steel,
    fire,
    water,
    grass,
    electric,
    psychic,
    ice,
    dragon,
    dark,
    fairy,
    none
};

float damage_multiplier(Type attack, Type defense);
float damage_multiplier(Type attack, Type defense1, Type defense2);
Type get_type_from_string(const std::string& in_type);
const std::string& get_type_name(Type in_type);



Type serial_type_to_pkmnlib(Pokemon::PokemonType type);




}
}
}
}
#endif
