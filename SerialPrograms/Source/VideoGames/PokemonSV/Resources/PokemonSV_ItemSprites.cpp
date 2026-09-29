/*  Pokemon Scarlet/Violet Item Sprites
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV_ItemSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

const SpriteDatabase& AUCTION_ITEM_SPRITES(){
    static const SpriteDatabase database("PokemonSV/Auction/AuctionItemSprites.png", "PokemonSV/Auction/AuctionItemSprites.json");
    return database;
}


}
}
}
