/*  Pokemon FRLG Pokemon Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <set>
#include "PokemonFRLG_PokemonSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


const SpriteDatabase& ALL_POKEMON_SPRITES(){
    static const SpriteDatabase database("PokemonFRLG/FrontSprites.png", "PokemonFRLG/FrontSprites.json");
    return database;
}
const SpriteDatabase& ALL_POKEMON_SHINY_SPRITES(){
    static const SpriteDatabase database("PokemonFRLG/FrontSpritesShiny.png", "PokemonFRLG/FrontSprites.json");
    return database;
}
const SpriteDatabase& FLIPPED_POKEMON_SPRITES(){
    static const SpriteDatabase database("PokemonFRLG/FrontSprites.png", "PokemonFRLG/FrontSprites.json", sprite_flipped_on_summary);
    return database;
}
const SpriteDatabase& FLIPPED_POKEMON_SHINY_SPRITES(){
    static const SpriteDatabase database("PokemonFRLG/FrontSpritesShiny.png", "PokemonFRLG/FrontSprites.json", sprite_flipped_on_summary);
    return database;
}

const SpriteDatabase& POKEMON_SPRITES(bool shiny, bool flipped){
    if (flipped){
        return shiny ? FLIPPED_POKEMON_SHINY_SPRITES() : FLIPPED_POKEMON_SPRITES();
    }else{
        return shiny ? ALL_POKEMON_SHINY_SPRITES() : ALL_POKEMON_SPRITES();
    }
}


const std::string& sprite_slug_to_species(const std::string& sprite_slug){
    static const std::map<std::string, std::string> MAP{
        {"deoxys-attack",       "deoxys"},
        {"deoxys-defense",      "deoxys"},
        {"unown-exclamation",   "unown-!"},
        {"unown-question",      "unown-?"},
    };
    auto iter = MAP.find(sprite_slug);
    return iter == MAP.end() ? sprite_slug : iter->second;
}

bool sprite_in_species_subset(const std::string& sprite_slug, const std::set<std::string>& subset){
    if (subset.find(sprite_slug) != subset.end()){
        return true;
    }
    const std::string& species = sprite_slug_to_species(sprite_slug);
    if (subset.find(species) != subset.end()){
        return true;
    }
    if (species.rfind("unown-", 0) == 0){
        return subset.find("unown") != subset.end();
    }
    return false;
}

bool sprite_flipped_on_summary(const std::string& sprite_slug){
    //  From the "noFlip" flags in pokefirered's src/data/pokemon/species_info.h.
    static const std::set<std::string> NO_FLIP{
        "poliwag", "poliwhirl", "poliwrath", "kingler", "electabuzz",
        "croconaw", "igglybuff", "politoed", "sneasel", "teddiursa",
        "elekid", "spinda", "roselia", "absol", "seviper", "zangoose",
        "deoxys",
    };
    const std::string& species = sprite_slug_to_species(sprite_slug);
    if (species.rfind("unown", 0) == 0){
        return false;
    }
    return NO_FLIP.find(species) == NO_FLIP.end();
}



}
}
}
