/*  Berry Selector, UI component to select multiple berries
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Resources/Pokemon_BerryNames.h"
#include "Pokemon/Resources/Pokemon_BerrySprites.h"
#include "PokemonBDSP_BerrySelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


StringSelectDatabase make_all_berries_database(){
    StringSelectDatabase ret;
    for (const auto& slug : BERRY_SLUGS()){
        const BerryNames& data = get_berry_name(slug);
        const SpriteDatabase::Sprite* sprite = ALL_BERRY_SPRITES().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
    }
    return ret;
}
const StringSelectDatabase& ALL_BERRYS_SELECT_DATABASE(){
    static StringSelectDatabase database = make_all_berries_database();
    return database;
}




BerrySelectCell::BerrySelectCell(
    const std::string& default_slug
)
    : StringSelectCell(
        ALL_BERRYS_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}


}
}
}
