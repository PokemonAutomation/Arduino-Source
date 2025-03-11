/*  Tournament Prize Select Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "PokemonSV/Resources/PokemonSV_TournamentPrizeNames.h"
#include "PokemonSV/Resources/PokemonSV_ItemSprites.h"
#include "PokemonSV_TournamentPrizeSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

StringSelectDatabase make_tournament_prize_database(){
    StringSelectDatabase ret;
    for (const auto& slug : TOURNAMENT_PRIZE_SLUGS()){
        const TournamentPrizeNames& data = get_tournament_prize_name(slug);
        /*
        const SpriteDatabase::Sprite* sprite = TOURNAMENT_PRIZE_SPRITES().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
        */
        ret.add_entry(StringSelectEntry(slug, data.display_name()));
    }
    return ret;
}
const StringSelectDatabase& TOURNAMENT_PRIZE_SELECT_DATABASE(){
    static StringSelectDatabase database = make_tournament_prize_database();
    return database;
}


TournamentPrizeSelectCell::TournamentPrizeSelectCell(
    const std::string& default_slug
)
    : StringSelectCell(
        TOURNAMENT_PRIZE_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}


}
}
}
