#include <filesystem>
/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include <cmath>
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShopDetectors.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "Detect/PokemonBDSP_AutoStory_OverworldDetector.h"
#include "PokemonBDSP_AutoStoryTools.h"

#include <cstdlib>
#include <unordered_set>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const char* torterra_move_slug(TorterraMove move){
    switch (move){
    case TorterraMove::EARTHQUAKE:  return "earthquake";
    case TorterraMove::WOOD_HAMMER: return "wood-hammer";
    case TorterraMove::RAZOR_LEAF:  return "razor-leaf";
    case TorterraMove::CRUNCH:      return "crunch";
    case TorterraMove::GIGA_DRAIN:  return "giga-drain";
    case TorterraMove::IRON_TAIL:   return "iron-tail";
    case TorterraMove::ROCK_TOMB:   return "rock-tomb";
    case TorterraMove::FACADE:      return "facade";
    case TorterraMove::STONE_EDGE:  return "stone-edge";
    case TorterraMove::ROCK_SLIDE:  return "rock-slide";
    case TorterraMove::BULLDOZE:    return "bulldoze";
    case TorterraMove::STRENGTH:    return "strength";
    case TorterraMove::ROCK_SMASH:  return "rock-smash";
    case TorterraMove::ROCK_CLIMB:  return "rock-climb";
    }
    return "unknown";
}

const char* trainer_slug(Trainer t){
    switch (t){
    case Trainer::Veilstone_GalacticGrunts_2v2: return "Veilstone_GalacticGrunts_2v2";
    }
    return "unknown";
}

bool fake_save_game(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    pbf_mash_button(context, BUTTON_L, 1000ms);

    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_R, 80ms, 1000ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();

    return true;
}

bool save_game(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
) {
    static constexpr size_t MAX_RETRIES = 3;

    for (size_t attempt = 0; attempt < MAX_RETRIES; attempt++) {
        try {
            env.console.log(
                "Checkpoint save attempt: " + std::to_string(attempt + 1)
            );

            // Only mash B on retries - no need to on first attempt
            // if already cleanly in the overworld.
            if (attempt > 0) {
                pbf_mash_button(context, BUTTON_B, 1000ms);
            }

            // Reuse existing feedback-based save: opens menu with MenuWatcher
            // + respects GameSettings::OVERWORLD_TO_MENU_DELAY0 internally.
            save_game(env.console, context);

            // Return to overworld cleanly.
            pbf_mash_button(context, BUTTON_B, 1000ms);

            // Drain R/confirm/B before detection; an earlier overworld frame
            // must not report success while saving commands are still queued.
            context.wait_for_all_requests();

            // Verify overworld. Generous timeout: save animation + fade can be slow.
            OverworldWatcher overworld;
            int ret = wait_until(
                env.console,
                context,
                std::chrono::seconds(15),
                { overworld }
            );
            if (ret < 0) {
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to detect overworld after save.",
                    env.console
                );
            }

            env.console.log("Checkpoint save verified.");
            return true;

        }
        catch (OperationFailedException& e) {
            env.console.log(
                "Checkpoint save failed: " + e.message(),
                COLOR_RED
            );
            context.wait_for(1000ms);
        }
    }

    // Blind fallback: reuse the non-feedback save_game(context) from GameNavigation
    // rather than duplicating the button sequence manually.
    env.console.log(
        "All save retries failed. Using blind fallback.",
        COLOR_ORANGE
    );
    pbf_mash_button(context, BUTTON_B, 1000ms);
    save_game(context);  // PokemonBDSP_GameNavigation.h - non-feedback overload
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();

    return false;
}


void checkpoint_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    CheckpointSaveMode save_mode
){
    const bool fake = save_mode == CheckpointSaveMode::FAKE;
    env.console.log(fake ? "[Checkpoint save] DIAGNOSTIC FAKE mode: save is not verified."
                         : "[Checkpoint save] REAL mode.");
    bool success = fake ? fake_save_game(env, context) : save_game(env, context);

    stats.m_checkpoint++;
    env.update_stats();

    send_program_status_notification(
        env,
        notif_status_update,
        fake ? "Diagnostic fake checkpoint save (not verified)." : success
            ? "Saved at checkpoint (verified)."
            : "Saved at checkpoint (blind fallback)."
    );
}


void reload_backup_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    // TODO: implement backup save reload.
    // This is called when a checkpoint with save=false fails.
    // Load the game's automatic backup save rather than resetting from HOME.
    (void)env;
    (void)context;
}


void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action,
    bool save,
    CheckpointSaveMode save_mode
){
    static constexpr size_t MAX_ATTEMPTS = 20;

    for (size_t i = 0;; i++){
        try{
            if (i == 0){
                if (save || stats.m_first_run){
                    checkpoint_save(env, context, notif_status_update, stats, save_mode);
                }
                stats.m_first_run = false;
            }

            action(i);
            return; // success

        }catch (OperationFailedException& e){
            env.console.log("checkpoint_reattempt_loop: attempt " + std::to_string(i) + " failed: " + e.message(), COLOR_RED);
            stats.m_reset++;
            env.update_stats();

            if (i + 1 >= MAX_ATTEMPTS){
                throw;
            }

            if (save){
                env.console.log("Resetting game from HOME before retry...", COLOR_ORANGE);
                bool ok = reset_game_from_home(env, env.console, context, true, 1000ms);
                if (!ok){
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to reset game from HOME during checkpoint retry.",
                        env.console
                    );
                }
            } else {
                env.console.log("Reloading from backup save before retry...", COLOR_ORANGE);
                reload_backup_save(env, context);
            }
        }
    }
}



void repeat(std::initializer_list<std::function<void()>> actions, size_t times){
    for (size_t i = 0; i < times; i++){
        for (const auto& action : actions){
            action();
        }
    }
}


void wait_for_dialogue(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label,
    Milliseconds timeout
){
    auto log_detected = [&]{
        if (!label.empty()){
            stream.log(label + " dialogue detected.");
        }
    };

    ShortDialogWatcher watcher_cyan(COLOR_CYAN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [timeout](ProControllerContext& context){
            context.wait_for(timeout);
        },
        {{watcher_cyan}}
    );
    if (ret == 0){ log_detected(); return; }

    ShortDialogWatcher watcher_red(COLOR_RED);
    ret = run_until<ProControllerContext>(
        stream, context,
        [timeout](ProControllerContext& context){
            context.wait_for(timeout);
        },
        {{watcher_red}}
    );
    if (ret == 0){ log_detected(); return; }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        (label.empty() ? "wait_for_dialogue" : label) + ": dialogue not detected within timeout.",
        stream
    );
}


void mash_until_dialogue_ends(
    VideoStream& stream,
    ProControllerContext& context,
    Button button,
    Milliseconds timeout
){
    OverworldWatcher overworld;
    int ret = run_until<ProControllerContext>(
        stream, context,
        [button, timeout](ProControllerContext& context){
            pbf_mash_button(context, button, timeout);
        },
        {{overworld}}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "mash_until_dialogue_ends: overworld not reached within timeout.",
            stream
        );
    }
}


void repeat_dpad(
    ProControllerContext& context,
    DpadState& state,
    DpadPosition dir,
    Milliseconds press,
    Milliseconds hold,
    size_t times,
    bool include_last_dir
){
    if (include_last_dir && state.last_dir != DPAD_NONE && state.last_dir != dir){
        times += 1;
    }
    state.last_dir = dir;
    for (size_t i = 0; i < times; i++){
        pbf_press_dpad(context, dir, press, hold);
    }
}

static const char* shop_item_name(ShopItem item){
    switch (item){
    case ShopItem::FullRestore:  return "Full restore";
    case ShopItem::MaxPotion:    return "Max potion";
    case ShopItem::HyperPotion:  return "Hyper potion";
    case ShopItem::SuperPotion:  return "Super potion";
    case ShopItem::Potion:       return "Potion";
    case ShopItem::Antidote:     return "Antidote";
    case ShopItem::ParalyzHeal:  return "Paralyz heal";
    case ShopItem::Awakening:    return "Awakening";
    case ShopItem::Repel:        return "Repel";
    case ShopItem::SuperRepel:   return "Super repel";
    case ShopItem::MaxRepel:     return "Max repel";
    case ShopItem::EscapeRope:   return "Escape rope";
    default:                     return "Unknown";
    }
}

