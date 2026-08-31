/*  Roaming Legendary RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Language.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_RngLoopRoutines.h"
#include "PokemonFRLG_RoamingLegendaryRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


RoamingLegendaryRng_Descriptor::RoamingLegendaryRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:RoamingLegendaryRng",
        Pokemon::STRING_POKEMON + " FRLG", "Roaming Legendary RNG",
        "Programs/PokemonFRLG/RoamingLegendaryRng.html",
        "Automatically calibrate timings to hit a specific RNG target for FRLG Roaming Legendary " + STRING_POKEMON,
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct RoamingLegendaryRng_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> RoamingLegendaryRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

RoamingLegendaryRng::RoamingLegendaryRng()
    : m_calibration_displays(
        "<font size=4><b>Calibration Displays</b></font> — These will update automatically as the program runs"
    )
    , m_game_info(
        "<font size=4><b>Game Information</b></font>"
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
    , LANGUAGE(
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
    , SOUND(
        "<b>Sound:</b><br>"
        "Your in-game sound setting. This affects the possible seeds.",
        {
            {SoundSetting::Mono, "mono", "Mono"},
            {SoundSetting::Stereo, "stereo", "Stereo"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        SoundSetting::Mono
    )
    , m_target_settings(
        "<font size=4><b>Target Settings</b></font> — Get these from an RNG search tool"
    )
    , TARGET(
        "<b>Target:</b>",
        {
            {PokemonFRLG_RngTarget::raikou, "raikou", "Raikou"},
            {PokemonFRLG_RngTarget::entei, "entei", "Entei"},
            {PokemonFRLG_RngTarget::suicune, "suicune", "Suicune"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        PokemonFRLG_RngTarget::raikou
    )    
    , SEED(
        false,
        "<b>Target Seed:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "70FE", "70FE",
        true
    )
    , ADVANCES(
        "<b>Advances:</b><br>The total number of RNG advances for your target.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 520, 1000000000 // default, min, max
    )
    , m_program_settings(
        "<font size=4><b>Program Settings</b></font>"
    )
    , USE_TEACHY_TV(
        "<b>Use Teachy TV:</b>"
        "<br>Opens the Teachy TV to quickly advance the RNG at 313x speed.<br>"
        "<i>Warning: can result in larger misses.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        false // default
    )
    , SEED_RADIUS(
        "<b>Nearby Seed Radius:</b><br>"
        "The number of nearby seeds on each side of the target to search when identifying which seed was hit.",
        LockMode::LOCK_WHILE_RUNNING,
        5, 1 // default, min
    )
    , MAX_RESETS(
        "<b>Max Resets:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        50, 0 // default, min
    )
    , MAX_RARE_CANDIES(
        "<b>Max Rare Candies:</b><br>"
        "The number of rare candies in your bag. Make sure these are at the top position of the bag.<br>"
        "Rare candies used during calibration will be restored after resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0, 50 // default, min, max
    )
    , MAX_BALL_THROWS(
        "<b>Max Balls Thrown:</b><br>"
        "The number of " + STRING_POKEBALL + "s in your bag to attempt to throw. Make sure these are at the top position of the bag.<br>"
        "Balls thrown during calibration will be restored after resetting.<br>"
        "<i>Master Balls are strongly recommended for Roaming Legendary RNG.</i>",
        LockMode::UNLOCK_WHILE_RUNNING,
        30, 1, 999 // default, min, max
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
    PA_ADD_OPTION(m_calibration_displays);
    PA_ADD_OPTION(RNG_TARGET);
    PA_ADD_OPTION(RNG_FILTERS);
    PA_ADD_OPTION(RNG_CALIBRATION);
    PA_ADD_OPTION(m_game_info);
    PA_ADD_OPTION(GAME_VERSION);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SOUND);
    PA_ADD_OPTION(m_target_settings);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(SEED);
    PA_ADD_OPTION(ADVANCES);
    PA_ADD_OPTION(m_program_settings);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(SEED_RADIUS);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(MAX_RARE_CANDIES);
    PA_ADD_OPTION(MAX_BALL_THROWS);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void RoamingLegendaryRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    RoamingLegendaryRng_Descriptor::Stats& stats = env.current_stats<RoamingLegendaryRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    RNG_FILTERS.reset();
    RNG_CALIBRATION.reset_hits();

    const uint16_t TARGET_SEED = parse_seed(env.console, SEED);
    SeedsDatabase seeds_db(seeds_json_path(GAME_VERSION == GameVersion::firered, LANGUAGE));
    const SeedMatch seed_match = seeds_db.find_seed(TARGET_SEED, SOUND, SEED_RADIUS);
    if (!seed_match.found){
        throw UserSetupError(env.console, "The target Seed was not found in the seed database for this game version, language, and sound setting.");
    }
    const std::vector<uint16_t> SEED_VALUES = seed_match.seed_values;
    const int16_t SEED_POSITION = seed_match.seed_position;
    const uint64_t SEED_DELAY = seed_match.seed_delay;
    const SeedButton SEED_BUTTON = seed_match.seed_button;
    const BlackoutButton EXTRA_BUTTON = seed_match.extra_button;
    env.log("Seed delay: " + std::to_string(SEED_DELAY) + "ms, button mode: " + seed_match.button_mode
        + ", column: " + seed_match.column_name);

    BaseStats BASE_STATS;
    int16_t GENDER_THRESHOLD = -1;
    switch (TARGET){
    case PokemonFRLG_RngTarget::raikou:
        BASE_STATS = { 90, 85, 75, 115, 100, 115 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::entei:
        BASE_STATS = { 115, 115, 85, 90, 75, 100 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::suicune:
        BASE_STATS = { 100, 75, 115, 90, 115, 85 };
        GENDER_THRESHOLD = -1;
        break;
    default:
        break;
    }

    static const int64_t FIXED_SEED_OFFSET = -845; // milliseconds, approximate
    static const int64_t FIXED_ADVANCES_OFFSET = 160; // frames, approximate

    static const uint64_t CONTINUE_SCREEN_FRAMES = 200;

    const uint64_t INITIAL_ADVANCES_RADIUS = USE_TEACHY_TV ? 4096 : 1024;

    const uint8_t MAX_HISTORY_LENGTH = USE_TEACHY_TV ? 2 : 10;

    env.log("RNG Target: " + std::to_string(TARGET.current_value()));
    env.log("Target Seed: " + to_hex_string(TARGET_SEED));
    env.log("Target Advances: " + std::to_string(ADVANCES));

    AdvRngSearcher searcher(TARGET_SEED, ADVANCES, AdvRngMethod::Method1, true);
    AdvPokemonResult target_result = searcher.generate_pokemon();
    RNG_TARGET.set_target(target_result, GENDER_THRESHOLD);
    log_target_pokemon(env.console, target_result);

    RngCalibrations calibrations = {
        RNG_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
        RNG_CALIBRATION.csf_calibration,
        RNG_CALIBRATION.advances_calibration
    };
    log_calibrations(env.console, calibrations, true);

    Milliseconds launch_delay = INITIAL_LAUNCH_DELAY;

    RngUncertainHistory uncertain_history;
    RngCalibrationHistory calibration_history; 

    uint16_t failed_to_encounter = 0;
    uint16_t failed_searches = 0;

    while (true){
        if (calibration_history.results.size() > 0){
            env.log("Checking for nonshiny target hit...");
            if (have_hit_target(TARGET_SEED, ADVANCES, calibration_history.results.back())){
                env.log("Target Hit!");
                stats.nonshiny++;
                break;
            }
            env.log("Missed target.");
        }

        if (failed_searches >= 5){
            env.log("Failed to find any matches 5 times in a row");
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::NO_ERROR_REPORT,
                "Failed to find any matches 5 times in a row. Check your seed and advances settings.",
                env.console
            ); 
            break;
        }

        if (failed_to_encounter >= 5){
            env.log("Failed to encounter the Roaming Legendary 5 times in a row");
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::NO_ERROR_REPORT,
                "Failed to encounter the Roaming Legendary 5 times in a row",
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
        apply_seed_bump(calibrations, uncertain_history);

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
            launch_delay, false, PROFILE
        );
        stats.resets++; 

        RNG_FILTERS.reset();
        RNG_CALIBRATION.set_calibrations(calibrations);
        RNG_CALIBRATION.reset_hits();

        int ret = encounter_roamer(env.console, context, LANGUAGE, ROAMER_ROUTE1_SUBSET);
        if (ret < 0){
            failed_to_encounter++;
            continue;
        }
        failed_to_encounter = 0;
        bool shiny_found = (ret == 1);

        if (shiny_found){
            env.log("Shiny found!");
            stats.shinies++;
            RNG_CALIBRATION.hits.set("Shiny!");
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

        int balls_thrown = auto_catch(env.console, context, MAX_BALL_THROWS);
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

        go_to_summary(env.console, context);
        AdvObservedPokemon pokemon = read_summary(env.console, context, LANGUAGE, ROAMER_ROUTE1_SUBSET);
        pokemon.level[0] = 50;
        AdvRngFilters filters = observation_to_filters(pokemon, BASE_STATS);
        filters.ivs.defense = {0,0};
        filters.ivs.spatk   = {0,0};
        filters.ivs.spdef   = {0,0};
        filters.ivs.speed   = {0,0};
        RNG_FILTERS.set(filters);

        std::vector<AdvRngState> search_hits = refine_calibration_with_rare_candy(
            env, context, LANGUAGE, pokemon, filters, BASE_STATS,
            uncertain_history, calibration_history, calibrations,
            MAX_HISTORY_LENGTH, MAX_RARE_CANDIES, AdvRngMethod::Method1, false,
            stats.errors, NOTIFICATION_ERROR_RECOVERABLE,
            [&](AdvRngFilters& f){
                f.ivs.defense = {0,0};
                f.ivs.spatk   = {0,0};
                f.ivs.spdef   = {0,0};
                f.ivs.speed   = {0,0};
                return get_search_results(env.console, searcher, f, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
            },
            [&](const std::vector<AdvRngState>& h){ RNG_CALIBRATION.set_hits(h); },
            [&](const AdvRngFilters& f){ RNG_FILTERS.set(f); },
            [&](const std::vector<AdvRngState>& h){
                return all_equal(h) || all_indistinguishable(h, searcher, GENDER_THRESHOLD);
            }
        );

        env.log("RNG search finished.");
        update_failed_searches(failed_searches, search_hits);

    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}



}
}
}
