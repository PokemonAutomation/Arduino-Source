/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Logging/LoggerQt.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_NameSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


StringSelectDatabase make_all_names_database(){
    StringSelectDatabase ret;
    for (const auto& slug : NATIONAL_DEX_SLUGS()){
        const PokemonNames& data = get_pokemon_name(slug);
        const SpriteDatabase::Sprite* sprite = ALL_POKEMON_SPRITES().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
    }
    return ret;
}
const StringSelectDatabase& ALL_NAMES_SELECT_DATABASE(){
    static StringSelectDatabase database = make_all_names_database();
    return database;
}




PokemonNameSelectCell::PokemonNameSelectCell(
    const std::string& default_slug
)
    : StringSelectCell(
        ALL_NAMES_SELECT_DATABASE(),
        default_slug
    )
{}



PokemonNameSelectOption::PokemonNameSelectOption(
    std::string label, const std::string& default_slug
)
    : StringSelectOption(
        std::move(label),
        ALL_NAMES_SELECT_DATABASE(),
        default_slug
    )
{}



}
}
}