// Returns the OCR slug used as the dictionary key in ShopItemNameOCR.json.
static const char* shop_item_slug(ShopItem item){
    switch (item){
    case ShopItem::FullRestore:  return "full-restore";
    case ShopItem::MaxPotion:    return "max-potion";
    case ShopItem::HyperPotion:  return "hyper-potion";
    case ShopItem::SuperPotion:  return "super-potion";
    case ShopItem::Potion:       return "potion";
    case ShopItem::Antidote:     return "antidote";
    case ShopItem::ParalyzHeal:  return "paralyze-heal";
    case ShopItem::Awakening:    return "awakening";
    case ShopItem::Repel:        return "repel";
    case ShopItem::SuperRepel:   return "super-repel";
    case ShopItem::MaxRepel:     return "max-repel";
    case ShopItem::EscapeRope:   return "escape-rope";
    default:                     return "unknown";
    }
}

// {MaxRepel, 2, 4}, {FullRestore, 0, 3} â†’ "4 Max repel, 3 Full restore"
static std::string make_buy_label(const std::vector<ItemToBuy>& items){
    std::string label;
    for (size_t i = 0; i < items.size(); i++){
        if (i > 0) label += ", ";
        if (items[i].quantity == BUY_MAX){
            label += "max ";
        } else {
            label += std::to_string(items[i].quantity) + " ";
        }
        label += shop_item_name(items[i].item);
    }
    return label;
}

bool buy_items(
    VideoStream& stream,
    ProControllerContext& context,
    const std::vector<ItemToBuy>& items,
    Language language
){
    if (items.empty()){
        return true;
    }

    const std::string label = make_buy_label(items);
    context.wait_for_all_requests();
    pbf_wait(context, 500ms);
    pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
    context.wait_for_all_requests();


    // â”€â”€ Coordinate constants â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

    // Area where the selection arrow appears at the vendor greeting menu (Buy/Sell/Leave).
    static const ImageFloatBox VENDOR_ARROW_BOX{0.630000, 0.550000, 0.070000, 0.120000};

    // Area where the selection arrow appears at the YES/NO confirmation dialog.
    static const ImageFloatBox CONFIRM_ARROW_BOX{0.700000, 0.620000, 0.070000, 0.120000};

    // Full area containing all item rows in the shop list.
    // The hover detector scans this region to dynamically locate the red outline.
    static const ImageFloatBox ITEM_SCAN_AREA{0.417000, 0.100000, 0.540000, 0.625000};

    static constexpr size_t MAX_OUTER       = 50;
    static constexpr size_t MAX_ITEM_SEARCH = 50;  // max iterations per item before giving up

    // â”€â”€ Outer state machine â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    for (size_t outer = 0; outer < MAX_OUTER; outer++){
        SelectionArrowFinder  arrow_vendor(stream.overlay(), VENDOR_ARROW_BOX, COLOR_GREEN);
        ShopBuyingWindowWatcher buying_window;
        ShortDialogWatcher    dialog(COLOR_RED);

        // case 0: no detectors fired â€” press A in a for-loop (5 presses) while watching.
        stream.log("Starting to loop over items");
        int state = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& ctx){
                for (int i = 0; i < 5; i++){
                    pbf_press_button(ctx, BUTTON_A, 80ms, 1000ms);
                }
            },
            {{arrow_vendor}, {buying_window}, {dialog}}
        );

        context.wait_for_all_requests();
        pbf_wait(context, 500ms);

        switch (state){

        case 0:  // case 1: SelectionArrow at vendor menu â†’ press A to choose "Buy"
            stream.log("Found the selection arrow, pressing A to enter the buying window...", COLOR_GREEN);
            pbf_press_button(context, BUTTON_A, 80ms, 500ms);
            break;

        case 1:  // case 2: buying window open â†’ buy each requested item
        stream.log("Found the buying window, searching for the right item...", COLOR_GREEN);
        {
            bool buying_failed = false;

            for (const ItemToBuy& item : items){
                stream.log("Buying " + std::to_string(item.quantity) + "x " + shop_item_slug(item.item));
                if (buying_failed) break;

                // Detector fires only when the red outline is found AND both
                // name and price OCR match the expected item slug.
                ShopItemHoverDetector     hover(stream.logger(), ITEM_SCAN_AREA,
                                               shop_item_slug(item.item), language, COLOR_CYAN);
                ShopQuantityWindowWatcher qty_window;
                SelectionArrowFinder      confirm_arrow(stream.overlay(), CONFIRM_ARROW_BOX, COLOR_GREEN);

                bool item_bought     = false;
                bool hover_confirmed = false;
                for (size_t inner = 0; inner < MAX_ITEM_SEARCH && !item_bought; inner++){
                    context.wait_for_all_requests();

                    int sub = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& ctx){
                            ctx.wait_for(1000ms);
                        },
                        {{confirm_arrow}, {qty_window}, {hover}}
                    );

                    context.wait_for_all_requests();

                    stream.log("buy_items: sub=" + std::to_string(sub), COLOR_WHITE);

                    switch (sub){

                    case 0:  // YES/NO confirmation arrow â†’ confirm purchase
                        stream.log("Found the confirmation to buy arrow, pressing A to buy", COLOR_GREEN);
                    {
                        context.wait_for_all_requests();
                        ShopBuyingWindowWatcher back_in_shop;
                        run_until<ProControllerContext>(
                            stream, context,
                            [](ProControllerContext& ctx){
                                for (int i = 0; i < 10; i++){
                                    pbf_press_button(ctx, BUTTON_A, 80ms, 800ms);
                                }
                            },
                            {{back_in_shop}}
                        );
                        item_bought = true;
                        break;
                    }

                    case 1:  // quantity window
                        if (!hover_confirmed){
                            // Quantity window appeared before we confirmed the right item â€”
                            // an earlier A press accidentally opened the wrong item's selector.
                            // Press B to go back to the list and continue searching.
                            stream.log("Unexpected quantity window (wrong item), pressing B...", COLOR_YELLOW);
                            pbf_press_button(context, BUTTON_B, 80ms, 800ms);
                            break;
                        }
                        stream.log("Found the quantity window", COLOR_GREEN);
                        if (item.quantity == BUY_MAX){
                            stream.log("Buying max quantity");
                            pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                        } else {
                            stream.log("Buying " + std::to_string(item.quantity) + " of the item");
                            for (int q = 1; q < item.quantity; q++){
                                pbf_press_dpad(context, DPAD_UP, 80ms, 300ms);
                            }
                        }
                        pbf_press_button(context, BUTTON_A, 80ms, 300ms);
                        pbf_wait(context, 1000ms);
                        break;

                    case 2:  // correct item highlighted â†’ press A to open quantity selector
                        stream.log("Found the right item, pressing A to buy a certain quantity...", COLOR_GREEN);
                        hover_confirmed = true;
                        pbf_press_button(context, BUTTON_A, 80ms, 300ms);
                        pbf_wait(context, 1000ms);
                        break;

                    default:  // outline not found or wrong item â†’ advance to next row
                        stream.log("Not on the right item, moving...", COLOR_GREEN);
                        pbf_press_dpad(context, DPAD_DOWN, 80ms, 300ms);
                        pbf_wait(context, 300ms);
                        break;
                    }
                }

                if (!item_bought){
                    stream.log(label + ": buy_items: could not locate " +
                               shop_item_name(item.item) + " in shop list.", COLOR_RED);
                    buying_failed = true;
                }
            }

            if (!buying_failed){
                stream.log("All items bought, exiting shop...", COLOR_GREEN);
                pbf_mash_button(context, BUTTON_B, 5000ms);
                return true;
            }
            break;
        }

        case 2:  // case 3: unexpected ShortDialog â†’ press B until back in buying window
        {
            ShopBuyingWindowWatcher back_in_shop;
            run_until<ProControllerContext>(
                stream, context,
                [](ProControllerContext& ctx){
                    pbf_mash_button(ctx, BUTTON_B, 5000ms);
                },
                {{back_in_shop}}
            );
            break;
        }

        default:  // outer default: unrecognised state â†’ press B to recover
            pbf_press_button(context, BUTTON_B, 80ms, 3000ms);
            break;
        }
    }

    stream.log(label + ": buy_items: failed to complete after max iterations.", COLOR_RED);
    return false;
}


bool heal_pokemon(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label
){
    context.wait_for_all_requests();
    pbf_wait(context, 2000ms);

    pbf_move_left_joystick(context, {0, +1}, 1800ms, 100ms); // 7+
    context.wait_for_all_requests();

    pbf_mash_button(context, BUTTON_A, 5000ms);
    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    wait_for_dialogue(stream, context, label + " - Pokemon healing");
    mash_until_dialogue_ends(stream, context, BUTTON_B);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    // Exiting the Pokemon Center
    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 15000ms, 100ms);
        },
        {{black_screen}}
    );
    if (ret < 0){
        stream.log("heal_pokemon(" + label + "): black screen not detected", COLOR_RED);
        return false;
    }

    return true;
}

