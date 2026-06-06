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
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "CommonTools/Images/SolidColorTest.h"
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
    AutoStoryStats& stats
){
    bool success = fake_save_game(env, context);

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
    bool save
){
    static constexpr size_t MAX_ATTEMPTS = 20;

    for (size_t i = 0;; i++){
        try{
            if (i == 0){
                if (save || stats.m_first_run){
                    checkpoint_save(env, context, notif_status_update, stats);
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
        BlackScreenOverWatcher flying_black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.8});
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 200000ms);
            },
            {{flying_black_screen}}
        );
        if (ret < 0){
            stream.log("Flying to " + name + ": black screen not detected!", COLOR_RED);
            return false;
        } else if (ret == 0) {
            stream.log("Flying to " + name + ": black screen detected!", COLOR_GREEN);
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

    context.wait_for_all_requests();
    overworld_to_menu(stream, context);
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

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


void use_strength(
    VideoStream& stream,
    ProControllerContext& context
){
    stream.log("[AutoStory] use_strength: pressing A until selection arrow appears.");
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
        stream.log("[AutoStory] use_strength: selection arrow not found.", COLOR_RED);
        return;
    }

    stream.log("[AutoStory] use_strength: selection arrow found, confirming.", COLOR_GREEN);
    pbf_press_button(context, BUTTON_A, 80ms, 200ms);
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