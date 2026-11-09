/*  BDSP AutoStory - Shopping
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShopDetectors.h"
#include "PokemonBDSP/Programs/AutoStory/PokemonBDSP_AutoStoryTools.h"

#include "PokemonBDSP_AutoStory_Dialog.h"
#include "PokemonBDSP_AutoStory_Shopping.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {

// ---------------------------------------------------------------------------
//  Reusable shopping lists
// ---------------------------------------------------------------------------

namespace ShoppingLists {
const std::vector<ItemToBuy> ASSISTED_LEAGUE_MEDICINE{
    {ShopItem::FullRestore, 10, ShopQuantityMode::TARGET_STOCK},
};

const VeilstoneShoppingList ASSISTED_INFERNAPE_LEAGUE{
    {},
    {},
    {
        {ShopItem::XAttack, 3, ShopQuantityMode::TARGET_STOCK},
        {ShopItem::XSpDef, 2, ShopQuantityMode::TARGET_STOCK},
    },
    {},
    {},
};

// Initial testing reserve; validate consumption with Empoleon's League battles.
const VeilstoneShoppingList ASSISTED_EMPOLEON_LEAGUE{
    {},
    {},
    {
        {ShopItem::XSpAtk, 5, ShopQuantityMode::TARGET_STOCK},
        {ShopItem::XSpeed, 5, ShopQuantityMode::TARGET_STOCK},
    },
    {},
    {},
};

// The assisted Torterra plan needs medicine but no department-store purchases.
const VeilstoneShoppingList ASSISTED_TORTERRA_LEAGUE{};
}

// ---------------------------------------------------------------------------
//  Purchasing and store navigation
// ---------------------------------------------------------------------------

static const char* shop_item_name(ShopItem item){
    switch (item){
    case ShopItem::FullRestore:
        return "Full restore";
    case ShopItem::MaxPotion:
        return "Max potion";
    case ShopItem::HyperPotion:
        return "Hyper potion";
    case ShopItem::SuperPotion:
        return "Super potion";
    case ShopItem::Potion:
        return "Potion";
    case ShopItem::Antidote:
        return "Antidote";
    case ShopItem::ParalyzHeal:
        return "Paralyz heal";
    case ShopItem::Awakening:
        return "Awakening";
    case ShopItem::Repel:
        return "Repel";
    case ShopItem::SuperRepel:
        return "Super repel";
    case ShopItem::MaxRepel:
        return "Max repel";
    case ShopItem::EscapeRope:
        return "Escape rope";
    case ShopItem::XAttack:
        return "X Attack";
    case ShopItem::XSpDef:
        return "X Sp. Def";
    case ShopItem::XSpAtk:
        return "X Sp. Atk";
    case ShopItem::XSpeed:
        return "X Speed";
    default:
        return "Unknown";
    }
}

// Returns the OCR slug used as the dictionary key in ShopItemNameOCR.json.
static const char* shop_item_slug(ShopItem item){
    switch (item){
    case ShopItem::FullRestore:
        return "full-restore";
    case ShopItem::MaxPotion:
        return "max-potion";
    case ShopItem::HyperPotion:
        return "hyper-potion";
    case ShopItem::SuperPotion:
        return "super-potion";
    case ShopItem::Potion:
        return "potion";
    case ShopItem::Antidote:
        return "antidote";
    case ShopItem::ParalyzHeal:
        return "paralyze-heal";
    case ShopItem::Awakening:
        return "awakening";
    case ShopItem::Repel:
        return "repel";
    case ShopItem::SuperRepel:
        return "super-repel";
    case ShopItem::MaxRepel:
        return "max-repel";
    case ShopItem::EscapeRope:
        return "escape-rope";
    case ShopItem::XAttack:
        return "x-attack";
    case ShopItem::XSpDef:
        return "x-sp-def";
    case ShopItem::XSpAtk:
        return "x-sp-atk";
    case ShopItem::XSpeed:
        return "x-speed";
    default:
        return "unknown";
    }
}

// Describe purchase counts separately from desired inventory totals.
static std::string make_buy_label(const std::vector<ItemToBuy>& items){
    std::string label;
    for (size_t i = 0; i < items.size(); i++){
        if (i > 0){
            label += ", ";
        }
        if (items[i].mode == ShopQuantityMode::TARGET_STOCK){
            label += "stock target " + std::to_string(items[i].quantity) + " ";
        }else if (items[i].quantity == BUY_MAX){
            label += "max ";
        }else{
            label += std::to_string(items[i].quantity) + " ";
        }
        label += shop_item_name(items[i].item);
    }
    return label;
}

bool buy_items(VideoStream& stream,
    ProControllerContext& context,
    const std::vector<ItemToBuy>& items,
    Language language,
    DpadPosition search_direction){
    if (items.empty()){
        return true;
    }

    if (search_direction != DPAD_UP && search_direction != DPAD_DOWN){
        stream.log("buy_items: item search direction must be UP or DOWN.", COLOR_RED);
        return false;
    }

    for (const ItemToBuy& item : items){
        if ((item.quantity <= 0 && item.quantity != BUY_MAX) || item.quantity > 999 ||
            (item.mode == ShopQuantityMode::TARGET_STOCK && item.quantity <= 0)){
            stream.log("buy_items: invalid purchase quantity or stock target.", COLOR_RED);
            return false;
        }
    }
    const std::string label = make_buy_label(items);
    context.wait_for_all_requests();
    pbf_wait(context, 500ms);
    pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
    context.wait_for_all_requests();


    // ── Coordinate constants ─────────────────────────────────────────────────

    // Area where the selection arrow appears at the vendor greeting menu (Buy/Sell/Leave).
    static const ImageFloatBox VENDOR_ARROW_BOX{0.630000, 0.550000, 0.070000, 0.120000};

    // Area where the selection arrow appears at the YES/NO confirmation dialog.
    static const ImageFloatBox CONFIRM_ARROW_BOX{0.700000, 0.620000, 0.070000, 0.120000};

    // Full area containing all item rows in the shop list.
    // The hover detector scans this region to dynamically locate the red outline.
    static const ImageFloatBox ITEM_SCAN_AREA{0.417000, 0.100000, 0.540000, 0.625000};

    static constexpr size_t MAX_OUTER = 50;
    static constexpr size_t MAX_ITEM_SEARCH = 50; // max iterations per item before giving up

    // ── Outer state machine ──────────────────────────────────────────────────
    for (size_t outer = 0; outer < MAX_OUTER; outer++){
        SelectionArrowFinder arrow_vendor(stream.overlay(), VENDOR_ARROW_BOX, COLOR_GREEN);
        ShopBuyingWindowWatcher buying_window;
        ShortDialogWatcher dialog(COLOR_RED);

        // case 0: no detectors fired — press A in a for-loop (5 presses) while watching.
        stream.log("Starting to loop over items");
        int state = run_until<ProControllerContext>(stream,
            context,
            [](ProControllerContext& ctx){
                for (int i = 0; i < 5; i++){
                    pbf_press_button(ctx, BUTTON_A, 80ms, 1000ms);
                }
            },
            {{arrow_vendor}, {buying_window}, {dialog}});

        context.wait_for_all_requests();
        pbf_wait(context, 500ms);

        switch (state){

        case 0: // case 1: SelectionArrow at vendor menu → press A to choose "Buy"
            stream.log("Found the selection arrow, pressing A to enter the buying window...", COLOR_GREEN);
            pbf_press_button(context, BUTTON_A, 80ms, 500ms);
            break;

        case 1: // case 2: buying window open → buy each requested item
            stream.log("Found the buying window, searching for the right item...", COLOR_GREEN);
            {
                bool buying_failed = false;

                for (const ItemToBuy& item : items){
                    stream.log("Shopping for " + std::string(shop_item_slug(item.item)));
                    if (buying_failed){
                        break;
                    }

                    // Confirm the selection outline and item name before opening
                    // the quantity selector.
                    ShopItemHoverDetector hover(stream.logger(),
                        ITEM_SCAN_AREA,
                        shop_item_slug(item.item),
                        language,
                        COLOR_CYAN);
                    ShopQuantityWindowWatcher qty_window;
                    SelectionArrowFinder confirm_arrow(stream.overlay(), CONFIRM_ARROW_BOX, COLOR_GREEN);

                    const auto read_stock = [&](){
                        int previous = -1;
                        for (size_t sample = 0; sample < 4; sample++){
                            pbf_wait(context, 200ms);
                            context.wait_for_all_requests();
                            const auto frame = stream.video().snapshot();
                            const int stock = frame ? hover.read_stock(*frame.frame) : -1;
                            if (stock >= 0 && stock == previous){
                                return stock;
                            }
                            previous = stock;
                        }
                        return -1;
                    };
                    int quantity_to_buy = item.quantity;
                    bool item_bought = false;
                    bool hover_confirmed = false;
                    bool quantity_selected = false;
                    for (size_t inner = 0; inner < MAX_ITEM_SEARCH && !item_bought; inner++){
                        context.wait_for_all_requests();

                        int sub;
                        if (!hover_confirmed){
                            sub = run_until<ProControllerContext>(stream,
                                context,
                                [](ProControllerContext& ctx){
                                    // Name OCR needs three matching sampled frames.
                                    ctx.wait_for(3000ms);
                                },
                                {{confirm_arrow}, {qty_window}, {hover}});
                        }else if (!quantity_selected){
                            // The selected name remains visible behind the quantity
                            // window. Do not let it trigger another A press here.
                            sub = wait_until(stream, context, 3000ms, {{qty_window}}) == 0 ? 1 : -1;
                        }else{
                            // Once the count is set, only the purchase confirmation
                            // may advance the transaction.
                            sub = wait_until(stream, context, 3000ms, {{confirm_arrow}}) == 0 ? 0 : -1;
                        }

                        context.wait_for_all_requests();

                        stream.log("buy_items: sub=" + std::to_string(sub), COLOR_WHITE);

                        switch (sub){

                        case 0: // YES/NO confirmation arrow → confirm purchase
                            stream.log("Found the confirmation to buy arrow, pressing A to buy", COLOR_GREEN);
                            {
                                context.wait_for_all_requests();
                                if (!hover_confirmed || !quantity_selected){
                                    pbf_press_button(context, BUTTON_B, 80ms, 800ms);
                                    break;
                                }
                                // Confirm once. Further A presses can reopen the quantity
                                // selector and purchase another item before the watcher fires.
                                pbf_press_button(context, BUTTON_A, 80ms, 800ms);
                                context.wait_for_all_requests();
                                ShopBuyingWindowWatcher back_in_shop;
                                int purchase = run_until<ProControllerContext>(stream,
                                    context,
                                    [](ProControllerContext& ctx){
                                        for (int i = 0; i < 10; i++){
                                            pbf_press_button(ctx, BUTTON_B, 80ms, 800ms);
                                        }
                                    },
                                    {{back_in_shop}});
                                if (purchase < 0){
                                    stream.log(label + ": purchase did not return to the item list.", COLOR_RED);
                                    return false;
                                }
                                if (item.mode == ShopQuantityMode::TARGET_STOCK && read_stock() < item.quantity){
                                    stream.log(label + ": target stock not confirmed after purchase.", COLOR_RED);
                                    return false;
                                }
                                item_bought = true;
                                break;
                            }

                        case 1: // quantity window
                            if (!hover_confirmed){
                                // Quantity window appeared before we confirmed the right item —
                                // an earlier A press accidentally opened the wrong item's selector.
                                // Press B to go back to the list and continue searching.
                                stream.log("Unexpected quantity window (wrong item), pressing B...", COLOR_YELLOW);
                                pbf_press_button(context, BUTTON_B, 80ms, 800ms);
                                break;
                            }
                            stream.log("Found the quantity window", COLOR_GREEN);
                            if (quantity_selected){
                                // Do not increment the quantity twice if the menu is slow to change.
                                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                                break;
                            }
                            if (quantity_to_buy == BUY_MAX){
                                stream.log("Buying max quantity");
                                pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                            }else{
                                stream.log("Buying " + std::to_string(quantity_to_buy) + " of the item");
                                for (int q = 1; q < quantity_to_buy; q++){
                                    pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                                }
                            }
                            quantity_selected = true;
                            pbf_press_button(context, BUTTON_A, 80ms, 300ms);
                            pbf_wait(context, 1000ms);
                            break;

                        case 2: // correct item highlighted → press A to open quantity selector
                            stream.log("Found the right item, pressing A to buy a certain quantity...", COLOR_GREEN);
                            if (quantity_selected){
                                stream.log(label + ": returned to list without a confirmed purchase.", COLOR_RED);
                                return false;
                            }
                            if (item.mode == ShopQuantityMode::TARGET_STOCK){
                                const int owned = read_stock();
                                if (owned < 0){
                                    stream.log(label + ": could not read current stock.", COLOR_RED);
                                    return false;
                                }
                                stream.log("Current stock: " + std::to_string(owned) +
                                    "; target: " + std::to_string(item.quantity));
                                if (owned >= item.quantity){
                                    item_bought = true;
                                    break;
                                }
                                quantity_to_buy = item.quantity - owned;
                            }
                            hover_confirmed = true;
                            pbf_press_button(context, BUTTON_A, 80ms, 300ms);
                            pbf_wait(context, 1000ms);
                            break;

                        default:
                            if (hover_confirmed){
                                stream.log(label + (quantity_selected
                                    ? ": purchase confirmation did not appear."
                                    : ": quantity selector did not appear."), COLOR_RED);
                                return false;
                            }
                            // Only move between rows while searching the item list.
                            stream.log(search_direction == DPAD_UP
                                ? "Not on the right item, searching upward..."
                                : "Not on the right item, searching downward...", COLOR_GREEN);
                            pbf_press_dpad(context, search_direction, 80ms, 300ms);
                            pbf_wait(context, 300ms);
                            break;
                        }
                    }

                    if (!item_bought){
                        stream.log(label + ": buy_items: could not locate " + shop_item_name(item.item) +
                            " in shop list.",
                            COLOR_RED);
                        buying_failed = true;
                    }
                }

                if (!buying_failed){
                    stream.log("All items bought, exiting shop...", COLOR_GREEN);
                    pbf_mash_button(context, BUTTON_B, 5000ms);
                    return true;
                }
                // A previous item may already have been purchased. Do not restart the list.
                return false;
            }

        case 2: // Vendor greeting can appear before the Buy/Sell arrow.
        {
            stream.log("buy_items: advance vendor greeting to the shop menu.");
            SelectionArrowFinder greeting_menu(stream.overlay(), VENDOR_ARROW_BOX, COLOR_GREEN);
            ShopBuyingWindowWatcher back_in_shop;
            int greeting = run_until<ProControllerContext>(stream,
                context,
                [](ProControllerContext& ctx){
                    for (size_t press = 0; press < 5; press++){
                        pbf_press_button(ctx, BUTTON_A, 80ms, 1000ms);
                    }
                },
                {{greeting_menu}, {back_in_shop}});
            if (greeting < 0){
                stream.log(label + ": dialogue did not lead to a shop menu; check the vendor approach.", COLOR_RED);
                return false;
            }
            break;
        }

        default: // outer default: unrecognised state → press B to recover
            pbf_press_button(context, BUTTON_B, 80ms, 3000ms);
            break;
        }
    }

    stream.log(label + ": buy_items: failed to complete after max iterations.", COLOR_RED);
    return false;
}


bool shop_veilstone_department_store(VideoStream& stream,
    ProControllerContext& context,
    const VeilstoneShoppingList& items,
    Language language){
    if (!items.floor2_bottom.empty()){
        stream.log("Veilstone shopping: the 2F lower NPC gives the Counter app, not shop items.", COLOR_RED);
        return false;
    }
    DpadState dpad;
    // Counts include facing presses. Do not add another turning press.
    const auto move = [&](DpadPosition dir, size_t count){
        repeat_dpad(context, dpad, dir, 80ms, 300ms, count, false);
    };
    const auto transition = [&](const char* label, auto action){
        context.wait_for_all_requests();
        BlackScreenWatcher black(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(stream, context,
            [&](ProControllerContext& ctx){
                action(ctx);
                pbf_wait(ctx, 10000ms);
            },
            {{black}});
        if (ret < 0){
            stream.log(std::string("Veilstone shopping: ") + label + " transition missing.", COLOR_RED);
            return false;
        }
        // Stop the approach when the fade starts, then wait without movement
        // so the remaining hold cannot carry into the next room.
        BlackScreenWatcher visible(COLOR_RED, {0.1, 0.1, 0.8, 0.8},
            100, 10, BlackScreenWatcher::FinderType::GONE, 500ms);
        if (wait_until(stream, context, 15s, {{visible}}) < 0){
            stream.log(std::string("Veilstone shopping: ") + label + " transition did not finish.", COLOR_RED);
            return false;
        }
        pbf_wait(context, 500ms);
        return true;
    };
    const auto buy = [&](const char* vendor, const std::vector<ItemToBuy>& list){
        if (list.empty()){
            return true;
        }
        stream.log(std::string("Veilstone shopping: ") + vendor);
        return buy_items(stream, context, list, language);
    };

    // Let the Fly landing finish before the first facing/movement presses.
    pbf_wait(context, 1000ms);
    // Approach the department store from the Fly landing outside the Center.
    move(DPAD_LEFT, 4);
    move(DPAD_DOWN, 2);
    pbf_move_left_joystick(context, {-1, 0}, 2211ms, 300ms);
    // Give the wandering NPC an extra tile of clearance, then restore the
    // west-wall lane above him. Both crossings include a facing press.
    move(DPAD_RIGHT, 4);
    pbf_move_left_joystick(context, {0, 1}, 1344ms, 300ms);
    move(DPAD_LEFT, 4);
    move(DPAD_DOWN_LEFT, 4);
    pbf_move_left_joystick(context, {0, -1}, 894ms, 300ms);
    move(DPAD_DOWN_LEFT, 3);
    if (!transition("store entry", [](ProControllerContext& ctx){
        // Reach the storefront before sliding west. Starting the diagonal
        // from the south wall can pass the door before reaching its entry row.
        // Watch both movements in case the north hold enters the door.
        pbf_move_left_joystick(ctx, {0, 1}, 1000ms, 0ms);
        pbf_move_left_joystick(ctx, {-1, 1}, 2500ms, 0ms);
    })){
        return false;
    }
    move(DPAD_DOWN_RIGHT, 4);
    if (!transition("elevator entry", [&](ProControllerContext& ctx){
        repeat_dpad(ctx, dpad, DPAD_UP, 80ms, 300ms, 9, false);
        // The north approach ends west of the elevator. Slide along the
        // back wall into its doorway instead of only turning right.
        pbf_move_left_joystick(ctx, {1, 1}, 2000ms, 0ms);
    })){
        return false;
    }

    // Turn west and step beside the attendant; a single tap only faces her.
    move(DPAD_LEFT, 2);
    context.wait_for_all_requests();
    // Confirm the initial 5F arrow without queuing A presses across the menu.
    SelectionArrowFinder floor_menu(stream.overlay(), {0.71, 0.425, 0.03, 0.055}, COLOR_GREEN);
    bool floor_menu_ready = false;
    for (size_t press = 0; press < 10; press++){
        if (wait_until(stream, context, 1500ms, {{floor_menu}}) == 0){
            floor_menu_ready = true;
            break;
        }
        pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
        context.wait_for_all_requests();
    }
    if (!floor_menu_ready){
        stream.log("Veilstone shopping: elevator floor menu did not open.", COLOR_RED);
        return false;
    }
    // The initial 5F cursor moves down twice to 3F. Verify that row before A.
    move(DPAD_DOWN, 2);
    context.wait_for_all_requests();
    SelectionArrowFinder third_floor(stream.overlay(), {0.71, 0.54, 0.03, 0.055}, COLOR_GREEN);
    if (wait_until(stream, context, 3000ms, {{third_floor}}) < 0){
        stream.log("Veilstone shopping: third floor selection not confirmed.", COLOR_RED);
        return false;
    }
    stream.log("Veilstone shopping: third floor selected.");
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    // The elevator pauses between departure and arrival text. Clear both before
    // moving; a quiet period longer than the ride prevents an early return.
    clear_dialog(stream, context, BUTTON_B, 6000ms);
    // Return from the attendant to the central door column before leaving.
    move(DPAD_RIGHT, 2);
    if (!transition("third floor", [](ProControllerContext& ctx){
        pbf_move_left_joystick(ctx, {0, -1}, 2000ms, 0ms);
    })){
        return false;
    }
    stream.log("Veilstone shopping: 3F south aisle to the west counters.");
    // Leave the elevator south to the wall, then west to the planter beside the lower counter.
    // These holds include margin against fixed stops, not an NPC.
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 2500ms, 300ms);
    // Four presses = face north + three tiles past the planter.
    // Then face west + three tiles to the vendor column. The upper
    // counter stops the final northward approach.
    move(DPAD_UP, 4);
    move(DPAD_LEFT, 4);
    move(DPAD_UP, 4);
    if (!buy("3F upper vendor", items.floor3_top)){
        return false;
    }
    move(DPAD_DOWN, 4);
    if (!buy("3F lower vendor", items.floor3_bottom)){
        return false;
    }
    // From the vendor column: face east + three tiles to the stair mat.
    move(DPAD_RIGHT, 4);
    // Turn north, then step onto the stair mat. One tap only changes facing.
    move(DPAD_UP, 2);
    if (!transition("second floor", [&](ProControllerContext& ctx){
        // Turn east, then enter the stairs under the transition watcher.
        repeat_dpad(ctx, dpad, DPAD_RIGHT, 80ms, 300ms, 2, false);
    })){
        return false;
    }
    stream.log("Veilstone shopping: 2F north aisle to the west counters.");
    // 2froute.txt: pass north of the stairwell, then approach the upper
    // counter from the east. Counts already include the extra facing taps.
    move(DPAD_UP, 6);
    move(DPAD_LEFT, 8);
    move(DPAD_DOWN, 2);
    move(DPAD_LEFT, 4);
    if (!buy("2F upper vendor", items.floor2_top)){
        return false;
    }
    // The middle vendor is one tile south; face west across the counter.
    move(DPAD_DOWN, 2);
    move(DPAD_LEFT, 1);
    if (!buy("2F middle vendor", items.floor2_middle)){
        return false;
    }
    // Stay west of the stair railing: face east + two tiles, then
    // face south + three tiles onto the mat before entering east.
    move(DPAD_RIGHT, 3);
    move(DPAD_DOWN, 4);
    if (!transition("first floor", [&](ProControllerContext& ctx){
        repeat_dpad(ctx, dpad, DPAD_RIGHT, 80ms, 300ms, 2, false);
    })){
        return false;
    }
    move(DPAD_DOWN, 4);
    // Face west and move three tiles to the exterior doorway column.
    move(DPAD_LEFT, 4);
    return transition("store exit", [&](ProControllerContext& ctx){
        repeat_dpad(ctx, dpad, DPAD_DOWN, 80ms, 300ms, 2, false);
    });
}


}
}
}