std::string fly_point_name(FlyPoint place) {
    switch (place) {
        case FlyPoint::CanalavCity:        return "canalave_city";
        case FlyPoint::CelesticTown:      return "celestic_town";
        case FlyPoint::EternaCity:         return "eterna_city";
        case FlyPoint::FloaromaTown:       return "floaroma_town";
        case FlyPoint::HearthomeCity:      return "hearthome_city";
        case FlyPoint::JubilifeCity:       return "jubilife_city";
        case FlyPoint::OreburghCity:       return "oreburgh_city";
        case FlyPoint::PastoriaCity:       return "pastoria_city";
        case FlyPoint::PokemonLeagueLower: return "pokemon_league_lower";
        case FlyPoint::PokemonLeagueUpper: return "pokemon_league_upper";
        case FlyPoint::Route221:           return "route_221";
        case FlyPoint::SandgemTown:        return "sandgem_town";
        case FlyPoint::SolaceonTown:       return "solaceon_town";
        case FlyPoint::SnowpointCity:      return "snowpoint_city";
        case FlyPoint::SunyshoreCity:      return "sunyshore_city";
        case FlyPoint::TwinleafTown:       return "twinleaf_town";
        case FlyPoint::VeilstoneCity:      return "veilstone_city";
        case FlyPoint::CurrentCity:        return "current_city";
    }
    return "unknown";
}

bool fly_to(
    VideoStream& stream,
    ProControllerContext& context,
    FlyPoint place
){
    // Each city branch below should update this to the city icon's position on the fly map.
    // Detection: when the cursor brackets overlap the icon, its normally uniform
    // color becomes non-uniform (stddev rises). Box stays as a rough fallback
    // until each city is calibrated.
    ImageFloatBox city_icon_box{0.700000, 0.550000, 0.100000, 0.120000};

    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    open_menu(stream, context, MenuCursorPosition::MAP, 8);

    if (place == FlyPoint::CurrentCity) {
        ImageFloatBox button_a_box{0.540000, 0.960000, 0.070000, 0.035000};
        struct FlyAButtonDetector : VisualInferenceCallback {
            ImageFloatBox m_box;
            FlyAButtonDetector(const ImageFloatBox& box)
                : VisualInferenceCallback("FlyAButtonDetector"), m_box(box) {}
            void make_overlays(VideoOverlaySet&) const override {}
            bool process_frame(const ImageViewRGB32& frame, WallClock) override {
                ImageStats s = image_stats(extract_box_reference(frame, m_box));
                return s.stddev.sum() > 30.0;
            }
        };
        FlyAButtonDetector watcher(button_a_box);
        int ret = wait_until(stream, context, 3000ms, {{watcher}});
        if (ret < 0) {
            stream.log("CurrentCity: A button not found.", COLOR_RED);
            return false;
        }
        BlackScreenOverWatcher flying_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int fly_ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){ pbf_mash_button(context, BUTTON_A, 200000ms); },
            {{flying_black_screen}}
        );
        if (fly_ret < 0) {
            stream.log("CurrentCity: black screen not detected!", COLOR_RED);
            return false;
        }
        stream.log("CurrentCity: black screen detected, arrived.", COLOR_GREEN);
        return true;
    }

    switch (place) {
        case FlyPoint::CelesticTown:
            pbf_move_left_joystick(context, {-1, +1}, 4000ms, 500ms);
            break;
        case FlyPoint::HearthomeCity:
        case FlyPoint::OreburghCity:
            pbf_move_left_joystick(context, {-1, -1}, 4000ms, 500ms);
            break;
        case FlyPoint::PastoriaCity:
        case FlyPoint::PokemonLeagueLower:
        case FlyPoint::PokemonLeagueUpper:
        case FlyPoint::SolaceonTown:
        case FlyPoint::SunyshoreCity:
        case FlyPoint::VeilstoneCity:
            pbf_move_left_joystick(context, {+1, -1}, 4000ms, 500ms);
            break;
        case FlyPoint::SnowpointCity:
            pbf_move_left_joystick(context, {-1, +1}, 4000ms, 500ms);
            break;
        case FlyPoint::TwinleafTown:
            pbf_move_left_joystick(context, {-1, -1}, 4000ms, 500ms);
            break;
        default:
            pbf_move_left_joystick(context, {-1, -1}, 4000ms, 500ms);
            break;
    }
    context.wait_for_all_requests();

    ImageFloatBox button_a_box{0.540000, 0.960000, 0.070000, 0.035000};

    struct FlyAButtonDetector : VisualInferenceCallback {
        ImageFloatBox m_box;
        FlyAButtonDetector(const ImageFloatBox& box)
            : VisualInferenceCallback("FlyAButtonDetector"), m_box(box) {}
        void make_overlays(VideoOverlaySet&) const override {}
        bool process_frame(const ImageViewRGB32& frame, WallClock) override {
            ImageStats s = image_stats(extract_box_reference(frame, m_box));
            // Blue stripe is uniform when no button shown; A button disrupts it
            return s.stddev.sum() > 30.0;
        }
    };
    FlyAButtonDetector watcher_1(button_a_box);

    switch (place) {
        case FlyPoint::CanalavCity:
            break;
        case FlyPoint::CelesticTown:
            city_icon_box = {0.660000, 0.470000, 0.012000, 0.020000};
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 15, false);
            repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 13, false);
            break;
        case FlyPoint::EternaCity:
            break;
        case FlyPoint::FloaromaTown:
            break;
        case FlyPoint::HearthomeCity:
            city_icon_box = {0.660000, 0.637000, 0.029000, 0.052000};
            repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 16, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 7, false);
            break;
        case FlyPoint::JubilifeCity:
            city_icon_box = {0.453000, 0.702000, 0.016000, 0.054000};
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 4, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 5, false);
            break;
        case FlyPoint::OreburghCity:
            city_icon_box = {0.566000, 0.702000, 0.012000, 0.054000};
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 10, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 5, false);
            break;
        case FlyPoint::PastoriaCity:
            break;
        case FlyPoint::PokemonLeagueLower:
            break;
        case FlyPoint::PokemonLeagueUpper:
            break;
        case FlyPoint::Route221:
            break;
        case FlyPoint::SandgemTown:
            break;
        case FlyPoint::SolaceonTown:
            city_icon_box = {0.735000, 0.603000, 0.030000, 0.022000};
            repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 12, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 9, false);
            break;
        case FlyPoint::SnowpointCity:
            city_icon_box = {0.602000, 0.070000, 0.013000, 0.054000};
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 12, false);
            repeat_dpad(context, dpad, DPAD_DOWN, 80ms, 300ms, 1, false);
            break;
        case FlyPoint::SunyshoreCity:
            break;
        case FlyPoint::TwinleafTown:
            city_icon_box = {0.415000, 0.870000, 0.013000, 0.022000};
            repeat_dpad(context, dpad, DPAD_RIGHT, 80ms, 300ms, 2, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 1, false);
            break;
        case FlyPoint::VeilstoneCity:
            city_icon_box = {0.829000, 0.537000, 0.017000, 0.054000};
            repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 8, false);
            repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 10, false);
            break;
        case FlyPoint::CurrentCity:
            break;
    }

    context.wait_for_all_requests();

    struct CityIconDetector : VisualInferenceCallback {
        ImageFloatBox m_box;
        CityIconDetector(const ImageFloatBox& box)
            : VisualInferenceCallback("CityIconDetector"), m_box(box) {}
        void make_overlays(VideoOverlaySet&) const override {}
        bool process_frame(const ImageViewRGB32& frame, WallClock) override {
            ImageStats s = image_stats(extract_box_reference(frame, m_box));
            // City icon is normally a uniform color; cursor brackets disrupt it
            return s.stddev.sum() > 30.0;
        }
    };
    CityIconDetector watcher_2(city_icon_box);

    const std::string name = fly_point_name(place);
    int ret_1 = wait_until(stream, context, 3000ms, {{watcher_1}});
    int ret_2 = wait_until(stream, context, 4000ms, {{watcher_2}});
    if (ret_1 < 0) {
        stream.log("A button not found, retrying...", COLOR_RED);
    };
    if (ret_2 < 0) {
        stream.log(name + " not found, retrying...", COLOR_RED);
    };
    if (ret_1 == 0 and ret_2 == 0){
        stream.log("Found " + name + ", flying towards it.", COLOR_GREEN);
        WhiteScreenOverWatcher flying_white_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        BlackScreenOverWatcher flying_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{flying_white_screen}, {flying_black_screen}}
        );
        if (ret < 0){
            stream.log("Flying to " + name + ": white / black screen not detected!", COLOR_RED);
            return false;
        } else if (ret >= 0) {
            stream.log("Flying to " + name + ": white / black screen detected!", COLOR_GREEN);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Arrow - Based Potion Usage Helper

// This version assumes :
//
// you are already inside the Bag
// Potion has already been selected
// the game is currently waiting for you to choose a Pokemon
//
// Flow :
// 1. Move LEFT once to Pokemon list
// 2. Press A on first Pokemon
// 3. Use SelectionArrow to detect arrow position
// 4. Move DOWN until arrow reaches slot 3
// 5. Mash A for 3 seconds
// 6. Mash B for 10 seconds
//
// ---------------------------------------------------------------------------

// Slot 1's colored HP bar in the Bag. Read fill position rather than OCR.
static double potion_hp_fraction(const ImageViewRGB32& frame) {
    if (bag_pocket_position(frame) == 0) return -1;
    const ImageViewRGB32 bar = extract_box_reference(frame, ImageFloatBox{0.137, 0.160, 0.202, 0.016});
    size_t filled = 0;
    for (size_t x = 0; x < bar.width(); x++) {
        size_t color = 0;
        for (size_t y = 0; y < bar.height(); y++) {
            uint32_t p = bar.pixel(x, y);
            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
            const bool green = g > r + 30 && g > b + 20;
            const bool yellow = r > 150 && g > 120 && b < 100;
            const bool red = r > 150 && r > g + 50 && r > b + 50;
            color += green || yellow || red;
        }
        filled += color > bar.height() / 4;
    }
    return bar.width() ? (double)filled / bar.width() : -1;
}

static double confirmed_potion_hp(VideoStream& stream, ProControllerContext& context) {
    double previous = -1;
    size_t stable = 0;
    for (size_t check = 0; check < 20; check++) {
        context.wait_for_all_requests();
        auto frame = stream.video().snapshot();
        double hp = frame ? potion_hp_fraction(frame) : -1;
        if (hp >= 0 && previous >= 0 && std::abs(hp - previous) <= 0.015) {
            if (++stable >= 3) return hp;
        } else stable = 0;
        previous = hp;
        pbf_wait(context, 150ms);
    }
    OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
        "Potion: Slot 1 HP bar could not be confirmed.", stream);
}

