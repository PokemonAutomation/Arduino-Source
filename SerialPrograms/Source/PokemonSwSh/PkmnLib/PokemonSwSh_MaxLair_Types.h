/*  Max Lair Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_MaxLair_Types_H
#define _PokemonAutomation_PokemonSwSh_MaxLair_Types_H

#include <string>
#include <map>

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



}
}
}
}
#endif
