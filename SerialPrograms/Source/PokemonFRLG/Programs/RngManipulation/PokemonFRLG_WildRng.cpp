/*  Wild RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_LocationsDatabase.h"
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
    , LOCATIONS_DATABASE(make_locations_database("PokemonFRLG/Locations.json"))
    , GAME_LOCATION(
        "<b>Location:</b>",
        LOCATIONS_DATABASE,
        LockMode::LOCK_WHILE_RUNNING,
        "route_1"
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
        "<b>Seed Delay Time (ms):</b><br>"
        "The delay between starting the game and advancing past the title screen. Set this to match your target seed.<br>"
        "<i>If using Ten Lines, select <b>Nintendo Switch 1</b> as your console even if using a Switch 2.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 30400 // default, min
    )
    , ADVANCES(
        "<b>Advances:</b><br>The total number of RNG advances for your target.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 700, 1000000000 // default, min
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

void WildRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    WildRng_Descriptor::Stats& stats = env.current_stats<WildRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    RNG_FILTERS.reset();
    RNG_CALIBRATION.reset();

    // prepare database of base stats and gender thresholds
    RngStatsDatabase STATS_DATA("PokemonFRLG/BaseStats.json");

    // get the relevant encounter slots
    EncountersDatabase encounters_data(GAME_VERSION == GameVersion::firered ? "PokemonFRLG/EncounterSlotsFR.json" : "PokemonFRLG/EncounterSlotsLG.json");

    EncounterType enc = ENCOUNTER_TYPE;
    int enc_idx = static_cast <int> (enc);
    auto enc_entry = ENCOUNTER_TYPE.database().find(enc_idx);
    std::string enc_slug = enc_entry->slug;


    std::string loc_slug = GAME_LOCATION.slug();

    std::map<std::string, std::vector<AdvEncounterSlot>> location_map = encounters_data.get_throw(enc_slug);
    if (location_map.find(loc_slug)==location_map.end()){
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "Invalid combination for encounter type and location.",
            env.console
        ); 
    }

    std::vector<AdvEncounterSlot> ENCOUNTER_SLOTS = location_map.find(loc_slug)->second;
    env.log("Encounter slots");
    for (size_t i=0; i<ENCOUNTER_SLOTS.size(); i++){
        AdvEncounterSlot& slot = ENCOUNTER_SLOTS[i];
        env.log("   Slot " + std::to_string(i) + ": " + slot.species + " " + std::to_string(slot.minlevel) + "-" + std::to_string(slot.maxlevel));
    }    

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
        loc_slug == "safari_zone_area_1_east"  || 
        loc_slug == "safari_zone_area_2_north" || 
        loc_slug == "safari_zone_area_3_west"  || 
        loc_slug == "safari_zone_entrance"
    );
    switch (ENCOUNTER_TYPE){
        case EncounterType::rocksmash:
            TARGET = PokemonFRLG_RngTarget::rocksmash;
            break;
        case EncounterType::grass:
            if (safari_zone){
                if (loc_slug == "safari_zone_area_1_east"){
                    TARGET = PokemonFRLG_RngTarget::safarizoneeast;
                }else if (loc_slug == "safari_zone_area_2_north"){
                    TARGET = PokemonFRLG_RngTarget::safarizonenorth;
                }else if (loc_slug == "safari_zone_area_3_west"){
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
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "WildRng(): Unrecognized encounter type",
                env.console
            ); 
    }

    static const int64_t FIXED_SEED_OFFSET = -845; // milliseconds, approximate
    static const int64_t FIXED_ADVANCES_OFFSET = -352; // frames, approximate

    static const uint64_t CONTINUE_SCREEN_FRAMES = 200;

    static const double SEED_BUMPS[] = {0, 1, -1, 2, -2};

    const uint64_t INITIAL_ADVANCES_RADIUS = USE_TEACHY_TV ? 4096 : 1024;

    const uint8_t MAX_HISTORY_LENGTH = USE_TEACHY_TV ? 2 : 10;


    RngCalibrations calibrations = {
        RNG_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
        RNG_CALIBRATION.csf_calibration,
        RNG_CALIBRATION.advances_calibration
    };

    AdvRngWildSearcher searcher(TARGET_SEED, ADVANCES, ENCOUNTER_SLOTS, AdvRngMethod::Any);
    AdvWildPokemonResult target_result = searcher.generate_pokemon(SUPER_ROD);
    env.log("Target Species: " + target_result.species);
    env.log("Target Level: " + std::to_string(target_result.level));
    env.log("Target Encounter Slot: " + std::to_string(target_result.slot));
    env.log("Target PID (base10): " + std::to_string(target_result.pid));
    env.log("Target Nature: " + nature_to_string(target_result.nature));
    env.log("Target IVs (assuming Method 1):");
    env.log("   HP: " + std::to_string(target_result.ivs.hp));
    env.log("   Atk: " + std::to_string(target_result.ivs.attack));
    env.log("   Def: " + std::to_string(target_result.ivs.defense));
    env.log("   SpA: " + std::to_string(target_result.ivs.spatk));
    env.log("   SpD: " + std::to_string(target_result.ivs.spdef));
    env.log("   Spe: " + std::to_string(target_result.ivs.speed));
    RngAdvanceHistory advance_history;
    RngCalibrationHistory calibration_history; 

    uint16_t failed_searches = 0;

    while (true){
        if (calibration_history.results.size() > 0){
            env.log("Checking for nonshiny target hit...");
            if (have_hit_target(env, TARGET_SEED, calibration_history.results.back())){
                env.log("Target Hit!");
                stats.nonshiny++;
                break;
            }
            env.log("Missed target.");
        }

        if (failed_searches >= 5){
            env.log("Failed to find any matches 5 times in a row");
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "Failed to find any matches 5 times in a row. Check your seed and advances settings.",
                env.console
            ); 
            break;
        }

        if (stats.resets > MAX_RESETS){
            env.log("Max resets reached.");
            break;
        }

        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Calibrating."
        );
        env.update_stats();

        uint64_t advances_radius = get_advances_radius(env.console, calibration_history, INITIAL_ADVANCES_RADIUS);

        if (calibration_history.results.size() > 0){
            calibrations = get_calibrations(env.console, calibration_history, SEED_VALUES, SEED_POSITION, ADVANCES);
        }

        // if previous resets had uncertain advances, slightly modify the seed delay to try to hit a different target
        double seed_bump = SEED_BUMPS[advance_history.results.size() % 5];
        calibrations.seed_offset += seed_bump;

        uint64_t ingame_advances = ADVANCES - CONTINUE_SCREEN_FRAMES;

        RngTimings timings = prepare_timings(
            env.console, TARGET,
            SEED_DELAY, CONTINUE_SCREEN_FRAMES, ingame_advances,
            USE_TEACHY_TV, calibrations,
            FIXED_SEED_OFFSET, FIXED_ADVANCES_OFFSET
        );

        env.log("Resetting Game...");
        reset_and_perform_blind_sequence(
            env.console, context, TARGET, 
            SEED_BUTTON, EXTRA_BUTTON, timings, 
            safari_zone, PROFILE
        );
        stats.resets++; 

        RNG_FILTERS.reset();
        RNG_CALIBRATION.reset();

        int ret = watch_for_shiny_encounter(env.console, context);
        if (ret < 0){
            if (TARGET == PokemonFRLG_RngTarget::fishing || TARGET == PokemonFRLG_RngTarget::rocksmash){
                env.log("No battle triggered. Resetting...");
                continue;
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

        int balls_thrown = auto_catch(env.console, context, MAX_BALL_THROWS, safari_zone);
        if (balls_thrown < 0){
            stats.errors++;
            send_program_recoverable_error_notification(
                env, NOTIFICATION_ERROR_RECOVERABLE,
                "auto_catch() encountered an error."
            ); 
            continue;
        }else if(balls_thrown == 0){
            env.log("Failed catch.");
            continue;
        }

        go_to_summary(env.console, context, 0, safari_zone ? StartMenuContext::SAFARI_ZONE : StartMenuContext::STANDARD);
        AdvObservedPokemon pokemon = read_summary(env.console, context, LANGUAGE, SPECIES_LIST);
        RngStats species_stats;
        try{
            species_stats = STATS_DATA.get_throw(pokemon.species);        
        }catch (const InternalProgramError& err){
            env.log(err.message());
            env.log("Failed to load base stats.");
            continue;
        }
        BaseStats base_stats = species_stats.base_stats;
        int16_t gender_threshold = species_stats.gender_threshold;

        AdvRngFilters filters = observation_to_filters(pokemon, base_stats, AdvRngMethod::Any);
        RNG_FILTERS.set(filters);

        std::vector<AdvRngState> search_hits = get_wild_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, gender_threshold, SUPER_ROD);
        RNG_CALIBRATION.set(
            calibrations.seed_offset * FRLG_FRAME_DURATION,
            calibrations.csf_offset,
            calibrations.ingame_offset,
            search_hits
        );           
        bool finished = update_history(env.console, advance_history, calibration_history, MAX_HISTORY_LENGTH, calibrations, search_hits, 1);
        finished = finished || all_indistinguishable(search_hits, searcher, gender_threshold, SUPER_ROD);

        for (uint64_t i=0; i<MAX_RARE_CANDIES; i++){
            if (finished){
                break;
            }
            bool failed = use_rare_candy(env.console, context, LANGUAGE, pokemon, filters, base_stats, AdvRngMethod::Any, safari_zone, i == 0);
            if (failed) {
                stats.errors++;
                send_program_recoverable_error_notification(
                    env, NOTIFICATION_ERROR_RECOVERABLE,
                    "Failed to use Rare Candy."
                ); 
            }

            search_hits = get_wild_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, gender_threshold, SUPER_ROD);
            RNG_CALIBRATION.set(
                calibrations.seed_offset * FRLG_FRAME_DURATION,
                calibrations.csf_offset,
                calibrations.ingame_offset,
                search_hits
            );       

            bool force_finish = failed || (i == (MAX_RARE_CANDIES - 1));
            finished = update_history(
                env.console, advance_history, 
                calibration_history, MAX_HISTORY_LENGTH,
                calibrations, search_hits, 
                1, 2, force_finish
            );
            finished = finished || all_indistinguishable(search_hits, searcher, gender_threshold, SUPER_ROD);
        }

        env.log("RNG search finished.");
        if (search_hits.size() == 0){
            failed_searches++;
        }else{
            failed_searches = 0;
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