void use_potion_first_pokemon(VideoStream& stream, ProControllerContext& context) {
    OverlayBoxScope hp_overlay(stream.overlay(), COLOR_GREEN, {0.137, 0.160, 0.202, 0.016}, "");
    BagItemTemplate potion;
    potion.reference_path = "PokemonBDSP/Autostory/Bag_Potion.png";
    potion.label = "Potion";
    potion.reference_row = 1;
    bool healed = false;
    for (size_t dose = 0; dose < 3; dose++) {
        stream.log("[Potion] Finding Bag/Medicine/Potion; dose " + std::to_string(dose + 1) + "/3.");
        if (!find_bag_item(stream, context, potion, 1, 5, 64, std::chrono::seconds(45))) {
            OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
                "Potion: Could not verify Potion in the Medicine pocket.", stream);
        }
        const double before = confirmed_potion_hp(stream, context);
        stream.log("[Potion] Slot 1 HP bar before use=" + std::to_string(before * 100) + "%.");
        if (before >= 0.98) { healed = true; break; }
        if (before <= 0.005) {
            OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
                "Potion: Slot 1 appears fainted; a Potion cannot revive it.", stream);
        }
        pbf_press_button(context, BUTTON_A, 80ms, 500ms);
        pbf_press_button(context, BUTTON_A, 80ms, 500ms);
        pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
        pbf_mash_button(context, BUTTON_B, 2000ms);
        context.wait_for_all_requests();
        OverworldWatcher overworld;
        if (wait_until(stream, context, std::chrono::seconds(10), {{overworld}}) != 0) {
            OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
                "Potion: Did not return to the overworld after use.", stream);
        }
        // Reopen Bag to verify the result, including when the last Potion was used.
        open_menu(stream, context, MenuCursorPosition::BAG, 5);
        pbf_wait(context, 750ms);
        context.wait_for_all_requests();
        const double after = confirmed_potion_hp(stream, context);
        stream.log("[Potion] Slot 1 HP bar after use=" + std::to_string(after * 100) + "%.");
        if (after <= before + 0.005) {
            OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
                "Potion: Slot 1 HP did not increase; refusing to continue movement.", stream);
        }
        if (after >= 0.98) { healed = true; break; }
        pbf_mash_button(context, BUTTON_B, 2000ms);
        context.wait_for_all_requests();
    }
    if (!healed) OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
        "Potion: Full HP not confirmed after three uses.", stream);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
    OverworldWatcher overworld;
    if (wait_until(stream, context, std::chrono::seconds(10), {{overworld}}) != 0)
        OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
            "Potion: Final overworld return not confirmed.", stream);
    stream.log("[Potion] Slot 1 HP bar full and overworld confirmed.");
}


namespace{

struct IconGrid { int row, col; };

IconGrid icon_to_grid(MenuCursorPosition pos, int num_icons){
    switch (pos){
    case MenuCursorPosition::POKEDEX:      return {0, 0};
    case MenuCursorPosition::POKEMON:      return {0, 1};
    case MenuCursorPosition::BAG:          return {0, 2};
    case MenuCursorPosition::CARD:         return {0, 3};
    case MenuCursorPosition::OPTIONS:      return {1, num_icons <= 5 ? 0 : 2};
    case MenuCursorPosition::MAP:          return {1, 0};
    case MenuCursorPosition::CAPSULES:     return {1, 1};
    case MenuCursorPosition::MYSTERYGIFT:  return {1, 3};
    default:                               return {-1, -1};
    }
}

} // anonymous namespace


void open_menu(
    VideoStream& stream,
    ProControllerContext& context,
    MenuCursorPosition target,
    int num_icons
){
    using namespace std::chrono_literals;

    // Previous single opening attempt:
    // context.wait_for_all_requests();
    // overworld_to_menu(stream, context);
    // pbf_wait(context, 1000ms);
    // context.wait_for_all_requests();
    context.wait_for_all_requests();

    MenuCursorDetector detector;
    {
        static const char* ICON_NAMES[MENU_CURSOR_NUM_ICONS] = {
            "POKEDEX", "POKEMON", "BAG", "CARD",
            "OPTIONS", "MAP", "CAPSULES", "MYSTERYGIFT",
        };
        // Previous optional template directory: PokemonBDSP/AutoStory.
        std::string ref_dir = RESOURCE_PATH() + "PokemonBDSP/Autostory";
        for (int i = 0; i < MENU_CURSOR_NUM_ICONS; i++){
            std::string path = ref_dir + "/" + ICON_NAMES[i] + ".png";
            // These templates are optional. The dark-pixel cursor detector
            // remains available when a template is not packaged.
            std::error_code file_error;
            if (!std::filesystem::is_regular_file(path, file_error)){
                continue;
            }
            // Previously constructed the image before checking its availability.
            ImageRGB32 img(path);
            if (img){
                detector.set_icon_reference(
                    static_cast<MenuCursorPosition>(i),
                    std::move(img)
                );
            }
        }
    }
    // Previous single detection attempt retained for comparison.
    // MenuCursorPosition cur = MenuCursorPosition::UNKNOWN;
    // for (int i = 0; i < 10 && cur == MenuCursorPosition::UNKNOWN; i++){
    //     context.wait_for(50ms);
    //     VideoSnapshot snap = stream.video().snapshot();
    //     if (snap) cur = detector.detect(snap);
    // }
    // if (cur == MenuCursorPosition::UNKNOWN){
    //     OperationFailedException::fire(
    //         ErrorReport::SEND_ERROR_REPORT,
    //         "Menu cursor not detected after opening menu.",
    //         stream
    //     );
    // }

    MenuCursorPosition cur = MenuCursorPosition::UNKNOWN;
    for (size_t attempt = 1; attempt <= 3; attempt++){
        stream.log("[Open menu] Open and detect attempt " + std::to_string(attempt) + "/3.");
        try {
            // A late-opening menu may already be visible after a timeout.
            // Check before pressing X again, since X would close that menu.
            MenuWatcher already_open;
            if (wait_until(stream, context, 100ms, {{already_open}}) < 0){
                overworld_to_menu(stream, context);
            }
            pbf_wait(context, 1000ms);
            context.wait_for_all_requests();
            for (int frame = 0; frame < 10 && cur == MenuCursorPosition::UNKNOWN; frame++){
                context.wait_for(50ms);
                const VideoSnapshot snapshot = stream.video().snapshot();
                if (snapshot){
                    cur = detector.detect(snapshot);
                }
            }
            if (cur != MenuCursorPosition::UNKNOWN){
                stream.log("[Open menu] Menu and cursor detected on attempt " + std::to_string(attempt) + ".", COLOR_GREEN);
                break;
            }
            stream.log("[Open menu] Menu cursor not detected on attempt " + std::to_string(attempt) + ".", COLOR_ORANGE);
        }
        catch (OperationFailedException& error){
            stream.log("[Open menu] Attempt " + std::to_string(attempt) + " failed: " + error.message(), COLOR_ORANGE);
        }
    }
    if (cur == MenuCursorPosition::UNKNOWN){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Menu or cursor detection failed after 3 opening attempts.",
            stream
        );
    }

    IconGrid cur_g = icon_to_grid(cur,    num_icons);
    IconGrid tgt_g = icon_to_grid(target, num_icons);
    if (tgt_g.row == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unknown target menu icon.",
            stream
        );
    }

    if (cur_g.row != tgt_g.row){
        DpadPosition vert = (cur_g.row < tgt_g.row) ? DPAD_DOWN : DPAD_UP;
        pbf_press_dpad(context, vert, 80ms, 300ms);
        context.wait_for_all_requests();
    }

    int col_diff = tgt_g.col - cur_g.col;
    if (col_diff != 0){
        DpadPosition horiz = (col_diff > 0) ? DPAD_RIGHT : DPAD_LEFT;
        for (int i = 0; i < std::abs(col_diff); i++){
            pbf_press_dpad(context, horiz, 80ms, 300ms);
            context.wait_for_all_requests();
        }
    }

    pbf_press_button(context, BUTTON_A, 80ms, 200ms);
}


