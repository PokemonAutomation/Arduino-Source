/*  Static RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Language.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_StaticRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


StaticRng_Descriptor::StaticRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:StaticRng",
        Pokemon::STRING_POKEMON + " FRLG", "Static RNG",
        "Programs/PokemonFRLG/StaticRng.html",
        "Automatically calibrate timings to hit a specific RNG target for FRLG static encounters.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct StaticRng_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> StaticRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

StaticRng::StaticRng()
    : m_calibration_displays(
        "<font size=4><b>Calibration Displays</b></font> — These will update automatically as the program runs"
    )
    , m_game_info(
        "<font size=4><b>Game Information</b></font>"
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
    , m_target_settings(
        "<font size=4><b>Target Settings</b></font> — Get these from an RNG search tool"
    )
    , TARGET(
        "<b>Target:</b>",
        {
            {PokemonFRLG_RngTarget::electrode, "electrode", "Electrode"},
            {PokemonFRLG_RngTarget::snorlax, "snorlax", "Snorlax"},
            {PokemonFRLG_RngTarget::articuno, "articuno", "Articuno"},
            {PokemonFRLG_RngTarget::zapdos, "zapdos", "Zapdos"},
            {PokemonFRLG_RngTarget::moltres, "moltres", "Moltres"},
            {PokemonFRLG_RngTarget::mewtwo, "mewtwo", "Mewtwo"},
            {PokemonFRLG_RngTarget::hypno, "hypno", "Hypno"},
            {PokemonFRLG_RngTarget::hooh, "hooh", "Ho-oh"},
            {PokemonFRLG_RngTarget::lugia, "lugia", "Lugia"},
            {PokemonFRLG_RngTarget::deoxys_attack, "deoxys_attack", "Deoxys-Attack"},
            {PokemonFRLG_RngTarget::deoxys_defense, "deoxys_defense", "Deoxys-Defense"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        PokemonFRLG_RngTarget::electrode
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
        "<b>Seed Button:</b>",
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
        10000, 520, 1000000000 // default, min
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
        0, 0, 999 // default, min, max
    )
    , MAX_BALL_THROWS(
        "<b>Max Balls Thrown:</b><br>"
        "The number of " + STRING_POKEBALL + "s in your bag to attempt to throw. Make sure these are at the top position of the bag.<br>"
        "Balls thrown during calibration will be restored after resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20, 1, 999 // default, min, max
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
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(m_target_settings);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(SEED);
    PA_ADD_OPTION(SEED_LIST);
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(EXTRA_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(ADVANCES);
    PA_ADD_OPTION(m_program_settings);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(MAX_BALL_THROWS);
    PA_ADD_OPTION(MAX_RARE_CANDIES);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool StaticRng::have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit){
    return (hit.seed == TARGET_SEED) && (hit.advance == ADVANCES);
}

void StaticRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    StaticRng_Descriptor::Stats& stats = env.current_stats<StaticRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    RNG_FILTERS.reset();
    RNG_CALIBRATION.reset_hits();

    const uint16_t TARGET_SEED = parse_seed(env.console, SEED);
    const std::vector<uint16_t> SEED_VALUES = parse_seed_list(env.console, SEED_LIST);
    const int16_t SEED_POSITION = seed_position_in_list(TARGET_SEED, SEED_VALUES);

    if (SEED_POSITION == -1){
        throw UserSetupError(env.console, "The target Seed is missing from the list of nearby seeds.");
    }

    BaseStats BASE_STATS;
    int16_t GENDER_THRESHOLD = -1;
    switch (TARGET){
    case PokemonFRLG_RngTarget::electrode:
        BASE_STATS = { 60, 50, 70, 80, 80, 140 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::snorlax:
        BASE_STATS = { 160, 110, 65, 65, 110, 30 };
        GENDER_THRESHOLD = 30;
        break;
    case PokemonFRLG_RngTarget::articuno:
        BASE_STATS = { 90, 85, 100, 95, 125, 85 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::zapdos:
        BASE_STATS = { 90, 90, 85, 125, 90, 100 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::moltres:
        BASE_STATS = { 90, 100, 90, 125, 85, 90 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::mewtwo:
        BASE_STATS = { 106, 110, 90, 154, 90, 130 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::hypno:
        BASE_STATS = { 85, 73, 70, 73, 115, 67 };
        GENDER_THRESHOLD = 126;
        break;
    case PokemonFRLG_RngTarget::hooh:
        BASE_STATS = { 106, 130, 90, 110, 154, 90 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::lugia:
        BASE_STATS = { 106, 90, 130, 90, 154, 110 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::deoxys_attack:
        BASE_STATS = { 50, 180, 20, 180, 20, 150 };
        GENDER_THRESHOLD = -1;
        break;
    case PokemonFRLG_RngTarget::deoxys_defense:
        BASE_STATS = { 50, 70, 160, 70, 160, 90 };
        GENDER_THRESHOLD = -1;
        break;
    default:
        break;
    }

    static const int64_t FIXED_SEED_OFFSET = -845; // milliseconds, approximate
    static const int64_t FIXED_ADVANCES_OFFSET = 160; // frames, approximate
    
    static const uint64_t CONTINUE_SCREEN_FRAMES = 200;

    static const double SEED_BUMPS[] = {0, 1, -1, 2, -2};

    const uint8_t MAX_HISTORY_LENGTH = USE_TEACHY_TV ? 2 : 10;

    const uint64_t INITIAL_ADVANCES_RADIUS = USE_TEACHY_TV ? 4096 : 1024;

    static const std::set<std::string> SPECIES_LIST = {
        "electrode", "snorlax", 
        "articuno", "zapdos", "moltres", "mewtwo"
        "hypno", "ho-oh", "lugia", "deoxys" 
    };

    env.log("RNG Target: " + std::to_string(TARGET.current_value()));
    env.log("Target Seed: " + to_hex_string(TARGET_SEED));
    env.log("Target Advances: " + std::to_string(ADVANCES));

    AdvRngSearcher searcher(TARGET_SEED, ADVANCES, AdvRngMethod::Method1);
    AdvPokemonResult target_result = searcher.generate_pokemon();
    RNG_TARGET.set_target(target_result, GENDER_THRESHOLD);
    env.log("Target PID: " + to_hex_string(target_result.pid));
    env.log("Target Nature: " + nature_to_string(target_result.nature));
    env.log("Target IVs:");
    env.log("   HP: " + std::to_string(target_result.ivs.hp));
    env.log("   Atk: " + std::to_string(target_result.ivs.attack));
    env.log("   Def: " + std::to_string(target_result.ivs.defense));
    env.log("   SpA: " + std::to_string(target_result.ivs.spatk));
    env.log("   SpD: " + std::to_string(target_result.ivs.spdef));
    env.log("   Spe: " + std::to_string(target_result.ivs.speed));

    RngCalibrations calibrations = {
        RNG_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
        RNG_CALIBRATION.csf_calibration,
        RNG_CALIBRATION.advances_calibration
    };
    env.log("Initial Seed calibration (frames): " + std::to_string(calibrations.seed_offset));
    env.log("Initial CSF calibration (frames): " + std::to_string(calibrations.csf_offset));
    env.log("Initial In-game calibration (frames x2): " + std::to_string(calibrations.ingame_offset));

    Milliseconds launch_delay = INITIAL_LAUNCH_DELAY;

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
            throw UserSetupError(env.console, "The target Seed is missing from the list of nearby seeds.");
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
            launch_delay, false, PROFILE
        );
        stats.resets++;

        RNG_FILTERS.reset();
        RNG_CALIBRATION.set_calibrations(calibrations);
        RNG_CALIBRATION.reset_hits();

        bool shiny_found = check_for_shiny(env.console, context, TARGET);

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
        AdvObservedPokemon pokemon = read_summary(env.console, context, LANGUAGE, SPECIES_LIST);
        AdvRngFilters filters = observation_to_filters(pokemon, BASE_STATS);
        RNG_FILTERS.set(filters);

        std::vector<AdvRngState> search_hits = get_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
        RNG_CALIBRATION.set_hits(search_hits);      
        bool finished = update_history(
            env.console, advance_history, calibration_history, MAX_HISTORY_LENGTH, 
            calibrations, search_hits, 1
        );

        for (uint64_t i=0; i<MAX_RARE_CANDIES; i++){
            if (finished){
                break;
            }
            bool failed = use_rare_candy(env.console, context, LANGUAGE, pokemon, filters, BASE_STATS, AdvRngMethod::Method1, false, i == 0);
            if (failed) {
                stats.errors++;
                send_program_recoverable_error_notification(
                    env, NOTIFICATION_ERROR_RECOVERABLE,
                    "Failed to use Rare Candy."
                ); 
            }
            RNG_FILTERS.set(filters);

            search_hits = get_search_results(env.console, searcher, filters, SEED_VALUES, ADVANCES, advances_radius, GENDER_THRESHOLD);
            RNG_CALIBRATION.set_hits(search_hits);    

            bool force_finish = failed || (i == (MAX_RARE_CANDIES - 1));
            finished = update_history(
                env.console, advance_history, 
                calibration_history, MAX_HISTORY_LENGTH, 
                calibrations, search_hits, 
                1, 2, force_finish
            );
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
