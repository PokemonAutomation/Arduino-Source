/*  Pokemon Ball Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "PokemonSwSh_BallSelectOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


StringSelectDatabase make_all_balls_database(){
    StringSelectDatabase ret;
    for (const auto& slug : POKEBALL_SLUGS()){
//        cout << "slug = " << slug << endl;
        const PokeballNames& data = get_pokeball_name(slug);
        const SpriteDatabase::Sprite* sprite = ALL_POKEBALL_SPRITES().get_nothrow(slug);
        if (sprite != nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }else{
//            ret.add_entry(StringSelectEntry(slug, data.display_name()));
//            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }
    }
    return ret;
}
const StringSelectDatabase& ALL_BALLS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_all_balls_database();
    return database;
}




PokemonBallSelectCell::PokemonBallSelectCell(
    const std::string& default_slug
)
    : StringSelectCell(
        ALL_BALLS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}



PokemonBallSelectOption::PokemonBallSelectOption(
    std::string label,
    LockMode lock_while_running,
    const std::string& default_slug
)
    : StringSelectOption(
        std::move(label),
        ALL_BALLS_SELECT_DATABASE(),
        lock_while_running,
        default_slug
    )
{}




}
}
}