// Search the selected item's fixed description header rather than guessing
// which list row is selected. This remains in place while the list scrolls.
// A cursor parked at row 2 or 6 does not indicate a list end: only repeated
// unchanged item headers after movement are used to detect an end.
// Previous selected-item-only search, retained for reference.
// bool find_bag_item(
//     VideoStream& stream,
//     ProControllerContext& context,
//     const BagItemTemplate& item,
//     int num_icons,
//     size_t max_scrolls_per_pocket,
//     std::chrono::milliseconds search_timeout
// ){
//     auto reference = std::make_shared<const ImageRGB32>(
//         RESOURCE_PATH() + item.reference_path
//     );
//     ImageMatchDetector icon(reference, item.icon_box, item.max_icon_rmsd, true);
//     ImageMatchDetector name(reference, item.name_box, item.max_name_rmsd, true);
//     ImageMatchDetector bag(reference, ImageFloatBox(0.055, 0.012, 0.044, 0.045), 35.0, true);
//     const ImageFloatBox selected_header(0.428, 0.754, 0.382, 0.046);
//     const auto deadline = std::chrono::steady_clock::now() + search_timeout;
// 
//     stream.log("[Bag search] Looking for " + item.label + ".");
//     open_menu(stream, context, MenuCursorPosition::BAG, num_icons);
//     pbf_wait(context, 1000ms);
//     context.wait_for_all_requests();
// 
//     for (size_t pocket = 0; pocket < 8; pocket++){
//         ImageRGB32 initial_header;
//         ImageRGB32 previous_header;
//         size_t unchanged = 0;
//         size_t matching_frames = 0;
//         size_t scrolls = 0;
//         bool scanning_up = false;
// 
//         while (std::chrono::steady_clock::now() < deadline){
//             context.wait_for_all_requests();
//             const VideoSnapshot frame = stream.video().snapshot();
//             if (!frame || !bag.detect(frame)){
//                 stream.log("[Bag search] Bag screen not detected.", COLOR_ORANGE);
//                 return false;
//             }
//             const double icon_score = icon.rmsd(frame);
//             const double name_score = item.check_name ? name.rmsd(frame) : 0.0;
//             stream.log(
//                 "[Bag search] Pocket " + std::to_string(pocket + 1) +
//                 ", scroll " + std::to_string(scrolls) +
//                 ": icon=" + std::to_string(icon_score) +
//                 ", name=" + std::to_string(name_score)
//             );
//             if (icon_score <= item.max_icon_rmsd &&
//                 name_score <= item.max_name_rmsd){
//                 if (++matching_frames >= 3){
//                     stream.log("[Bag search] " + item.label + " confirmed.", COLOR_GREEN);
//                     return true;
//                 }
//                 pbf_wait(context, 150ms);
//                 continue;
//             }
//             matching_frames = 0;
// 
//             const ImageRGB32 header = extract_box_reference(frame, selected_header).scale_to(382, 46);
//             if (!initial_header){
//                 initial_header = header.copy();
//             }
//             else if (scrolls > 0){
//                 const bool same_as_previous =
//                     ImageMatch::pixel_RMSD(previous_header, header) <= 3.0;
//                 const bool returned_to_start =
//                     ImageMatch::pixel_RMSD(initial_header, header) <= 3.0;
//                 if (returned_to_start && !same_as_previous){
//                     break;
//                 }
//                 // Repeated unchanged rows mean an empty pocket or list end.
//                 // Returning to the initial row means the list wrapped.
//                 unchanged = same_as_previous || returned_to_start ? unchanged + 1 : 0;
//                 if (unchanged >= 3){
//                     if (scanning_up){
//                         break;
//                     }
//                     // A non-wrapping list may have opened in the middle.
//                     // Search upward as well before moving to another pocket.
//                     scanning_up = true;
//                     unchanged = 0;
//                     initial_header = header.copy();
//                 }
//             }
//             previous_header = header.copy();
//             if (scrolls >= max_scrolls_per_pocket){
//                 stream.log("[Bag search] Scroll limit reached; search incomplete.", COLOR_ORANGE);
//                 return false;
//             }
//             scrolls++;
//             pbf_press_dpad(context, scanning_up ? DPAD_UP : DPAD_DOWN, 80ms, 350ms);
//         }
//         if (std::chrono::steady_clock::now() >= deadline){
//             stream.log("[Bag search] Search timed out; search incomplete.", COLOR_ORANGE);
//             return false;
//         }
//         if (pocket < 7){
//             pbf_press_dpad(context, DPAD_RIGHT, 80ms, 500ms);
//             context.wait_for_all_requests();
//         }
//     }
//     stream.log("[Bag search] Item not found in any pocket.", COLOR_ORANGE);
//     return false;
// }
// 

// One narrow strip on the left side of the red selection border. Divide its
// height into the seven row bands; the item's art and NEW badge are outside it.
int bag_cursor_position(const ImageViewRGB32& frame){
    if (!frame){
        return 0;
    }
    const ImageViewRGB32 strip = extract_box_reference(
        frame, ImageFloatBox(0.431, 0.229, 0.005, 0.470)
    );
    double best = 0.0;
    double second = 0.0;
    int position = 0;
    for (int row = 0; row < 7; row++){
        const size_t top = static_cast<size_t>(strip.height() * (row * 0.075 / 0.470));
        const size_t bottom = static_cast<size_t>(strip.height() * ((row * 0.075 + 0.020) / 0.470));
        size_t red = 0;
        size_t total = 0;
        for (size_t y = top; y < bottom && y < strip.height(); y++){
            for (size_t x = 0; x < strip.width(); x++){
                const uint32_t pixel = strip.pixel(x, y);
                const unsigned r = (pixel >> 16) & 255;
                const unsigned g = (pixel >> 8) & 255;
                const unsigned b = pixel & 255;
                red += r > 180 && g < 120 && b < 120;
                total++;
            }
        }
        const double score = total == 0 ? 0.0 : static_cast<double>(red) / total;
        if (score > best){
            second = best;
            best = score;
            position = row + 1;
        }else if (score > second){
            second = score;
        }
    }
    return best >= 0.15 && best - second >= 0.10 ? position : 0;
}

static ImageFloatBox bag_list_icon_box(size_t row){
    return ImageFloatBox(0.449, 0.212 + (row - 1) * 0.075, 0.033, 0.050);
}

static ImageFloatBox bag_list_name_box(size_t row){
    // Previous height included the faint lower selection border in rows 2/6.
    // return ImageFloatBox(0.492, 0.213 + (row - 1) * 0.075, 0.335, 0.052);
    return ImageFloatBox(0.492, 0.213 + (row - 1) * 0.075, 0.335, 0.050);
}

