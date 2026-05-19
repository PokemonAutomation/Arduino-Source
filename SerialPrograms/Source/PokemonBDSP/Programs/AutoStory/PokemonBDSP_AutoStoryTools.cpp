/*  BDSP AutoStory Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include <cmath>
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
#include "CommonTools/Images/SolidColorTest.h"
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

void fake_save_game(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_R, 80ms, 1000ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}


void checkpoint_save(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats
){
    static constexpr size_t MAX_RETRIES = 10;

    bool success = false;

    for (size_t attempt = 0; attempt < MAX_RETRIES; attempt++){
        try{
            env.console.log(
                "Checkpoint save attempt: " + std::to_string(attempt + 1)
            );

            // ---------------------------
            // Force-close anything weird first.
            // ---------------------------
            pbf_mash_button(context, BUTTON_B, 1000ms);

            // ---------------------------
            // Aggressively attempt to open menu.
            // No dependence on MenuWatcher.
            // ---------------------------
            pbf_press_button(context, BUTTON_X, 160ms, 1500ms);

            // ---------------------------
            // Trigger save
            // ---------------------------
            pbf_press_button(context, BUTTON_R, 160ms, 2500ms);

            // ---------------------------
            // Confirm save
            // ---------------------------
            pbf_press_button(context, BUTTON_ZL, 160ms, 5000ms);

            // ---------------------------
            // Return to overworld cleanly
            // ---------------------------
            pbf_mash_button(context, BUTTON_B, 1500ms);

            // ---------------------------
            // Verify overworld
            // ---------------------------
            OverworldWatcher overworld;

            int ret = wait_until(
                env.console,
                context,
                std::chrono::seconds(5),
                {overworld}
            );

            if (ret < 0){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to detect overworld after save.",
                    env.console
                );
            }

            env.console.log("Checkpoint save verified.");

            success = true;
            break;

        }catch (OperationFailedException& e){
            env.console.log(
                "Checkpoint save failed: " + e.message(),
                COLOR_RED
            );

            context.wait_for(1000ms);
        }
    }

    // ---------------------------
    // Blind fallback
    // ---------------------------
    if (!success){
        env.console.log(
            "All save retries failed. Using blind fallback.",
            COLOR_ORANGE
        );

        pbf_mash_button(context, BUTTON_B, 1000ms);
        pbf_press_button(context, BUTTON_X, 160ms, 1500ms);
        pbf_press_button(context, BUTTON_R, 160ms, 2500ms);
        pbf_press_button(context, BUTTON_ZL, 160ms, 5000ms);
        pbf_mash_button(context, BUTTON_B, 2000ms);

        context.wait_for_all_requests();
    }

    stats.m_checkpoint++;
    env.update_stats();

    send_program_status_notification(
        env,
        notif_status_update,
        success
            ? "Saved at checkpoint (verified)."
            : "Saved at checkpoint (blind fallback)."
    );
}


void checkpoint_reattempt_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EventNotificationOption& notif_status_update,
    AutoStoryStats& stats,
    std::function<void(size_t attempt_number)>&& action
){
    static constexpr size_t MAX_ATTEMPTS = 20;

    for (size_t i = 0;; i++){
        try{
            if (i == 0){
                checkpoint_save(env, context, notif_status_update, stats);
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

            env.console.log("Resetting game from HOME before retry...", COLOR_ORANGE);
            bool ok = reset_game_from_home(env, env.console, context, true, 1000ms);
            if (!ok){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to reset game from HOME during checkpoint retry.",
                    env.console
                );
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

bool fly_to(
    VideoStream& stream,
    ProControllerContext& context,
    const std::string& place
){
    static const std::unordered_set<std::string> EAST_CITIES = {
        "celestic_town",
        "hearthome_city",
        "pastoria_city",
        "pokemon_league_lower",
        "pokemon_league_upper",
        "solaceon_town",
        "sunyshore_city",
        "veilstone_city"
    };

    static const std::unordered_set<std::string> NORTH_CITY = {
        "snowpoint_city",
    };

    // Each city branch below should update this to the city icon's position on the fly map.
    // Detection: when the cursor brackets overlap the icon, its normally uniform
    // color becomes non-uniform (stddev rises). Box stays as a rough fallback
    // until each city is calibrated.
    ImageFloatBox city_icon_box{0.700000, 0.550000, 0.100000, 0.120000};

    DpadState dpad;
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
    pbf_press_button(context, BUTTON_PLUS, 80ms, 1000ms);
    if (EAST_CITIES.count(place)){
        pbf_move_left_joystick(context, {+1, -1}, 4000ms, 500ms);
    } else if (NORTH_CITY.count(place)) {
        pbf_move_left_joystick(context, {-1, +1}, 4000ms, 500ms);
    } else {
        pbf_move_left_joystick(context, {-1, -1}, 4000ms, 500ms);
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

    if (place == "canalave_city") {

    } else if (place == "celestic_town") {

    } else if (place == "eterna_city") {

    } else if (place == "floaroma_town") {

    } else if (place == "hearthome_city") {

        city_icon_box = {0.660000, 0.637000, 0.029000, 0.052000};
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 16, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 7, false);

    } else if (place == "jubilife_city") {

    } else if (place == "oreburgh_city") {

    } else if (place == "pastoria_city") {
        
    } else if (place == "pokemon_league_lower") {
        
    } else if (place == "pokemon_league_upper") {
        
    } else if (place == "route_221") {
        
    } else if (place == "sandgem_town") {
        
    } else if (place == "solaceon_town") {

        city_icon_box = {0.735000, 0.603000, 0.030000, 0.022000};
        repeat_dpad(context, dpad, DPAD_LEFT, 80ms, 300ms, 12, false);
        repeat_dpad(context, dpad, DPAD_UP, 80ms, 300ms, 9, false);

    } else if (place == "sunyshore_city") {
        
    } else if (place == "twinleaf_town") {
        
    } else if (place == "veilstone_city") {
        
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

    int ret_1 = wait_until(stream, context, 3000ms, {{watcher_1}});
    int ret_2 = wait_until(stream, context, 4000ms, {{watcher_2}});
    if (ret_1 < 0) {
        stream.log("A button not found, retrying...", COLOR_RED);
    };
    if (ret_2 < 0) {
        stream.log(place + " not found, retrying...", COLOR_RED);
    };
    if (ret_1 == 0 and ret_2 == 0){
        stream.log("Found " + place + ", flying towards it.", COLOR_GREEN);
        BlackScreenOverWatcher flying_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{flying_black_screen}}
        );
        if (ret < 0){
            stream.log("Flying to " + place + ": black screen not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("Flying to " + place + ": black screen detected!", COLOR_GREEN);
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

void use_potion_first_pokemon(
    VideoStream & stream,
    ProControllerContext & context
) {
    stream.log(
        "[AutoStory] Starting arrow-based potion helper...",
        COLOR_BLUE
    );
    pbf_press_button(
        context,
        BUTTON_X,
        160ms,
        1500ms
    );
    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);
    // ------------------------------------------------------------
    // Move from item pane to pokemon pane.
    // ------------------------------------------------------------

    pbf_press_dpad(
        context,
        DPAD_LEFT,
        100ms,
        500ms
    );

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    // ------------------------------------------------------------
    // Select pokemon menu pokemon.
    // ------------------------------------------------------------

    pbf_press_button(
        context,
        BUTTON_A,
        160ms,
        1500ms
    );

    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);

    // ------------------------------------------------------------
    // Select first pokemon.
    // ------------------------------------------------------------

    pbf_press_button(
        context,
        BUTTON_A,
        160ms,
        1500ms
    );

    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);


    // ------------------------------------------------------------
    // Move from "Check summary" to "Restore".
    // ------------------------------------------------------------

    pbf_press_dpad(
        context,
        DPAD_DOWN,
        80ms,
        250ms
    );

    pbf_press_dpad(
        context,
        DPAD_DOWN,
        80ms,
        500ms
    );

    // ------------------------------------------------------------
    // Mash A.
    // ------------------------------------------------------------

    stream.log(
        "[AutoStory] Mashing A...",
        COLOR_BLUE
    );

    pbf_mash_button(
        context,
        BUTTON_A,
        3000ms
    );

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    // ------------------------------------------------------------
    // Mash B to fully exit.
    // ------------------------------------------------------------

    stream.log(
        "[AutoStory] Exiting menus...",
        COLOR_BLUE
    );

    pbf_mash_button(
        context,
        BUTTON_B,
        3000ms
    );

    context.wait_for_all_requests();
    pbf_wait(context, 3000ms);
    // ------------------------------------------------------------
    // open menu and reset cursor to bag.
    // ------------------------------------------------------------
    stream.log(
        "[AutoStory] Exiting menus...",
        COLOR_BLUE
    );

    pbf_press_button(
        context,
        BUTTON_X,
        160ms,
        1500ms
    );

    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    pbf_press_dpad(
        context,
        DPAD_RIGHT,
        100ms,
        500ms
    );
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);
    pbf_press_button(
        context,
        BUTTON_X,
        160ms,
        1500ms
    );
    context.wait_for_all_requests();
    pbf_wait(context, 1000ms);

    stream.log(
        "[AutoStory] Potion helper complete.",
        COLOR_GREEN
    );
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

    overworld_to_menu(stream, context);
    context.wait_for(1000ms);

    MenuCursorDetector detector;
    {
        static const char* ICON_NAMES[MENU_CURSOR_NUM_ICONS] = {
            "POKEDEX", "POKEMON", "BAG", "CARD",
            "OPTIONS", "MAP", "CAPSULES", "MYSTERYGIFT",
        };
        std::string ref_dir = RESOURCE_PATH() + "PokemonBDSP/AutoStory";
        for (int i = 0; i < MENU_CURSOR_NUM_ICONS; i++){
            std::string path = ref_dir + "/" + ICON_NAMES[i] + ".png";
            ImageRGB32 img(path);
            if (img){
                detector.set_icon_reference(
                    static_cast<MenuCursorPosition>(i),
                    std::move(img)
                );
            }
        }
    }
    MenuCursorPosition cur = MenuCursorPosition::UNKNOWN;
    for (int i = 0; i < 10 && cur == MenuCursorPosition::UNKNOWN; i++){
        context.wait_for(50ms);
        VideoSnapshot snap = stream.video().snapshot();
        if (snap) cur = detector.detect(snap);
    }
    if (cur == MenuCursorPosition::UNKNOWN){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Menu cursor not detected after opening menu.",
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


}
}
}