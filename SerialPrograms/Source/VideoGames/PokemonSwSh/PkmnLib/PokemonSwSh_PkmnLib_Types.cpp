/*  PkmnLib Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_PkmnLib_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{

const std::string TYPE_NAMES[19] = {
    "normal",
    "fighting",
    "flying",
    "poison",
    "ground",
    "rock",
    "bug",
    "ghost",
    "steel",
    "fire",
    "water",
    "grass",
    "electric",
    "psychic",
    "ice",
    "dragon",
    "dark",
    "fairy",
    "none"};

const std::map<std::string, Type> STR_TYPE_MAP = {
    {"normal", Type::normal},
    {"fighting", Type::fighting},
    {"flying", Type::flying},
    {"poison", Type::poison},
    {"ground", Type::ground},
    {"rock", Type::rock},
    {"bug", Type::bug},
    {"ghost", Type::ghost},
    {"steel", Type::steel},
    {"fire", Type::fire},
    {"water", Type::water},
    {"grass", Type::grass},
    {"electric", Type::electric},
    {"psychic", Type::psychic},
    {"ice", Type::ice},
    {"dragon", Type::dragon},
    {"dark", Type::dark},
    {"fairy", Type::fairy},
    {"none", Type::none}};

// row is attacker, column is defender
// normal,fighting,flying,poison,ground,rock,bug,ghost,steel,fire,water,grass,electric,psychic,ice,dragon,dark,fairy
const float TYPE_EFFECTIVENESS_TABLE[18][18] = {
    {1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, // normal
    {2.0, 1.0, 0.5, 0.5, 1.0, 2.0, 0.5, 0.0, 2.0, 1.0, 1.0, 1.0, 1.0, 0.5, 2.0, 1.0, 2.0, 0.5}, // fighting,
    {1.0, 2.0, 1.0, 1.0, 1.0, 0.5, 2.0, 1.0, 0.5, 1.0, 1.0, 2.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0}, // flying,
    {1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 0.5, 0.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0}, // poison
    {1.0, 1.0, 0.0, 2.0, 1.0, 2.0, 0.5, 1.0, 2.0, 2.0, 1.0, 0.5, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0}, // ground,
    {1.0, 0.5, 2.0, 1.0, 0.5, 1.0, 2.0, 1.0, 0.5, 2.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0}, // rock
    {1.0, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 2.0, 1.0, 2.0, 1.0, 1.0, 2.0, 0.5}, // bug,
    {0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 0.5, 1.0}, // ghost
    {1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 0.5, 0.5, 0.5, 1.0, 0.5, 1.0, 2.0, 1.0, 1.0, 2.0}, // steel,
    {1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 2.0, 1.0, 2.0, 0.5, 0.5, 2.0, 1.0, 1.0, 2.0, 0.5, 1.0, 1.0}, // fire
    {1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 2.0, 0.5, 0.5, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0}, // water
    {1.0, 1.0, 0.5, 0.5, 2.0, 2.0, 0.5, 1.0, 0.5, 0.5, 2.0, 0.5, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0}, // grass
    {1.0, 1.0, 2.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 0.5, 0.5, 1.0, 1.0, 0.5, 1.0, 1.0}, // electric
    {1.0, 2.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 0.0, 1.0}, // psychic
    {1.0, 1.0, 2.0, 1.0, 2.0, 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 2.0, 1.0, 1.0, 0.5, 2.0, 1.0, 1.0}, // ice
    {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 0.0}, // dragon
    {1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 0.5, 0.5}, // dark
    {1.0, 2.0, 1.0, 0.5, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 1.0}  // fairy
};

float damage_multiplier(Type attack, Type defense){
    if (attack == Type::none){
        return 0.0;
    }

    if (defense == Type::none){
        return 1.0;
    }

    return TYPE_EFFECTIVENESS_TABLE[(size_t)attack][(size_t)defense];
}

float damage_multiplier(Type attack, Type defense1, Type defense2){
    // calculate the damage multiplier for multiple types
    // the way damageMultiplier works, it should return 1 if the second type is none, so we're good to go
    float multiplier1 = damage_multiplier(attack, defense1);
    float multiplier2 = damage_multiplier(attack, defense2);

    return multiplier1 * multiplier2;
}

Type get_type_from_string(const std::string& in_type){
    std::map<std::string, Type>::const_iterator iter = STR_TYPE_MAP.find(in_type);
    if (iter != STR_TYPE_MAP.end()){
        return iter->second;
    }else{
        return Type::none;
    }
}

const std::string& get_type_name(const Type in_type){
    return TYPE_NAMES[(size_t)in_type];
}




Type serial_type_to_pkmnlib(Pokemon::PokemonType type){
    using Pokemon::PokemonType;
    switch (type){
    case PokemonType::NONE:     return Type::none;
    case PokemonType::NORMAL:   return Type::normal;
    case PokemonType::FIRE:     return Type::fire;
    case PokemonType::FIGHTING: return Type::fighting;
    case PokemonType::WATER:    return Type::water;
    case PokemonType::FLYING:   return Type::flying;
    case PokemonType::GRASS:    return Type::grass;
    case PokemonType::POISON:   return Type::poison;
    case PokemonType::ELECTRIC: return Type::electric;
    case PokemonType::GROUND:   return Type::ground;
    case PokemonType::PSYCHIC:  return Type::psychic;
    case PokemonType::ROCK:     return Type::rock;
    case PokemonType::ICE:      return Type::ice;
    case PokemonType::BUG:      return Type::bug;
    case PokemonType::DRAGON:   return Type::dragon;
    case PokemonType::GHOST:    return Type::ghost;
    case PokemonType::DARK:     return Type::dark;
    case PokemonType::STEEL:    return Type::steel;
    case PokemonType::FAIRY:    return Type::fairy;
    default: return Type::none;
    }
}





}
}
}
}