static double bag_row_patch_score(
    const ImageRGB32& reference,
    const ImageViewRGB32& frame,
    const ImageFloatBox& box
){
    const ImageRGB32 live = extract_box_reference(frame, box).scale_to(
        reference.width(), reference.height()
    );
    return ImageMatch::pixel_RMSD(reference, live);
}

// Previous name-area empty-list check retained for reference.
// static bool bag_list_is_empty(const ImageViewRGB32& frame){
//     const ImageViewRGB32 names = extract_box_reference(
//         frame, ImageFloatBox(0.492, 0.213, 0.335, 0.500)
//     );
//     size_t dark = 0;
//     for (size_t y = 0; y < names.height(); y++){
//         for (size_t x = 0; x < names.width(); x++){
//             const uint32_t pixel = names.pixel(x, y);
//             dark += ((pixel >> 16) & 255) < 100 &&
//                 ((pixel >> 8) & 255) < 100 && (pixel & 255) < 100;
//         }
//     }
//     return dark < names.width() * names.height() * 0.001;
// }

int bag_pocket_position(const ImageViewRGB32& frame){
    if (!frame) return 0;
    int selected = 0;
    for (int pocket = 1; pocket <= 8; pocket++){
        // The lower left edge of each tab is below the red NEW badge.
        // This strip includes both the normal outline and enlarged selection.
        const auto edge = extract_box_reference(frame,
            ImageFloatBox(0.570 + (pocket - 1) * 0.0421, 0.098, 0.0045, 0.021));
        size_t colored = 0;
        for (size_t y = 0; y < edge.height(); y++){
            for (size_t x = 0; x < edge.width(); x++){
                const uint32_t pixel = edge.pixel(x, y);
                const int r = (pixel >> 16) & 255;
                const int g = (pixel >> 8) & 255;
                const int b = pixel & 255;
                // Red when focused, blue when the item list has focus.
                colored += (r > 160 && r > g + 60 && r > b + 60) ||
                    (b > 160 && b > r + 40 && b > g + 20);
            }
        }
        const double pixels = (double)edge.width() * edge.height();
        if (pixels > 0 && colored / pixels >= 0.18){
            if (selected != 0) return 0;
            selected = pocket;
        }
    }
    return selected;
}

bool bag_list_is_empty(const ImageViewRGB32& frame){
    if (!frame || bag_pocket_position(frame) == 0 || bag_cursor_position(frame) != 0) return false;
    // Empty pockets have no item icons. The central Empty label is outside
    // this column; counting dark pixels in the name area included that label.
    const auto icons = extract_box_reference(frame, ImageFloatBox(0.449, 0.213, 0.032, 0.500));
    const auto stats = image_stats(icons);
    return stats.average.r > 220 && stats.average.g > 220 && stats.average.b > 220 &&
        stats.stddev.sum() < 20;
}


std::array<BagItemRowMatch, 7> match_bag_item_rows(
    const ImageViewRGB32& frame,
    const ImageViewRGB32& reference,
    const BagItemTemplate& item
){
    std::array<BagItemRowMatch, 7> result;
    if (!frame || !reference || item.reference_row < 1 || item.reference_row > 7){
        return result;
    }
    const ImageRGB32 icon = extract_box_reference(reference, bag_list_icon_box(item.reference_row)).copy();
    const ImageRGB32 name = extract_box_reference(reference, bag_list_name_box(item.reference_row)).copy();
    for (size_t row = 1; row <= 7; row++){
        auto& match = result[row - 1];
        match.icon_rmsd = bag_row_patch_score(icon, frame, bag_list_icon_box(row));
        match.name_rmsd = item.check_name
            ? bag_row_patch_score(name, frame, bag_list_name_box(row)) : 0.0;
        match.matched = match.icon_rmsd <= item.max_icon_rmsd && match.name_rmsd <= item.max_name_rmsd;
    }
    return result;
}

BagMovePlan plan_bag_item_move(int cursor_row, int item_row){
    if (cursor_row < 1 || cursor_row > 7 || item_row < 0 || item_row > 7){
        return {};
    }
    if (item_row != 0){
        if (item_row == cursor_row){
            return {BagMoveAction::CONFIRM, 0};
        }
        return {
            item_row > cursor_row ? BagMoveAction::DOWN : BagMoveAction::UP,
            static_cast<size_t>(std::abs(item_row - cursor_row))
        };
    }
    if (cursor_row == 7){
        return {BagMoveAction::REWIND, 1};
    }
    return {BagMoveAction::PAGE_DOWN, cursor_row == 1 ? 13 : static_cast<size_t>(13 - cursor_row)};
}


static int bag_confirm_pocket(VideoStream& stream, ProControllerContext& context){
    int previous = 0;
    size_t matches = 0;
    for (size_t check = 0; check < 10; check++){
        context.wait_for_all_requests();
        const auto frame = stream.video().snapshot();
        const int pocket = frame ? bag_pocket_position(frame) : 0;
        matches = pocket != 0 && pocket == previous ? matches + 1 : pocket != 0 ? 1 : 0;
        previous = pocket;
        if (matches >= 3) return pocket;
        pbf_wait(context, 100ms);
    }
    return 0;
}

// static bool bag_advance_pocket(VideoStream& stream, ProControllerContext& context, int current){
//     const int expected = current % 8 + 1;
//     for (size_t attempt = 0; attempt < 3; attempt++){
//         // Recheck before retrying: a delayed change must not cause a second step.
//         const int observed = bag_confirm_pocket(stream, context);
//         if (observed == expected) return true;
//         if (observed != current){
//             stream.log("[Bag search] Pocket selection uncertain; stopping.", COLOR_ORANGE);
//             return false;
//         }
//         pbf_press_dpad(context, DPAD_RIGHT, 80ms, 500ms);
//         const int result = bag_confirm_pocket(stream, context);
//         stream.log("[Bag search] Pocket change: " + std::to_string(current) + " -> " +
//             std::to_string(result) + ", expected " + std::to_string(expected) + ".");
//         if (result == expected) return true;
//     }
//     stream.log("[Bag search] Could not confirm pocket change after 3 attempts.", COLOR_ORANGE);
//     return false;
// }

BagPocketMovePlan plan_bag_pocket_move(int current, int target){
    if (current < 1 || current > 8 || target < 1 || target > 8) return {};
    const int right = (target - current + 8) % 8;
    const int left = (current - target + 8) % 8;
    if (right == 0) return {BagPocketAction::CONFIRM, 0};
    return right <= left ? BagPocketMovePlan{BagPocketAction::RIGHT, static_cast<size_t>(right)}
        : BagPocketMovePlan{BagPocketAction::LEFT, static_cast<size_t>(left)};
}

static bool bag_navigate_to_pocket(VideoStream& stream, ProControllerContext& context, int target){
    int current = bag_confirm_pocket(stream, context);
    for (size_t attempt = 0; attempt < 3; attempt++){
        const auto plan = plan_bag_pocket_move(current, target);
        if (plan.action == BagPocketAction::CONFIRM) return true;
        if (plan.action == BagPocketAction::UNKNOWN){
            stream.log("[Bag search] Selected pocket not detected.", COLOR_ORANGE);
            return false;
        }
        stream.log("[Bag search] Pocket " + std::to_string(current) + " to " +
            std::to_string(target) + ": " +
            (plan.action == BagPocketAction::RIGHT ? "right " : "left ") +
            std::to_string(plan.presses) + " presses, attempt " + std::to_string(attempt + 1) + "/3.");
        for (size_t press = 0; press < plan.presses; press++){
            pbf_press_dpad(context, plan.action == BagPocketAction::RIGHT ? DPAD_RIGHT : DPAD_LEFT,
                80ms, 150ms);
        }
        pbf_wait(context, 350ms);
        current = bag_confirm_pocket(stream, context);
        stream.log("[Bag search] Final pocket=" + std::to_string(current) +
            ", expected=" + std::to_string(target) + ".");
        if (current == target) return true;
    }
    stream.log("[Bag search] Target pocket not reached after 3 movement batches.", COLOR_ORANGE);
    return false;
}


