/*  Name Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonBDSP_NameDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;



StringSelectDatabase make_name_database(const std::vector<std::string>& slugs){
    const SpriteDatabase& sprites = PokemonSwSh::ALL_POKEMON_SPRITES();

    StringSelectDatabase database;
    for (const std::string& slug : slugs){
        const PokemonNames& name = get_pokemon_name(slug);
        const SpriteDatabase::Sprite* sprite = sprites.get_nothrow(slug);
        if (sprite){
            database.add_entry(StringSelectEntry(
                slug,
                name.display_name(),
                sprite->icon
            ));
        }else{
            global_logger_tagged().log("No sprite for: " + slug);
            database.add_entry(StringSelectEntry(
                slug,
                name.display_name()
            ));
        }
    }
    return database;
}
StringSelectDatabase make_name_database(const char* json_file_slugs){
    return make_name_database(load_pokemon_slug_json_list(json_file_slugs));
}



StringSelectDatabase make_ALL_POKEMON_NAMES(){
    std::vector<std::string> slugs = load_pokemon_slug_json_list("Pokemon/Pokedex/Pokedex-National.json");
    if (slugs.size() < 493){
        throw FileException(
            nullptr, PA_CURRENT_FUNCTION,
            "Expected national dex to be greater than 493 members.",
            "Pokemon/Pokedex/Pokedex-National.json"
        );
    }
    slugs.resize(493);
    return make_name_database(slugs);
}
const StringSelectDatabase& ALL_POKEMON_NAMES(){
    static const StringSelectDatabase database = make_ALL_POKEMON_NAMES();
    return database;
}



}
}
}
