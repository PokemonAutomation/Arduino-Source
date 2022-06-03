/*  Available Pokemon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonLA_AvailablePokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



const std::vector<std::string>& HISUI_DEX_SLUGS(){
    static const std::vector<std::string> database = Pokemon::load_pokemon_slug_json_list("Pokemon/Pokedex/Pokedex-Hisui.json");
    return database;
}

const std::vector<std::string>& HISUI_OUTBREAK_SLUGS(){
    static const std::vector<std::string> database = Pokemon::load_pokemon_slug_json_list("PokemonLA/OutbreakList.json");
    return database;
}

const std::vector<std::string>& MMO_FIRST_WAVE_SPRITE_SLUGS(){
    static const std::vector<std::string> database = Pokemon::load_pokemon_slug_json_list("PokemonLA/MMOFirstWaveSpriteList.json");
    return database;
}

std::array<std::vector<std::string>, 5> load_MMO_first_wave_region_sprite_slugs(){
    std::array<std::vector<std::string>, 5> slugs;
    for(int i = 0; i < 5; i++){
        std::string filename = "PokemonLA/MMOFirstWaveSpriteList-" + std::to_string(i) + ".json";
        slugs[i] = Pokemon::load_pokemon_slug_json_list(filename.c_str());
    }
    return slugs;
}

const std::array<std::vector<std::string>, 5>& MMO_FIRST_WAVE_REGION_SPRITE_SLUGS(){
    static const std::array<std::vector<std::string>, 5> database = load_MMO_first_wave_region_sprite_slugs();
    return database;
}



Pokemon::DisplayNameMapping load_MMO_first_wave_display_name_mapping(){
    const std::vector<std::string>& sprite_slugs = MMO_FIRST_WAVE_SPRITE_SLUGS();
    const char* display_json_path = "PokemonLA/MMOFirstWaveSpriteNameDisplay.json";
    std::vector<std::string> sprite_display_names = Pokemon::load_pokemon_slug_json_list(display_json_path);

    if (sprite_slugs.size() != sprite_display_names.size()){
        throw FileException(
            nullptr, PA_CURRENT_FUNCTION,
            "Expected MMO sprite slug list and its display name list to have same length.",
            (RESOURCE_PATH() + display_json_path)
        );
    }

    Pokemon::DisplayNameMapping mapping;
    for(size_t i = 0; i < sprite_slugs.size(); i++){
        const auto& slug = sprite_slugs[i];
        mapping.display_names.emplace(slug, sprite_display_names[i]);
        mapping.display_name_to_slug.emplace(std::move(sprite_display_names[i]), slug);
    }

    return mapping;
}

const Pokemon::DisplayNameMapping& MMO_FIRST_WAVE_DISPLAY_NAME_MAPPING(){
    static const Pokemon::DisplayNameMapping& mapping = load_MMO_first_wave_display_name_mapping();
    return mapping;
}

}
}
}