bool find_bag_item(
    VideoStream& stream,
    ProControllerContext& context,
    const BagItemTemplate& item,
    int target_pocket,
    int num_icons,
    size_t max_scrolls_per_pocket,
    std::chrono::milliseconds search_timeout
){
    if (target_pocket < 1 || target_pocket > 8 || item.reference_row < 1 || item.reference_row > 7){
        stream.log("[Bag search] Invalid template reference row.", COLOR_ORANGE);
        return false;
    }
    auto reference = std::make_shared<const ImageRGB32>(RESOURCE_PATH() + item.reference_path);
//     const ImageRGB32 reference_icon = extract_box_reference(
//         *reference, bag_list_icon_box(item.reference_row)
//     ).copy();
//     const ImageRGB32 reference_name = extract_box_reference(
//         *reference, bag_list_name_box(item.reference_row)
//     ).copy();
    ImageMatchDetector bag(reference, ImageFloatBox(0.055, 0.012, 0.044, 0.045), 35.0, true);
    ImageMatchDetector selected_icon(reference, item.icon_box, item.max_icon_rmsd, true);
    ImageMatchDetector selected_name(reference, item.name_box, item.max_name_rmsd, true);
    const auto deadline = std::chrono::steady_clock::now() + search_timeout;

    open_menu(stream, context, MenuCursorPosition::BAG, num_icons);
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    if (!bag_navigate_to_pocket(stream, context, target_pocket)) return false;
    // Previously searched all eight pockets. Only inspect the requested pocket now.
    // for (size_t pocket = 0; pocket < 8; pocket++){
    {
        const int selected_pocket = bag_confirm_pocket(stream, context);
        if (selected_pocket == 0){
            stream.log("[Bag search] Selected pocket not detected.", COLOR_ORANGE);
            return false;
        }
        size_t commands = 0;
        bool empty = false;
        auto move = [&](DpadPosition direction, size_t count, bool check_rows = false){
            for (size_t press = 0; press < count; press++){
                if (commands >= max_scrolls_per_pocket ||
                    std::chrono::steady_clock::now() >= deadline){
                    stream.log("[Bag search] Limit reached; search incomplete.", COLOR_ORANGE);
                    return false;
                }
                commands++;
                pbf_press_dpad(context, direction, 80ms, 150ms);
                if (check_rows){
                    context.wait_for_all_requests();
                    const VideoSnapshot page_frame = stream.video().snapshot();
                    if (!page_frame || !bag.detect(page_frame)){
                        return false;
                    }
                    const auto matches = match_bag_item_rows(page_frame, *reference, item);
                    for (const auto& match : matches){
                        if (match.matched){
                            // Stop paging; the main loop verifies and selects the row.
                            return true;
                        }
                    }
                }
            }
            pbf_wait(context, 250ms);
            context.wait_for_all_requests();
            return true;
        };
        for (size_t pass = 0; pass < 3 && !empty; pass++){
            size_t matching_frames = 0;
            size_t missing_cursor = 0;
            size_t failed_confirmation = 0;
            size_t unchanged_pages = 0;
            ImageRGB32 previous_page;
            stream.log(
                "[Bag search] Looking for " + item.label + ": pocket " +
                std::to_string(selected_pocket) + ", pass " + std::to_string(pass + 1) + "/3."
            );
            while (std::chrono::steady_clock::now() < deadline){
                const VideoSnapshot frame = stream.video().snapshot();
                if (!frame || !bag.detect(frame)){
                    stream.log("[Bag search] Bag screen not detected.", COLOR_ORANGE);
                    return false;
                }
                const int cursor = bag_cursor_position(frame);
                int target = 0;
                double best = 1000.0;
                bool ambiguous = false;
                // The seven list ROIs move neither with the cursor nor with
                // scrolling. Each contains an icon patch and a name patch.
                // Previous per-row loop retained for reference.
                // for (size_t row = 1; row <= 7; row++){
                // const double icon = bag_row_patch_score(reference_icon, frame, bag_list_icon_box(row));
                // const double name = item.check_name
                // ? bag_row_patch_score(reference_name, frame, bag_list_name_box(row)) : 0.0;
                // if (icon <= item.max_icon_rmsd && name <= item.max_name_rmsd){
                // if (target != 0){
                // ambiguous = true;
                // }
                // if (icon + name < best){
                // target = static_cast<int>(row);
                // best = icon + name;
                // }
                // }
                // }
                const auto matches = match_bag_item_rows(frame, *reference, item);
                for (size_t row = 1; row <= 7; row++){
                    const auto& match = matches[row - 1];
                    if (match.matched){
                        if (target != 0){
                            ambiguous = true;
                        }
                        if (match.icon_rmsd + match.name_rmsd < best){
                            target = static_cast<int>(row);
                            best = match.icon_rmsd + match.name_rmsd;
                        }
                    }
                }
                stream.log(
                    "[Bag search] " + item.label + (target == 0 ? " is not detected" : " is detected") +
                    " on screen; item row=" + std::to_string(target) +
                    ", cursor row=" + std::to_string(cursor) + "."
                );
                if (ambiguous){
                    stream.log("[Bag search] Multiple rows match; selection is uncertain.", COLOR_ORANGE);
                    return false;
                }
                if (cursor == 0){
                    if (bag_list_is_empty(frame)){
                        empty = true;
                        break;
                    }
                    if (++missing_cursor >= 3){
                        stream.log("[Bag search] Cursor location uncertain.", COLOR_ORANGE);
                        return false;
                    }
                    pbf_wait(context, 200ms);
                    context.wait_for_all_requests();
                    continue;
                }
                missing_cursor = 0;
                if (target != 0){
                    previous_page = ImageRGB32();
                    unchanged_pages = 0;
                    if (target == cursor && selected_icon.detect(frame) &&
                        (!item.check_name || selected_name.detect(frame))){
                        if (++matching_frames >= 3){
                            stream.log("[Bag search] Item and cursor confirmed in row " +
                                std::to_string(cursor) + ".", COLOR_GREEN);
                            return true;
                        }
                        pbf_wait(context, 150ms);
                        context.wait_for_all_requests();
                        continue;
                    }
                    matching_frames = 0;
                    if (target == cursor){
                        if (++failed_confirmation < 3){
                            pbf_wait(context, 150ms);
                            context.wait_for_all_requests();
                            continue;
                        }
                        stream.log("[Bag search] Selected-item confirmation failed.", COLOR_ORANGE);
                        return false;
                    }
                    failed_confirmation = 0;
                    const size_t count = plan_bag_item_move(cursor, target).presses;
                    stream.log("[Bag search] Moving " + std::string(target > cursor ? "down " : "up ") +
                        std::to_string(count) + " time(s), then checking again.");
                    if (!move(target > cursor ? DPAD_DOWN : DPAD_UP, count)){
                        return false;
                    }
                    continue;
                }
                matching_frames = 0;
                const ImageRGB32 page = extract_box_reference(
                    frame, ImageFloatBox(0.449, 0.212, 0.378, 0.500)
                ).scale_to(378, 500);
                if (previous_page && ImageMatch::pixel_RMSD(previous_page, page) <= 3.0){
                    unchanged_pages++;
                }else{
                    unchanged_pages = 0;
                }
                previous_page = page.copy();
                // Row 7 signals the bottom of a long list. Repeated pages
                // also cover shorter lists whose final row is less than 7.
                if (cursor == 7 || unchanged_pages >= 2){
                    stream.log("[Bag search] List end; returning to row 1.");
                    while (true){
                        const VideoSnapshot rewind_frame = stream.video().snapshot();
                        if (!rewind_frame || !bag.detect(rewind_frame)){
                            return false;
                        }
                        const int rewind_cursor = bag_cursor_position(rewind_frame);
                        if (rewind_cursor == 1){
                            break;
                        }
                        if (rewind_cursor == 0 || !move(DPAD_UP, 1)){
                            return false;
                        }
                    }
                    break;
                }
                const size_t count = plan_bag_item_move(cursor, 0).presses;
                stream.log("[Bag search] Advancing " + std::to_string(count) + " presses to another page.");
                if (!move(DPAD_DOWN, count, true)){
                    return false;
                }
            }
            if (std::chrono::steady_clock::now() >= deadline){
                stream.log("[Bag search] Timeout; search incomplete.", COLOR_ORANGE);
                return false;
            }
        }
//         if (pocket < 7){
//             // pbf_press_dpad(context, DPAD_RIGHT, 80ms, 500ms);
//             // context.wait_for_all_requests();
//             if (!bag_advance_pocket(stream, context, selected_pocket)) return false;
//         }

    }
    stream.log("[Bag search] Item not found after three passes per nonempty pocket.", COLOR_ORANGE);
    return false;
}

