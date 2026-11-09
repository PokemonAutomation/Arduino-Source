/*  BDSP AutoStory - Shopping
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Shopping_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Shopping_H

#include <vector>

#include "CommonFramework/Language.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
class VideoStream;
namespace NintendoSwitch {
namespace PokemonBDSP {

// ---------------------------------------------------------------------------
//  Mart buying
// ---------------------------------------------------------------------------

enum class ShopItem {
    FullRestore,
    MaxPotion,
    HyperPotion,
    SuperPotion,
    Potion,
    Antidote,
    ParalyzHeal,
    Awakening,
    Repel,
    SuperRepel,
    MaxRepel,
    EscapeRope,
    XAttack,
    XSpAtk,
    XSpeed,
    XSpDef,
};

// Pass as the `quantity` field to buy the maximum available amount.
static constexpr int BUY_MAX = -1;

enum class ShopQuantityMode {
    PURCHASE,
    TARGET_STOCK,
};

struct ItemToBuy {
    ShopItem item;
    int quantity; // purchase count (or BUY_MAX), or a positive target inventory total
    ShopQuantityMode mode = ShopQuantityMode::PURCHASE;
};

// Start facing a vendor at interaction distance; navigate through the Buy menu,
// and purchase each item in `items` in order. search_direction must be DPAD_DOWN
// or DPAD_UP; use UP to wrap toward items near the bottom of the list. Search
// starts at the current selection for each item and confirms its name via OCR.
// TARGET_STOCK buys only the deficit and verifies the resulting stock.
// Unknown stock or an unconfirmed purchase fails rather than replaying the list.
bool buy_items(VideoStream& stream,
    ProControllerContext& context,
    const std::vector<ItemToBuy>& items,
    Language language = Language::English,
    DpadPosition search_direction = DPAD_DOWN);


// Lists are visited in this order. Empty lists skip interaction at that vendor.
// Add items to the existing ShopItem/OCR catalog when introducing a new purchase.
struct VeilstoneShoppingList {
    std::vector<ItemToBuy> floor3_top;
    std::vector<ItemToBuy> floor3_bottom;
    std::vector<ItemToBuy> floor2_top;
    std::vector<ItemToBuy> floor2_middle;
    std::vector<ItemToBuy> floor2_bottom; // Counter-app NPC: must remain empty.
};

// Start: outside Veilstone Pokemon Center after Fly, on foot.
// Take the elevator to 3F, visit all five vendor positions, then exit the store.
// End: immediately outside the department-store door. Do not retry from inside
// the store; checkpoint recovery must restore the pre-shopping save first.
bool shop_veilstone_department_store(VideoStream& stream,
    ProControllerContext& context,
    const VeilstoneShoppingList& items,
    Language language);

// Reusable plans live in Shopping.cpp; callers select a plan or supply their own.
namespace ShoppingLists {
extern const std::vector<ItemToBuy> ASSISTED_LEAGUE_MEDICINE;
extern const VeilstoneShoppingList ASSISTED_INFERNAPE_LEAGUE;
extern const VeilstoneShoppingList ASSISTED_EMPOLEON_LEAGUE;
extern const VeilstoneShoppingList ASSISTED_TORTERRA_LEAGUE;
}

}
}
}
#endif
