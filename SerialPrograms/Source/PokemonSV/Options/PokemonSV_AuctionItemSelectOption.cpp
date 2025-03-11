/*  Auction Item Selector, UI component to select multiple items from auctions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "PokemonSV/Resources/PokemonSV_AuctionItemNames.h"
#include "PokemonSV/Resources/PokemonSV_ItemSprites.h"
#include "PokemonSV_AuctionItemSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


StringSelectDatabase make_auction_item_database(){
    StringSelectDatabase ret;
    for (const auto& slug : AUCTION_ITEM_SLUGS()){
        const AuctionItemNames& data = get_auction_item_name(slug);
        const SpriteDatabase::Sprite* sprite = AUCTION_ITEM_SPRITES().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
    }
    return ret;
}
const StringSelectDatabase& AUCTION_ITEM_SELECT_DATABASE(){
    static StringSelectDatabase database = make_auction_item_database();
    return database;
}




AuctionItemSelectCell::AuctionItemSelectCell(
    const std::string& default_slug
)
    : StringSelectCell(
        AUCTION_ITEM_SELECT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}


}
}
}