bool use_escape_rope(
    VideoStream& stream,
    ProControllerContext& context,
    int num_icons
){
    const BagItemTemplate rope{
        // "PokemonBDSP/AutoStory/Bag_EscapeRope.png", "escape-rope"
        "PokemonBDSP/Autostory/Bag_EscapeRope.png", "escape-rope"
    };
    if (!find_bag_item(stream, context, rope, 5, num_icons)){
        return false;
    }

    pbf_press_button(context, BUTTON_A, 80ms, 350ms);
    context.wait_for_all_requests();
    SelectionArrowFinder use_menu(
        stream.overlay(), ImageFloatBox(0.660, 0.100, 0.310, 0.430), COLOR_GREEN
    );
    if (wait_until(stream, context, 3s, {{use_menu}}) != 0){
        stream.log("[Escape Rope] Item action menu not detected.", COLOR_ORANGE);
        return false;
    }

    // Watch while selecting Use so the transition cannot happen before
    // the watcher starts. Extra A presses handle a confirmation prompt.
    BlackScreenWatcher transition;
    // Escape Rope fades white in the supplied clips; support black transitions too.
    WhiteScreenWatcher white_transition;
    const int result = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& ctx){
            for (size_t press = 0; press < 10; press++){
                pbf_press_button(ctx, BUTTON_A, 80ms, 500ms);
            }
            ctx.wait_for_all_requests();
            pbf_wait(ctx, 10s);
            ctx.wait_for_all_requests();
        },
        {transition, white_transition}
    );
    if (result < 0){
        stream.log("[Escape Rope] No exit transition detected.", COLOR_ORANGE);
        return false;
    }
    BlackScreenWatcher finished(
        COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 100, 10,
        BlackScreenWatcher::FinderType::GONE, 500ms
    );
    WhiteScreenWatcher white_finished(
        COLOR_RED, {0.1, 0.1, 0.8, 0.8}, 500, 10,
        WhiteScreenWatcher::FinderType::GONE, 500ms
    );
    stream.log(std::string("[Escape Rope] ") +
        (result == 1 ? "White" : "Black") + " exit transition detected.");
    // if (wait_until(stream, context, 15s, {{finished}}) != 0){ return false; }
    const int finish_result = result == 1
        ? wait_until(stream, context, 15s, {{white_finished}})
        : wait_until(stream, context, 15s, {{finished}});
    if (finish_result != 0){
        stream.log("[Escape Rope] Exit transition did not finish.", COLOR_ORANGE);
        return false;
    }
    OverworldWatcher overworld;
    if (wait_until(stream, context, 10s, {{overworld}}) != 0){
        stream.log("[Escape Rope] Overworld not detected after transition.", COLOR_ORANGE);
        return false;
    }
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();
    stream.log("[Escape Rope] Exit transition and overworld confirmed.", COLOR_GREEN);
    return true;
}


bool activate_repel(
    VideoStream& stream,
    ProControllerContext& context,
    int num_icons
){
    stream.log("Activating repel");
    open_menu(stream, context, MenuCursorPosition::BAG, num_icons);
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    auto repel_tab_ref = std::make_shared<const ImageRGB32>(
        RESOURCE_PATH() + "PokemonBDSP/AutoStory/Bag_RepelTab.png"
    );
    const ImageFloatBox repel_tab_box{0.735000, 0.055000, 0.045000, 0.080000};
    const double repel_tab_rmsd = 50.0;

    // BDSP bag has 8 compartment tabs; check before each press so zero-press
    // detection works when the bag already starts on the repel tab.
    bool found = false;
    for (int press = 0; press <= 8; press++){
        context.wait_for_all_requests();

        ImageMatchWatcher watcher(repel_tab_ref, repel_tab_box, repel_tab_rmsd);
        int ret = wait_until(stream, context, 400ms, {{watcher}});
        if (ret == 0){
            stream.log("[AutoStory] activate_repel: repel tab found after "
                       + std::to_string(press) + " press(es).", COLOR_GREEN);
            found = true;
            break;
        }

        if (press < 8){
            stream.log("[AutoStory] activate_repel: tab not found, pressing right ("
                       + std::to_string(press + 1) + "/8).");
            pbf_press_dpad(context, DPAD_RIGHT, 80ms, 300ms);
        }
    }

    if (!found){
        stream.log("[AutoStory] activate_repel: failed to find repel compartment tab.", COLOR_RED);
        return false;
    }

    context.wait_for_all_requests();
    pbf_wait(context, 400ms);
    pbf_press_button(context, BUTTON_A, 80ms, 300ms);
    context.wait_for_all_requests();

    const ImageFloatBox repel_item_box{0.720000, 0.100000, 0.070000, 0.150000};
    SelectionArrowFinder arrow_finder(stream.overlay(), repel_item_box, COLOR_GREEN);
    int arrow_ret = wait_until(stream, context, 2000ms, {{arrow_finder}});
    if (arrow_ret != 0){
        stream.log("[AutoStory] activate_repel: selection arrow not found after pressing A.", COLOR_RED);
        return false;
    }
    stream.log("[AutoStory] activate_repel: selection arrow confirmed.", COLOR_GREEN);
    pbf_press_button(context, BUTTON_A, 80ms, 105ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    return true;
}


void use_HM(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& label

){
    stream.log("Using " + label);
    const ImageFloatBox box{0.670000, 0.600000, 0.100000, 0.150000};
    SelectionArrowFinder arrow(stream.overlay(), box, COLOR_GREEN);

    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 30000ms);
        },
        {{arrow}}
    );
    if (ret != 0){
        stream.log("Using " + label + ": selection arrow not found.", COLOR_RED);
        return;
    }

    stream.log("Using " + label + ": selection arrow found, confirming.", COLOR_GREEN);
    pbf_mash_button(context, BUTTON_A, 1000ms);
    pbf_mash_button(context, BUTTON_B, 6000ms);
}


bool get_on_bicycle(VideoStream& stream, ProControllerContext& context, GearLevel /*gear*/){
    static const ImageFloatBox FRONT_BOX{0.440000, 0.470000, 0.035000, 0.054000};
    static const ImageFloatBox BACK_BOX {0.520000, 0.470000, 0.022000, 0.054000};
    // Spark color range: white (R=232,G=255,B=255) down to light green (R=163,G=217,B=181)
    static const uint32_t SPARK_MIN = 0xFF000000u | (163u << 16) | (217u << 8) | 181u;
    static const uint32_t SPARK_MAX = 0xFFFFFFFFu;
    static const size_t SPARK_THRESHOLD = 13;

    stream.log("[AutoStory] get_on_bicycle: mounting bicycle.", COLOR_WHITE);
    pbf_press_button(context, BUTTON_PLUS, 80ms, 800ms);

    // Press B, then poll up to 1 s for Fast-gear sparks.
    // Logs the peak pixel counts in both boxes for threshold tuning.
    // Returns true if sparks are detected (switched TO Fast gear).
    auto toggle_and_check = [&]() -> bool {
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_B, 80ms, 800ms);

        size_t max_front = 0, max_back = 0;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
        while (std::chrono::steady_clock::now() < deadline){
            context.wait_for(50ms);
            VideoSnapshot screen = stream.video().snapshot();
            if (!screen) continue;

            size_t front_pixels = 0, back_pixels = 0;
            filter_rgb32_range(front_pixels, extract_box_reference(screen, FRONT_BOX), SPARK_MIN, SPARK_MAX, Color(0), false);
            filter_rgb32_range(back_pixels,  extract_box_reference(screen, BACK_BOX),  SPARK_MIN, SPARK_MAX, Color(0), false);

            if (front_pixels > max_front) max_front = front_pixels;
            if (back_pixels  > max_back)  max_back  = back_pixels;

            if (front_pixels >= SPARK_THRESHOLD || back_pixels >= SPARK_THRESHOLD){
                stream.log(
                    "[AutoStory] get_on_bicycle: sparks detected "
                    "(front=" + std::to_string(front_pixels) +
                    " back="  + std::to_string(back_pixels)  +
                    " threshold=" + std::to_string(SPARK_THRESHOLD) + ").",
                    COLOR_GREEN
                );
                return true;
            }
        }
        stream.log(
            "[AutoStory] get_on_bicycle: no sparks detected "
            "(peak front=" + std::to_string(max_front) +
            " back="       + std::to_string(max_back)  +
            " threshold="  + std::to_string(SPARK_THRESHOLD) + ").",
            COLOR_ORANGE
        );
        return false;
    };

    if (toggle_and_check()){
        stream.log("[AutoStory] get_on_bicycle: Fast gear confirmed.", COLOR_GREEN);
        return true;
    }

    // First toggle produced no sparks -> we switched to Slow gear; toggle back to Fast.
    stream.log("[AutoStory] get_on_bicycle: Slow gear detected, toggling back to Fast.", COLOR_ORANGE);
    if (toggle_and_check()){
        stream.log("[AutoStory] get_on_bicycle: Fast gear confirmed after correction.", COLOR_GREEN);
        return true;
    }

    stream.log("[AutoStory] get_on_bicycle: Failed to confirm Fast gear after two toggles.", COLOR_RED);
    return false;
}


}
}
}