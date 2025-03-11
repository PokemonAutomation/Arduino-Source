/*  Sandwich Ingredients Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV_SandwichIngredientsOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

StringSelectDatabase make_sandwich_ingredient_database(){
    StringSelectDatabase ret;
    for (const auto& slug : ALL_SANDWICH_FILLINGS_SLUGS()){
        const SandwichIngredientNames& data = get_ingredient_name(slug);
        const SpriteDatabase::Sprite* sprite = SANDWICH_FILLINGS_DATABASE().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
    }
    for (const auto& slug : ALL_SANDWICH_CONDIMENTS_SLUGS()){
        const SandwichIngredientNames& data = get_ingredient_name(slug);
        const SpriteDatabase::Sprite* sprite = SANDWICH_CONDIMENTS_DATABASE().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
    }
    
    return ret;
}
const StringSelectDatabase& SANDWICH_INGREDIENT_DATABASE(){
    static StringSelectDatabase database = make_sandwich_ingredient_database();
    return database;
}


SandwichIngredientsTableCell::SandwichIngredientsTableCell(
    const std::string& default_slug
)
    : StringSelectCell(
        SANDWICH_INGREDIENT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}



}
}
}
