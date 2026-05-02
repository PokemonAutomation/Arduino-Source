/*  Wild RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <algorithm>
#include <sstream>
#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_PartyDialogs.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_BagDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_DexRegistrationDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_PartyLevelUpReader.h"
#include "PokemonFRLG/Inference/PokemonFRLG_StatsReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_RngStatsDatabase.h"
#include "PokemonFRLG_EncountersDatabase.h"
#include "PokemonFRLG_WildRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


WildRng_Descriptor::WildRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:WildRng",
        Pokemon::STRING_POKEMON + " FRLG", "Wild RNG",
        "Programs/PokemonFRLG/WildRng.html",
        "Automatically calibrate timings to hit a specific RNG target for FRLG random wild encounters.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct WildRng_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , nonshiny(m_stats["Non-Shiny Hits"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Non-Shiny Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& nonshiny;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> WildRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

WildRng::WildRng()
    : LANGUAGE(
        "<b>Game Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , GAME_VERSION(
        "<b>Game Version:</b>",
        {
            {GameVersion::firered, "firered", "FireRed"},
            {GameVersion::leafgreen, "leafgreen", "LeafGreen"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        GameVersion::firered
    )
    , ENCOUNTER_TYPE(
        "<b>Encounter Type:</b>",
        {
            {EncounterType::grass, "grass", "Grass"},
            {EncounterType::rocksmash, "rocksmash", "Rock Smash"},
            {EncounterType::surfing, "surfing", "Surfing"},
            {EncounterType::oldrod, "oldrod", "Old Rod"},
            {EncounterType::goodrod, "goodrod", "Good Rod"},
            {EncounterType::superrod, "superrod", "Super Rod"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        EncounterType::grass
    )
    , GAME_LOCATION(
        "<b>Location:</b>",
        {
            {GameLocation::altering_cave, "altering_cave", "Altering Cave"},
            {GameLocation::berry_forest, "berry_forest", "Berry Forest"},
            {GameLocation::bond_bridge, "bond_bridge", "Bond Bridge"},
            {GameLocation::cape_brink, "cape_brink", "Cape Brink"},
            {GameLocation::celadon_city, "celadon_city", "Celadon City"},
            {GameLocation::cerulean_cave_1f, "cerulean_cave_1f", "Cerulean Cave 1F"},
            {GameLocation::cerulean_cave_2f, "cerulean_cave_2f", "Cerulean Cave 2F"},
            {GameLocation::cerulean_cave_b1f, "cerulean_cave_b1f", "Cerulean Cave B1F"},
            {GameLocation::cerulean_city, "cerulean_city", "Cerulean City"},
            {GameLocation::cinnabar_island, "cinnabar_island", "Cinnabar Island"},
            {GameLocation::digletts_cave, "digletts_cave", "Digletts Cave"},
            {GameLocation::five_island, "five_island", "Five Island"},
            {GameLocation::five_isle_meadow, "five_isle_meadow", "Five Isle Meadow"},
            {GameLocation::four_island, "four_island", "Four Island"},
            {GameLocation::fuchsia_city, "fuchsia_city", "Fuchsia City"},
            {GameLocation::green_path, "green_path", "Green Path"},
            {GameLocation::icefall_cave_1f, "icefall_cave_1f", "Icefall Cave 1F"},
            {GameLocation::icefall_cave_back_cavern, "icefall_cave_back_cavern", "Icefall Cave Back Cavern"},
            {GameLocation::icefall_cave_b1f, "icefall_cave_b1f", "Icefall Cave B1F"},
            {GameLocation::icefall_cave_entrance, "icefall_cave_entrance", "Icefall Cave Entrance"},
            {GameLocation::kindle_road, "kindle_road", "Kindle Road"},
            {GameLocation::lost_cave, "lost_cave", "Lost Cave"},
            {GameLocation::memorial_pillar, "memorial_pillar", "Memorial Pillar"},
            {GameLocation::mt_ember_exterior, "mt_ember_exterior", "Mt. Ember Exterior"},
            {GameLocation::mt_ember_ruby_path_1f, "mt_ember_ruby_path_1f", "Mt. Ember Ruby Path 1F"},
            {GameLocation::mt_ember_ruby_path_b1f, "mt_ember_ruby_path_b1f", "Mt. Ember Ruby Path B1F"},
            {GameLocation::mt_ember_ruby_path_b2f, "mt_ember_ruby_path_b2f", "Mt. Ember Ruby Path B2F"},
            {GameLocation::mt_ember_ruby_path_b3f, "mt_ember_ruby_path_b3f", "Mt. Ember Ruby Path B3F"},
            {GameLocation::mt_ember_summit_path_1f, "mt_ember_summit_path_1f", "Mt. Ember Summit Path 1F"},
            {GameLocation::mt_ember_summit_path_2f, "mt_ember_summit_path_2f", "Mt. Ember Summit Path 2F"},
            {GameLocation::mt_ember_summit_path_3f, "mt_ember_summit_path_3f", "Mt. Ember Summit Path 3F"},
            {GameLocation::mt_moon_1f, "mt_moon_1f", "Mt. Moon 1F"},
            {GameLocation::mt_moon_b1f, "mt_moon_b1f", "Mt. Moon B1F"},
            {GameLocation::mt_moon_b2f, "mt_moon_b2f", "Mt. Moon B2F"},
            {GameLocation::one_island, "one_island", "One Island"},
            {GameLocation::outcast_island, "outcast_island", "Outcast Island"},
            {GameLocation::pallet_town, "pallet_town", "Pallet Town"},
            {GameLocation::pattern_bush, "pattern_bush", "Pattern Bush"},
            {GameLocation::pokemon_mansion_basement, "pokemon_mansion_basement", STRING_POKEMON +  " Mansion Basement"},
            {GameLocation::pokemon_mansion_upper_floors, "pokemon_mansion_upper_floors", STRING_POKEMON + " Mansion Upper Floors"},
            {GameLocation::pokemon_tower_3f, "pokemon_tower_3f", STRING_POKEMON + " Tower 3F"},
            {GameLocation::pokemon_tower_4f, "pokemon_tower_4f", STRING_POKEMON + " Tower 4F"},
            {GameLocation::pokemon_tower_5f, "pokemon_tower_5f", STRING_POKEMON + " Tower 5F"},
            {GameLocation::pokemon_tower_6f, "pokemon_tower_6f", STRING_POKEMON + " Tower 6F"},
            {GameLocation::pokemon_tower_7f, "pokemon_tower_7f", STRING_POKEMON + " Tower 7F"},
            {GameLocation::power_plant, "power_plant", "Power Plant"},
            {GameLocation::resort_gorgeous, "resort_gorgeous", "Resort Gorgeous"},
            {GameLocation::rock_tunnel_1f, "rock_tunnel_1f", "Rock Tunnel 1F"},
            {GameLocation::rock_tunnel_b1f, "rock_tunnel_b1f", "Rock Tunnel B1F"},
            {GameLocation::route_1, "route_1", "Route 1"},
            {GameLocation::route_10, "route_10", "Route 10"},
            {GameLocation::route_11, "route_11", "Route 11"},
            {GameLocation::route_12, "route_12", "Route 12"},
            {GameLocation::route_13, "route_13", "Route 13"},
            {GameLocation::route_14, "route_14", "Route 14"},
            {GameLocation::route_15, "route_15", "Route 15"},
            {GameLocation::route_16, "route_16", "Route 16"},
            {GameLocation::route_17, "route_17", "Route 17"},
            {GameLocation::route_18, "route_18", "Route 18"},
            {GameLocation::route_19, "route_19", "Route 19"},
            {GameLocation::route_2, "route_2", "Route 2"},
            {GameLocation::route_20, "route_20", "Route 20"},
            {GameLocation::route_21, "route_21", "Route 21"},
            {GameLocation::route_22, "route_22", "Route 22"},
            {GameLocation::route_23, "route_23", "Route 23"},
            {GameLocation::route_24, "route_24", "Route 24"},
            {GameLocation::route_25, "route_25", "Route 25"},
            {GameLocation::route_3, "route_3", "Route 3"},
            {GameLocation::route_4, "route_4", "Route 4"},
            {GameLocation::route_5, "route_5", "Route 5"},
            {GameLocation::route_6, "route_6", "Route 6"},
            {GameLocation::route_7, "route_7", "Route 7"},
            {GameLocation::route_8, "route_8", "Route 8"},
            {GameLocation::route_9, "route_9", "Route 9"},
            {GameLocation::ruin_valley, "ruin_valley", "Ruin Valley"},
            {GameLocation::safari_zone_area_1_east, "safari_zone_area_1_east", "Safari Zone Area 1 - East"},
            {GameLocation::safari_zone_area_2_north, "safari_zone_area_2_north", "Safari Zone Area 2 - North"},
            {GameLocation::safari_zone_area_3_west, "safari_zone_area_3_west", "Safari Zone Area 3 - West"},
            {GameLocation::safari_zone_entrance, "safari_zone_entrance", "Safari Zone Entrance"},
            {GameLocation::seafoam_islands_1f, "seafoam_islands_1f", "Seafoam Islands 1F"},
            {GameLocation::seafoam_islands_b1f, "seafoam_islands_b1f", "Seafoam Islands B1F"},
            {GameLocation::seafoam_islands_b2f, "seafoam_islands_b2f", "Seafoam Islands B2F"},
            {GameLocation::seafoam_islands_b3f, "seafoam_islands_b3f", "Seafoam Islands B3F"},
            {GameLocation::seafoam_islands_b4f, "seafoam_islands_b4f", "Seafoam Islands B4F"},
            {GameLocation::sevault_canyon, "sevault_canyon", "Sevault Canyon"},
            {GameLocation::sevault_canyon_entrance, "sevault_canyon_entrance", "Sevault Canyon Entrance"},
            {GameLocation::ss_anne, "ss_anne", "S.S. Anne"},
            {GameLocation::tanoby_ruins, "tanoby_ruins", "Tanoby Ruins"},
            {GameLocation::tanoby_ruins_chambers, "tanoby_ruins_chambers", "Tanoby Ruins Chambers"},
            {GameLocation::three_isle_port, "three_isle_port", "Three Isle Port"},
            {GameLocation::trainer_tower, "trainer_tower", "Trainer Tower"},
            {GameLocation::treasure_beach, "treasure_beach", "Treasure Beach"},
            {GameLocation::vermilion_city, "vermilion_city", "Vermilion City"},
            {GameLocation::victory_road_1f, "victory_road_1f", "Victory Road 1F"},
            {GameLocation::victory_road_2f, "victory_road_2f", "Victory Road 2F"},
            {GameLocation::victory_road_3f, "victory_road_3f", "Victory Road 3F"},
            {GameLocation::viridian_city, "viridian_city", "Viridian City"},
            {GameLocation::viridian_forest, "viridian_forest", "Viridian Forest"},
            {GameLocation::water_labyrinth, "water_labyrinth", "Water Labyrinth"},
            {GameLocation::water_path, "water_path", "Water Path"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        GameLocation::route_1
    )    
    , MAX_RESETS(
        "<b>Max Resets:</b><br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        50, 0 // default, min
    )
    , MAX_RARE_CANDIES(
        "<b>Max Rare Candies:</b><br>"
        "The number of rare candies in your bag. Make sure these are at the top position of the bag.<br>"
        "Rare candies used during calibration will be restored after resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0, 999 // default, min, max
    )
    , MAX_BALL_THROWS(
        "<b>Max Balls Thrown:</b><br>"
        "The number of " + STRING_POKEBALL + "s in your bag to attempt to throw. Make sure these are at the top position of the bag.<br>"
        "Balls thrown during calibration will be restored after resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20, 1, 999 // default, min, max
    )
    , SEED(
        false,
        "<b>Target Seed:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "70FE", "70FE",
        true
    )
    , SEED_LIST(
        "<b>Nearby Seeds:</b><br>"
        "This box should contain a list of seeds (in order) around and including your target seed, with one seed on each line",
        LockMode::LOCK_WHILE_RUNNING,
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E", 
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        true
    )
    , SEED_BUTTON(
        "<b>Seed Button:</b><br>",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
    )
    , EXTRA_BUTTON(
        "<b>Extra Button:</b><br>"
        "Additional button presses that affect the seed.",
        {
            {BlackoutButton::None, "None", "None"},
            {BlackoutButton::L, "L", "Blackout L"},
            {BlackoutButton::R, "R", "Blackout R"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BlackoutButton::None
    )
    , SEED_DELAY(
        "<b>Seed Delay Time (ms):</b><br>The delay between starting the game and advancing past the title screen. Set this to match your target seed.",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 30400 // default, min
    )
    , ADVANCES(
        "<b>Advances:</b><br>The total number of RNG advances for your target.<br>This should be the combined amount of continue screen and in-game advances.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 600, 1000000000 // default, min
    )
    // , CONTINUE_SCREEN_FRAMES(
    //     "<b>Continue Screen Frames:</b><br>The number of RNG advances to pass on the continue screen.<br>This should be less than the total number of advances above.",
    //     LockMode::LOCK_WHILE_RUNNING,
    //     1000, 192 // default, min
    // )
    , USE_TEACHY_TV(
        "<b>Use Teachy TV:</b>"
        "<br>Opens the Teachy TV to quickly advance the RNG at 313x speed.<br>"
        "<i>Warning: can result in larger misses.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        false // default
    )
    , PROFILE(
        "<b>User Profile Position:</b><br>"
        "The position, from left to right, of the Switch profile with the FRLG save you'd like to use.<br>"
        "If this is set to 0, Switch 1 defaults to the last-used profile, while Switch 2 defaults to the first profile (position 1)",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 8 // default, min, max
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>Record a video when the shiny is found.", 
        LockMode::LOCK_WHILE_RUNNING, 
        true // default
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(RNG_FILTERS);
    PA_ADD_OPTION(RNG_CALIBRATION);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(GAME_VERSION);
    PA_ADD_OPTION(ENCOUNTER_TYPE);
    PA_ADD_OPTION(GAME_LOCATION);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(MAX_BALL_THROWS);
    PA_ADD_OPTION(MAX_RARE_CANDIES);
    PA_ADD_OPTION(SEED);
    PA_ADD_OPTION(SEED_LIST);
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(EXTRA_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(ADVANCES);
    // PA_ADD_OPTION(CONTINUE_SCREEN_FRAMES);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool WildRng::have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit){
    return (hit.seed == TARGET_SEED) && (hit.advance == ADVANCES);
}

bool WildRng::auto_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, WildRng_Descriptor::Stats& stats, const uint64_t& MAX_BALL_THROWS){
    for (uint64_t i=0; i<=MAX_BALL_THROWS; i++){
        int count = 0;
        while(true){
            if (count >= 10){
                send_program_recoverable_error_notification(
                    env, NOTIFICATION_ERROR_RECOVERABLE,
                    "auto_catch(): failed to detect battle menu"
                ); 
                stats.errors++;
                return false;
            }
            count++;

            BattleMenuWatcher battle_menu(COLOR_RED);
            PartyMenuWatcher party_menu(COLOR_RED);
            DexRegistrationWatcher dex_registration(COLOR_RED);
            BlackScreenWatcher black_screen(COLOR_RED);
            context.wait_for_all_requests();
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    for (int i=0; i<60; i++){
                        pbf_press_button(context, BUTTON_B, 200ms, 300ms);
                    }
                },
                { battle_menu, party_menu, black_screen },
                10ms
            );

            int start_ret;
            switch (ret){
            case 0:
                env.log("Battle menu detected");
                break;
            case 1:
                env.log("Party menu detected. Attempting to send out next Pokemon");
                pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 1000ms);
                continue;
            case 2:
                env.log("Dex registration detected. Exiting battle...");
                pbf_mash_button(context, BUTTON_B, 5000ms);
                return false;
            case 3:
                env.log("Black screen detected. Battle exited.");
                return false;
            default:
                env.log("No recognized state. Try checking if in the overworld...");
                StartMenuWatcher start_menu;
                context.wait_for_all_requests();
                start_ret = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context) {
                        for (int i=0; i<3; i++){
                            pbf_press_button(context, BUTTON_PLUS, 200ms, 2800ms);
                            pbf_mash_button(context, BUTTON_B, 500ms);
                        }
                    },
                    { start_menu }
                );
                if (start_ret < 0){
                    send_program_recoverable_error_notification(
                        env, NOTIFICATION_ERROR_RECOVERABLE,
                        "auto_catch(): no recognized state after 30 seconds."
                    ); 
                    stats.errors++;
                    return true;
                }
                env.log("Overworld detected.");
                pbf_mash_button(context, BUTTON_B, 500ms);
                context.wait_for_all_requests();
                return false;
            }

            break;
        }

        if (i == MAX_BALL_THROWS) { break; }

        // select BAG (selection arrow does not wrap around)
        pbf_move_left_joystick(context, {+1, 0}, 100ms, 150ms);
        pbf_move_left_joystick(context, {0, +1}, 100ms, 150ms);
        pbf_move_left_joystick(context, {+1, 0}, 100ms, 150ms);
        pbf_move_left_joystick(context, {0, +1}, 100ms, 150ms);

        BagWatcher bag_open(COLOR_RED);
        int ret2 = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                for (int i=0; i<5; i++){
                    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
                }
            },
            { bag_open }
        );
        if (ret2 < 0){
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "auto_catch(): failed to open bag."
            ); 
            stats.errors++;
            return true;
        }

        if (i == 0){
            // go to balls pocket (pockets do not wrap around, topmost item will already be selected)
            pbf_move_left_joystick(context, {+1, 0}, 200ms, 800ms);
            pbf_move_left_joystick(context, {+1, 0}, 200ms, 800ms);
            pbf_move_left_joystick(context, {+1, 0}, 200ms, 800ms);
        }

        // use ball
        pbf_mash_button(context, BUTTON_A, 5s);
    }

    env.log("auto_catch(): ran out of balls.");
    return true;
}

AdvObservedPokemon WildRng::read_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::set<std::string>& SPECIES_LIST){
    // navigate to the summary page of the last occupied (not necessarily 6th) party slot
    open_party_menu_from_overworld(env.console, context);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);

    SummaryWatcher page_one(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            pbf_press_button(context, BUTTON_A, 200ms, 300ms);
            for (int i=0; i<5; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
            }
        },
        { page_one }
    );

    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "read_summary(): Failed to detect first summary screen.",
            env.console
        ); 
    }

    // read stats
    PokemonFRLG_Stats stats;
    StatsReader reader(COLOR_RED);

    env.log("Reading Page 1 (Name, Level, Nature, Gender)...");
    VideoSnapshot screen1 = env.console.video().snapshot();
    reader.read_page1(env.logger(), LANGUAGE, screen1, stats, SPECIES_LIST);

    SummaryPage2Watcher page_two(COLOR_RED);
    context.wait_for_all_requests();
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<5; i++){
                pbf_press_dpad(context, DPAD_RIGHT, 200ms, 1800ms);
            }
        },
        { page_two }
    );

    if (ret2 < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "read_summary(): Failed to detect second summary screen.",
            env.console
        ); 
    }

    env.log("Reading Page 2 (Stats)...");
    VideoSnapshot screen2 = env.console.video().snapshot();
    reader.read_page2(env.logger(), screen2, stats);

    StatReads statreads = {
        static_cast<int16_t>(stats.hp.value_or(0)),
        static_cast<int16_t>(stats.attack.value_or(0)),
        static_cast<int16_t>(stats.defense.value_or(0)),
        static_cast<int16_t>(stats.sp_attack.value_or(0)),
        static_cast<int16_t>(stats.sp_defense.value_or(0)),
        static_cast<int16_t>(stats.speed.value_or(0))
    };

    AdvGender gender;
    switch(stats.gender.value_or(SummaryGender::Genderless)){
    case SummaryGender::Male:
        gender = AdvGender::Male;
        break;
    case SummaryGender::Female:
        gender = AdvGender::Female;
        break;
    default:
        gender = AdvGender::Any;
        break;
    }

    AdvObservedPokemon pokemon = {
        stats.name,
        gender,
        string_to_nature(stats.nature),
        AdvAbility::Any,
        { uint8_t(stats.level.value_or(5)) },
        { statreads },
        { {0,0,0,0,0,0} },
        AdvShinyType::Any
    };

    return pokemon;
}

bool WildRng::use_rare_candy(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    WildRng_Descriptor::Stats& stats,
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& BASE_STATS,
    bool first
){
    // navigate to the bag (only needed for the first use)
    if (first){
        open_bag_from_overworld(env.console, context);
        // move left to the correct pocket (in case Teachy TV was used)
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
    }

    // use rare candy and watch for the party screen
    PartyMenuWatcher party_menu(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<5; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            }
        },
        { party_menu }
    );
    if (ret < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "use_rare_candy(): failed to detect party menu."
        ); 
        stats.errors++;
        return true;
    }

    // select the last party slot (unknown how full the party is, so we can't detect a particular slot)
    // only needed on the first use
    if (first){
        context.wait_for_all_requests();
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    }

    // watch for level up stats
    PartyLevelUpWatcher level_up(COLOR_RED, PartyLevelUpDialog::stats);
    context.wait_for_all_requests();
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<30; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            }
        },
        { level_up }
    );
    if (ret2 < 0){
        send_program_recoverable_error_notification(
            env, NOTIFICATION_ERROR_RECOVERABLE,
            "use_rare_candy(): failed to detect level-up stats."
        ); 
        stats.errors++;
        return true;
    }

    PartyLevelUpReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading stats...");
    VideoSnapshot screen = env.console.video().snapshot();
    StatReads statreads = reader.read_stats(env.logger(), screen);    

    update_filters(filters, pokemon, statreads, {}, BASE_STATS);
    RNG_FILTERS.set(filters);   

    // return to the bag (possibly learning a move, but trying to prevent evolution)
    int attempts = 0;
    while (true){
        if (attempts > 5){
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "use_rare_candy(): failed to return to bag menu in 5 attempts."
            );
            stats.errors++;
            return true;
        }
        BagWatcher bag_menu(COLOR_RED);
        PartyMoveLearnWatcher move_learn(COLOR_RED);
        context.wait_for_all_requests();
        int ret3 = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                for (int i=0; i<15; i++){
                    pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
                }
            },
            { bag_menu, move_learn }
        );
        attempts++;
        switch (ret3){
        case 0:
            env.log("Returned to bag.");
            return false;
        case 1:
            env.log("Move learn opportunity detected.");
            // don't learn move
            pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
            pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            continue;
        default:
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "use_rare_candy(): failed to return to bag menu."
            ); 
            stats.errors++;
            return true;
        }
    }
}


void WildRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    WildRng_Descriptor::Stats& stats = env.current_stats<WildRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    RNG_FILTERS.reset();
    RNG_CALIBRATION.reset();

    // prepare database of base stats and gender thresholds
    RngStatsDatabase stats_data("PokemonFRLG/BaseStats.json");

    // get the relevant encounter slots
    EncountersDatabase encounters_data(GAME_VERSION == GameVersion::firered ? "PokemonFRLG/EncounterSlotsFR.json" : "PokemonFRLG/EncounterSlotsLG.json");

    EncounterType enc = ENCOUNTER_TYPE;
    int enc_idx = static_cast <int> (enc);
    auto enc_entry = ENCOUNTER_TYPE.database().find(enc_idx);
    std::string enc_slug = enc_entry->slug;

    GameLocation loc = GAME_LOCATION;
    int loc_idx = static_cast <int> (loc);
    auto loc_entry = GAME_LOCATION.database().find(loc_idx);
    std::string loc_slug = loc_entry->slug;

    std::map<std::string, std::vector<AdvEncounterSlot>> location_map = encounters_data.get_throw(enc_slug);
    if (location_map.find(loc_slug)==location_map.end()){
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "Invalid combination for encounter type and location.",
            env.console
        ); 
    }

    std::vector<AdvEncounterSlot> ENCOUNTER_SLOTS = location_map.find(loc_slug)->second;

    std::set<std::string> SPECIES_LIST;
    for (auto slot : ENCOUNTER_SLOTS){
        SPECIES_LIST.emplace(slot.species);
    }

    const bool SUPER_ROD = ENCOUNTER_TYPE == EncounterType::superrod;



    // prepare timings

    const uint16_t TARGET_SEED = parse_seed(env.console, SEED);
    const std::vector<uint16_t> SEED_VALUES = parse_seed_list(env.console, SEED_LIST);
    const int16_t SEED_POSITION = seed_position_in_list(TARGET_SEED, SEED_VALUES);

    if (SEED_POSITION == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "WildRng(): Target Seed is missing from the list of nearby seeds.",
            env.console
        ); 
    }

    env.log("Target Seed Value (base10): " + std::to_string(TARGET_SEED));

    PokemonFRLG_RngTarget TARGET = PokemonFRLG_RngTarget::sweetscent;

    bool safari_zone = (
        GAME_LOCATION == GameLocation::safari_zone_area_1_east  || 
        GAME_LOCATION == GameLocation::safari_zone_area_2_north || 
        GAME_LOCATION == GameLocation::safari_zone_area_3_west  || 
        GAME_LOCATION == GameLocation::safari_zone_entrance
    );
    switch (ENCOUNTER_TYPE){
        case EncounterType::rocksmash:
            TARGET = PokemonFRLG_RngTarget::rocksmash;
            break;
        case EncounterType::grass:
            if (safari_zone){
                if (GAME_LOCATION == GameLocation::safari_zone_area_1_east){
                    TARGET = PokemonFRLG_RngTarget::safarizoneeast;
                }else if (GAME_LOCATION == GameLocation::safari_zone_area_2_north){
                    TARGET = PokemonFRLG_RngTarget::safarizonenorth;
                }else if (GAME_LOCATION == GameLocation::safari_zone_area_3_west){
                    TARGET = PokemonFRLG_RngTarget::safarizonewest;
                }else{
                    TARGET = PokemonFRLG_RngTarget::safarizonecenter;
                }
                break;
            }
        case EncounterType::surfing:
            TARGET = safari_zone ? PokemonFRLG_RngTarget::safarizonesurf : PokemonFRLG_RngTarget::sweetscent;
            break;
        case EncounterType::oldrod:
        case EncounterType::goodrod:
        case EncounterType::superrod:
            TARGET = safari_zone ? PokemonFRLG_RngTarget::safarizonefish : PokemonFRLG_RngTarget::fishing;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "WildRng(): Unrecognized encounter type",
                env.console
            ); 
    }

    const double FRAMERATE = 59.999977; // FPS
    const double FRAME_DURATION = 1000 / FRAMERATE;

    uint8_t MAX_HISTORY_LENGTH = USE_TEACHY_TV ? 2 : 10;
    double SEED_BUMPS[] = {0, 1, -1, 2, -2};

    uint64_t CONTINUE_SCREEN_FRAMES = 200;

    const int64_t FIXED_SEED_OFFSET = -845; // milliseconds. approximate;
    double SEED_CALIBRATION_FRAMES = RNG_CALIBRATION.seed_calibration / FRAME_DURATION;
    double ADVANCES_CALIBRATION = RNG_CALIBRATION.advances_calibration;
    double CONTINUE_SCREEN_ADJUSTMENT = RNG_CALIBRATION.csf_calibration;    


    AdvRngWildSearcher searcher(TARGET_SEED, ADVANCES, ENCOUNTER_SLOTS, AdvRngMethod::Any);
    AdvWildPokemonResult target_result = searcher.generate_pokemon();
    env.log("Target Species: " + target_result.species);
    env.log("Target Level: " + std::to_string(target_result.level));
    env.log("Target Encounter Slot: " + std::to_string(target_result.slot));
    env.log("Target PID (base10): " + std::to_string(target_result.pid));

    RngAdvanceHistory ADVANCE_HISTORY;
    RngCalibrationHistory CALIBRATION_HISTORY; 
    uint64_t INITIAL_ADVANCES_RADIUS = USE_TEACHY_TV ? 8192 : 1024;
    uint64_t resets = 0;

    while (true){
        if (CALIBRATION_HISTORY.results.size() > 0){
            env.log("Checking for nonshiny target hit...");
            if (have_hit_target(env, TARGET_SEED, CALIBRATION_HISTORY.results.back())){
                env.log("Target Hit!");
                stats.nonshiny++;
                break;
            }
            env.log("Missed target.");
        }

        if (resets > MAX_RESETS){
            env.log("Max resets reached.");
            break;
        }

        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Calibrating."
        );
        env.update_stats();

        uint64_t advances_radius = INITIAL_ADVANCES_RADIUS;
        for (size_t i=0; i<CALIBRATION_HISTORY.results.size(); i++){
            advances_radius = advances_radius / 2;
            if (advances_radius <= 4){
                advances_radius = 4;
                break;
            }
        }
        env.log("Advances search radius: " + std::to_string(advances_radius));

        if (CALIBRATION_HISTORY.results.size() > 0){
            SEED_CALIBRATION_FRAMES = get_seed_calibration_frames(CALIBRATION_HISTORY, SEED_VALUES, SEED_POSITION);
            ADVANCES_CALIBRATION = get_advances_calibration_frames(CALIBRATION_HISTORY, ADVANCES);
        }

        if (CALIBRATION_HISTORY.results.size() > 0){
            AdvRngState prev_hit = CALIBRATION_HISTORY.results.back();
            double prev_csf_calibration = CALIBRATION_HISTORY.continue_screen_adjustments.back();
            int64_t prev_advance_miss = int64_t(prev_hit.advance) - int64_t(ADVANCES);
            if (prev_advance_miss != 0 && std::abs(prev_advance_miss) < 2){
                env.log("Attempting to correct for off-by-one miss by modifying continue screen frames.");
                if (prev_advance_miss > 0){
                    CONTINUE_SCREEN_ADJUSTMENT = prev_csf_calibration - 0.5;
                }else{
                    CONTINUE_SCREEN_ADJUSTMENT = prev_csf_calibration + 0.5;
                }
                CONTINUE_SCREEN_ADJUSTMENT = fmod(CONTINUE_SCREEN_ADJUSTMENT, 2);
            }
        }

        // if previous resets had uncertain advances, slightly modify the seed delay to try to hit a different target
        double seed_bump = SEED_BUMPS[ADVANCE_HISTORY.results.size() % 5];
        SEED_CALIBRATION_FRAMES += seed_bump;

        double CALIBRATED_ADVANCES = ADVANCES + ADVANCES_CALIBRATION;
        double INGAME_ADVANCES = CALIBRATED_ADVANCES - CONTINUE_SCREEN_FRAMES - CONTINUE_SCREEN_ADJUSTMENT;

        double TEACHY_ADVANCES = 0;
        bool should_use_teachy_tv = USE_TEACHY_TV && (INGAME_ADVANCES > 5000); // don't use Teachy TV for short in-game advance targets
        if (should_use_teachy_tv) {
            TEACHY_ADVANCES = std::floor((INGAME_ADVANCES - 5000) / 313) * 313;
        }

        env.log("Seed calibration (frames): " + std::to_string(SEED_CALIBRATION_FRAMES));
        env.log("Advance calibration (frames / 2): " + std::to_string(ADVANCES_CALIBRATION));
        env.log("Continue screen adjustment (frames): " + std::to_string(CONTINUE_SCREEN_ADJUSTMENT));

        uint64_t CALIBRATED_SEED_DELAY = uint64_t(std::round(SEED_DELAY + FIXED_SEED_OFFSET + FRAME_DURATION * SEED_CALIBRATION_FRAMES));
        uint64_t CONTINUE_SCREEN_DELAY =  uint64_t(std::round(FRAME_DURATION * (CONTINUE_SCREEN_FRAMES + CONTINUE_SCREEN_ADJUSTMENT)));
        uint64_t TEACHY_DELAY = uint64_t(TEACHY_ADVANCES * FRAME_DURATION / 313);
        uint64_t INGAME_DELAY = uint64_t(std::round(FRAME_DURATION * (INGAME_ADVANCES - TEACHY_ADVANCES) / 2)) - (should_use_teachy_tv ? 13700 : 0);

        env.log("Title screen duration: " + std::to_string(CALIBRATED_SEED_DELAY) + "ms");
        env.log("Continue screen duration: " + std::to_string(CONTINUE_SCREEN_DELAY) + "ms");
        if (should_use_teachy_tv){
            env.log("Teachy TV duration: " + std::to_string(TEACHY_DELAY) + "ms");
            env.log("Non-Teachy TV in-game duration: " + std::to_string(INGAME_DELAY) + "ms");
        }else{
            env.log("In-game duration: " + std::to_string(INGAME_DELAY) + "ms");
        }

        env.log("Resetting Game...");
        reset_and_perform_blind_sequence(
            env.console, context, TARGET, 
            SEED_BUTTON, EXTRA_BUTTON, CALIBRATED_SEED_DELAY, 
            CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, 
            false, PROFILE
        );
        stats.resets++; 

        RNG_FILTERS.reset();
        RNG_CALIBRATION.reset();

        int ret = watch_for_shiny_encounter(env.console, context);
        if (ret < 1){
            if (TARGET == PokemonFRLG_RngTarget::fishing || TARGET == PokemonFRLG_RngTarget::rocksmash){
                env.log("No battle triggered. Resetting...");
            }else{
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "WildRng(): Failed to trigger battle",
                    env.console
                ); 
            }
        }
        bool shiny_found = (ret == 1);

        if (shiny_found){
            env.log("Shiny found!");
            stats.shinies++;
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            break;
        }

        bool failed = auto_catch(env, context, stats, MAX_BALL_THROWS);
        if (failed){
            env.log("Failed catch.");
            continue;
        }

        AdvObservedPokemon pokemon = read_summary(env, context, SPECIES_LIST);
        RngStats species_stats = stats_data.get_throw(pokemon.species);
        BaseStats BASE_STATS = species_stats.base_stats;
        int16_t GENDER_THRESHOLD = species_stats.gender_threshold;

        AdvRngFilters filters = observation_to_filters(pokemon, BASE_STATS, AdvRngMethod::Any);
        RNG_FILTERS.set(filters);

        std::vector<AdvRngState> search_hits = get_wild_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD, SUPER_ROD);
        RNG_CALIBRATION.set(
            SEED_CALIBRATION_FRAMES * FRAME_DURATION,
            CONTINUE_SCREEN_ADJUSTMENT,
            ADVANCES_CALIBRATION - CONTINUE_SCREEN_ADJUSTMENT,
            search_hits
        );        
        bool finished = update_history(env.console, ADVANCE_HISTORY, CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, CONTINUE_SCREEN_ADJUSTMENT, search_hits, 1);
        finished = finished || all_indistinguishable(search_hits, searcher, SUPER_ROD);
        if (finished || (MAX_RARE_CANDIES == 0)){
            env.log("RNG search finished.");
            continue;
        }

        for (uint64_t i=0; i<MAX_RARE_CANDIES; i++){
            failed = use_rare_candy(env, context, stats, pokemon, filters, BASE_STATS, i == 0);

            search_hits = get_wild_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD, SUPER_ROD);
            RNG_CALIBRATION.set(
                SEED_CALIBRATION_FRAMES * FRAME_DURATION,
                CONTINUE_SCREEN_ADJUSTMENT,
                ADVANCES_CALIBRATION - CONTINUE_SCREEN_ADJUSTMENT,
                search_hits
            );

            bool force_finish = failed || (i == (MAX_RARE_CANDIES - 1));
            finished = update_history(
                env.console, ADVANCE_HISTORY, 
                CALIBRATION_HISTORY, MAX_HISTORY_LENGTH, 
                SEED_CALIBRATION_FRAMES, ADVANCES_CALIBRATION, 
                CONTINUE_SCREEN_ADJUSTMENT, search_hits, 
                1, 2, force_finish
            );
            finished = finished || all_indistinguishable(search_hits, searcher, SUPER_ROD);

            if (finished){
                break;
            }
        }

    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}



}
}
}
