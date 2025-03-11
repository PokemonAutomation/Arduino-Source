/*  Pokemon Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_Types.h"

namespace PokemonAutomation{
namespace Pokemon{



const EnumStringMap<PokemonType>& POKEMON_TYPE_SLUGS(){
    static EnumStringMap<PokemonType> database{
        {PokemonType::NONE,     "none"},
        {PokemonType::NORMAL,   "normal"},
        {PokemonType::FIRE,     "fire"},
        {PokemonType::FIGHTING, "fighting"},
        {PokemonType::WATER,    "water"},
        {PokemonType::FLYING,   "flying"},
        {PokemonType::GRASS,    "grass"},
        {PokemonType::POISON,   "poison"},
        {PokemonType::ELECTRIC, "electric"},
        {PokemonType::GROUND,   "ground"},
        {PokemonType::PSYCHIC,  "psychic"},
        {PokemonType::ROCK,     "rock"},
        {PokemonType::ICE,      "ice"},
        {PokemonType::BUG,      "bug"},
        {PokemonType::DRAGON,   "dragon"},
        {PokemonType::GHOST,    "ghost"},
        {PokemonType::DARK,     "dark"},
        {PokemonType::STEEL,    "steel"},
        {PokemonType::FAIRY,    "fairy"},
    };
    return database;
}




}
}
