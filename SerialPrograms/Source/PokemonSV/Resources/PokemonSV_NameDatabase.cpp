/*  Name Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSV/Resources/PokemonSV_PokemonSprites.h"
#include "PokemonSV_NameDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


StringSelectDatabase make_name_database(const std::vector<std::string>& slugs){
    const SpriteDatabase& sprites = ALL_POKEMON_SPRITES();

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
StringSelectDatabase make_ALL_POKEMON_NAMES(){
    //  For now, use sprites to determine if it's in the game.
    const SpriteDatabase& sprites = ALL_POKEMON_SPRITES();

    std::vector<std::string> slugs;
    for (std::string& slug : load_pokemon_slug_json_list("Pokemon/Pokedex/Pokedex-National.json")){
        const SpriteDatabase::Sprite* sprite = sprites.get_nothrow(slug);
        if (sprite != nullptr){
            slugs.emplace_back(std::move(slug));
        }
    }

    return make_name_database(slugs);
}


const StringSelectDatabase& ALL_POKEMON_NAMES(){
    static const StringSelectDatabase database = make_ALL_POKEMON_NAMES();
    return database;
}




}
}
}
