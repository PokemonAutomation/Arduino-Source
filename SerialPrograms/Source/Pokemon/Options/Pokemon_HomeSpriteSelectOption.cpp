/*  HomeSprite Selector, UI component to select multiple berries
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Resources/Pokemon_PokemonForms.h"
#include "Pokemon/Resources/Pokemon_PokemonHomeSprites.h"
#include "Pokemon_HomeSpriteSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{


StringSelectDatabase make_all_home_sprites_database(){
    StringSelectDatabase ret;
    for(const auto& slug: ALL_POKEMON_FORMS()){
    // for(const auto& p: ALL_POKEMON_HOME_SPRITES().get()){
        const SpriteDatabase::Sprite* sprite = ALL_POKEMON_HOME_SPRITES().get_nothrow(slug);
        const PokemonForm* form_ptr = get_pokemon_form(slug);
        if (form_ptr == nullptr){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "A form slug is in ALL_POKEMON_FORMS but not available in get_pokemon_form() : " + slug);
        }
        if (sprite == nullptr){
            // we don't have the sprite. Use a replacement sprite instead:
            const auto& no_show_sprite = ALL_POKEMON_HOME_SPRITES().get_throw("floette-eternal-flower");
            ret.add_entry(StringSelectEntry(slug, form_ptr->display_name(), no_show_sprite.icon));
        } else{
            ret.add_entry(StringSelectEntry(slug, form_ptr->display_name(), sprite->icon));
        }
    }
    
    return ret;
}
const StringSelectDatabase& ALL_HOME_SPRITES_SELECT_DATABASE(){
    static StringSelectDatabase database = make_all_home_sprites_database();
    return database;
}




HomeSpriteSelectCell::HomeSpriteSelectCell(
    const std::string& default_slug
)
    : StringSelectCell(
        ALL_HOME_SPRITES_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}


}
}
