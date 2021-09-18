/*  Pokemon Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Pokemon_Types.h"

namespace PokemonAutomation{
namespace Pokemon{


const std::map<PokemonType, std::string> TYPE_ENUM_TO_SLUG{
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

const std::string& get_type_slug(PokemonType type){
    auto it = TYPE_ENUM_TO_SLUG.find(type);
    if (it == TYPE_ENUM_TO_SLUG.end()){
        PA_THROW_StringException("Invalid type enum.");
    }
    return it->second;
}



}
}
