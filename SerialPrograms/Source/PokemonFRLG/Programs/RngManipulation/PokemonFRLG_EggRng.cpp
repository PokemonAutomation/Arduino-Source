/*  Egg RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <algorithm>
#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Language.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/PokemonFRLG_DaycareManDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_RngLoopRoutines.h"
#include "PokemonFRLG_EncountersDatabase.h"
#include "PokemonFRLG_EggRng.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ParentIVsRow::ParentIVsRow(int index, std::string&& label)
    : StaticTableRow(label)
    , parent(LockMode::LOCK_WHILE_RUNNING, label)
    , hp(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , atk(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , def(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , spa(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , spd(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , spe(LockMode::LOCK_WHILE_RUNNING, 31, 0, 31)
    , index(index)
{
    PA_ADD_OPTION(parent);
    PA_ADD_OPTION(hp);
    PA_ADD_OPTION(atk);
    PA_ADD_OPTION(def);
    PA_ADD_OPTION(spa);
    PA_ADD_OPTION(spd);
    PA_ADD_OPTION(spe);
}

ParentIVsTable::ParentIVsTable()
    : StaticTableOption("<b>Parent IVs</b>.", LockMode::LOCK_WHILE_RUNNING)
{
    add_row(std::make_unique<ParentIVsRow>(0, "Parent A"));
    add_row(std::make_unique<ParentIVsRow>(1, "Parent B"));
    finish_construction();
}
std::vector<std::string> ParentIVsTable::make_header() const{
    std::vector<std::string> ret{
        "Parent",
        "HP",
        "Attack",
        "Defense",
        "Sp.Attack",
        "Sp.Defense",
        "Speed"
    };
    return ret;
}


EggRng_Descriptor::EggRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:EggRng",
        Pokemon::STRING_POKEMON + " FRLG", "Egg RNG",
        "Programs/PokemonFRLG/EggRng.html",
        "Automatically calibrate timings to hit a specific RNG target for daycare eggs.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct EggRng_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , heldframes(m_stats["Held Frames Checked"])
        , pickupframes(m_stats["Pickup Frames Checked"])
        , shinies(m_stats["Shinies"])
        , nonshiny(m_stats["Non-Shiny Hits"])
        , balls(m_stats["Balls Used"])
        , candies(m_stats["Rare Candies Used"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Held Frames Checked");
        m_display_order.emplace_back("Pickup Frames Checked");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Non-Shiny Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Balls Used", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Rare Candies Used", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& heldframes;
    std::atomic<uint64_t>& pickupframes;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& nonshiny;
    std::atomic<uint64_t>& balls;
    std::atomic<uint64_t>& candies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> EggRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

EggRng::EggRng()
    : m_calibration_displays("Calibration Displays")
    , HELD_CALIBRATION("Held Frame Calibration")
    , PICKUP_CALIBRATION("Pickup Frame Calibration")
    , m_game_info("Game Information")
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
    , GAME_VERSION(
        "<b>Game Version:</b>",
        {
            {GameVersion::firered, "firered", "FireRed"},
            {GameVersion::leafgreen, "leafgreen", "LeafGreen"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        GameVersion::firered
        )
    , m_target_settings("Target Settings")
    , EGG_SPECIES(
        "<b>Egg Species:</b>",
        "PokemonFRLG/EggSpeciesGen3.json"
        )
    , COMPATIBILITY(
        "<b>Parent Compatiblity:</b>",
        {
            {AdvEggCompatibility::low, "low", "The two don't seem to like each other"},
            {AdvEggCompatibility::medium, "medium", "The two seem to get along"},
            {AdvEggCompatibility::high, "high", "The two seem to get along very well"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        AdvEggCompatibility::low
        )
    , m_held_settings("Held Frame Settings")
    , HELD_SEED(
        false,
        "<b>Held Seed:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "70FE", "70FE",
        true
        )
    , HELD_SEED_LIST(
        "<b>Nearby Held Seeds:</b><br>"
        "This box should contain a list of seeds (in order) around and including your held seed, with one seed on each line",
        LockMode::LOCK_WHILE_RUNNING,
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        true
        )
    , HELD_SEED_BUTTON(
        "<b>Held Seed Button:</b><br>",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
        )
    , HELD_EXTRA_BUTTON(
        "<b>Held Seed Extra Button:</b><br>"
        "Additional button presses that affect the seed.",
        {
            {BlackoutButton::None, "None", "None"},
            {BlackoutButton::L, "L", "Blackout L"},
            {BlackoutButton::R, "R", "Blackout R"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BlackoutButton::None
        )
    , HELD_SEED_DELAY(
        "<b>Held Seed Delay Time (ms):</b><br>"
        "The delay between starting the game and advancing past the title screen. Set this to match your held seed.<br>"
        "<i>If using Ten Lines, select <b>Nintendo Switch 1</b> as your console even if using a Switch 2.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 30400 // default, min
        )
    , HELD_ADVANCES(
        "<b>Held Advances:</b><br>The total number of RNG advances for your held target.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 700, 1000000000 // default, min
        )
    , m_pickup_settings("Pickup Frame Settings")
    , PICKUP_SEED(
        false,
        "<b>Pickup Seed:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "70FE", "70FE",
        true
        )
    , PICKUP_SEED_LIST(
        "<b>Nearby Pickup Seeds:</b><br>"
        "This box should contain a list of seeds (in order) around and including your held seed, with one seed on each line",
        LockMode::LOCK_WHILE_RUNNING,
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        "D000\n199A\n77A1\nAABC\n280C\n70FE\nB573\n02F2\n8084\nA533\nED1E",
        true
        )
    , PICKUP_SEED_BUTTON(
        "<b>Pickup Seed Button:</b><br>",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
        )
    , PICKUP_EXTRA_BUTTON(
        "<b>Pickup Seed Extra Button:</b><br>"
        "Additional button presses that affect the seed.",
        {
            {BlackoutButton::None, "None", "None"},
            {BlackoutButton::L, "L", "Blackout L"},
            {BlackoutButton::R, "R", "Blackout R"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BlackoutButton::None
        )
    , PICKUP_SEED_DELAY(
        "<b>Pickup Seed Delay Time (ms):</b><br>"
        "The delay between starting the game and advancing past the title screen. Set this to match your held seed.<br>"
        "<i>If using Ten Lines, select <b>Nintendo Switch 1</b> as your console even if using a Switch 2.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        31338, 30400 // default, min
        )
    , PICKUP_ADVANCES(
        "<b>Pickup Advances:</b><br>The total number of RNG advances for your pickup target.",
        LockMode::LOCK_WHILE_RUNNING,
        10000, 700, 1000000000 // default, min
        )
    , m_program_settings("Program Settings")
    , STARTING_POINT(
        "<b>Starting Point:</b>",
        {
            {EggProgramState::held_prep, "held_prep", "Parents Dropped Off"},
            {EggProgramState::held_calibration, "held_calibration", "Held Frame Calibration"},
            {EggProgramState::pickup_calibration, "pickup_frame", "Pickup Frame Calibration"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        EggProgramState::held_prep
        )
    , MAX_RESETS(
        "<b>Max Resets:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        400, 0 // default, min
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
        20, 2, 999 // default, min, max
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
    PA_ADD_OPTION(HELD_CALIBRATION);
    PA_ADD_OPTION(PICKUP_CALIBRATION);
    PA_ADD_OPTION(m_game_info);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(GAME_VERSION);
    PA_ADD_OPTION(m_target_settings);
    PA_ADD_OPTION(EGG_SPECIES);
    PA_ADD_OPTION(COMPATIBILITY);
    PA_ADD_OPTION(PARENT_IVS);
    PA_ADD_OPTION(m_held_settings);
    PA_ADD_OPTION(HELD_SEED);
    PA_ADD_OPTION(HELD_SEED_LIST);
    PA_ADD_OPTION(HELD_SEED_BUTTON);
    PA_ADD_OPTION(HELD_EXTRA_BUTTON);
    PA_ADD_OPTION(HELD_SEED_DELAY);
    PA_ADD_OPTION(HELD_ADVANCES);
    PA_ADD_OPTION(m_pickup_settings);
    PA_ADD_OPTION(PICKUP_SEED);
    PA_ADD_OPTION(PICKUP_SEED_LIST);
    PA_ADD_OPTION(PICKUP_SEED_BUTTON);
    PA_ADD_OPTION(PICKUP_EXTRA_BUTTON);
    PA_ADD_OPTION(PICKUP_SEED_DELAY);
    PA_ADD_OPTION(PICKUP_ADVANCES);
    PA_ADD_OPTION(m_program_settings);
    PA_ADD_OPTION(STARTING_POINT);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(MAX_BALL_THROWS);
    PA_ADD_OPTION(MAX_RARE_CANDIES);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    STARTING_POINT.add_listener(*this);
}

void EggRng::on_config_value_changed(void* object){
    // switch between separate calibrations for the held and pickup frames
    if (STARTING_POINT.current_value() == 2){
        HELD_CALIBRATION.set_visibility(ConfigOptionState::HIDDEN);
        PICKUP_CALIBRATION.set_visibility(ConfigOptionState::ENABLED);
    }else{
        HELD_CALIBRATION.set_visibility(ConfigOptionState::ENABLED);
        PICKUP_CALIBRATION.set_visibility(ConfigOptionState::HIDDEN);
    }
}




void EggRng::prep_held_resets(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    use_max_repel(env.console, context);
    daycare_steps(env.console, context);
    save_game_to_overworld(env.console, context);
    pbf_wait(context, 5000ms);
    context.wait_for_all_requests();
}

// void EggRng::prep_pickup_resets(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

// }

bool EggRng::reset_and_check_seed(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    EggRng_Descriptor::Stats& stats,
    EggCalibrationHistories& hist,
    const EggFrameTargets& frame_targets,
    AdvRngWildSearcher& wild_searcher,
    WallClock& timestamp,
    uint16_t& current_seed,
    uint64_t& balls_left,
    uint64_t& candies_left,
    uint16_t& failed_searches,
    uint16_t& times_not_held,
    bool& shiny_found,
    const bool& previously_hit_held_frame,
    Milliseconds& launch_delay,
    const std::set<std::string>& SPECIES_LIST,
    const RngStatsDatabase& STATS_DATA,
    bool pickup_frame
){
    RngCalibrations& calibrations = hist.calibrations;
    RngUncertainHistory& wild_uncertain_history = hist.wild_uncertain;
    RngCalibrationHistory& wild_history = hist.wild;

    const EggFrameTarget& frame_target = frame_targets.frame(pickup_frame);
    const uint64_t& SEED_DELAY = frame_target.seed_delay;
    const std::vector<uint16_t>& SEED_VALUES = frame_target.seed_values;
    const int16_t& SEED_POSITION = frame_target.seed_position;

    static const int64_t FIXED_SEED_OFFSET = -845; // ms, approximate
    const int64_t FIXED_ADVANCES_OFFSET = pickup_frame ? -246 : 135;    // frames, approximate

    static const uint64_t CONTINUE_SCREEN_FRAMES = 200;

    static const uint64_t WILD_ADVANCES_RADIUS = 4096;
    
    static const uint16_t MAX_HISTORY_LENGTH = 2;

    send_program_status_notification(
        env, NOTIFICATION_STATUS_UPDATE,
        pickup_frame ? "Calibrating pickup frame" : "Calibrating held frame"
    );
    env.update_stats();

    uint64_t advances = pickup_frame ? PICKUP_ADVANCES : HELD_ADVANCES;
    PokemonFRLG_RngTarget target = pickup_frame ? PokemonFRLG_RngTarget::eggpickup : PokemonFRLG_RngTarget::eggheld;  

    select_calibrations_for_frame(
        env, hist, SEED_VALUES, SEED_POSITION, advances, pickup_frame, previously_hit_held_frame, times_not_held
    );

    uint64_t ingame_advances = advances - CONTINUE_SCREEN_FRAMES;

    RngTimings timings = prepare_timings(
        env.console, target,
        SEED_DELAY, CONTINUE_SCREEN_FRAMES, ingame_advances,
        false, calibrations,
        FIXED_SEED_OFFSET, FIXED_ADVANCES_OFFSET
    );

    // Step 1: reset and perform blind sequence
    env.log("Resetting Game...");
    reset_and_perform_blind_sequence(
        env.console, context, target, 
        pickup_frame ? PICKUP_SEED_BUTTON : HELD_SEED_BUTTON, 
        pickup_frame ? PICKUP_EXTRA_BUTTON : HELD_EXTRA_BUTTON,
        timings, launch_delay, false, PROFILE
    );
    stats.resets++; 

    RNG_FILTERS.reset();
    if (pickup_frame){
        PICKUP_CALIBRATION.set_calibrations(calibrations);
    }else{
        HELD_CALIBRATION.set_calibrations(calibrations);
    }

    // grab a timestamp after the blind button presses are over
    // this is used for estimating what advances wild encounters and test pickups occur on
    timestamp = current_time(); //

    // Step 2: walk to the pond
    if (pickup_frame){
        walk_from_daycare_man_to_pond(env.console, context);
    }else{
        leave_pokecenter(env.console, context);

        DaycareManWatcher egg_ready(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context,
            std::chrono::milliseconds(5000),
            { egg_ready }
        );

        if (ret < 0){
            env.log("Egg not ready. Resetting...");
            times_not_held++;
            return false;
        }
        env.log("Egg ready. Checking seed...");
        times_not_held = 0;

        walk_from_daycare_to_pond(env.console, context);
    }

    // Step 3: Catch a wild encounter and use it to determine the current seed
    AdvObservedPokemon pokemon;
    RngStats species_stats;
    AdvRngFilters filters;
    uint64_t wild_advances_estimate;
    WildCatchOutcome catch_outcome = catch_wild_for_seed_id(
        env, context, stats.errors, NOTIFICATION_ERROR_RECOVERABLE, LANGUAGE,
        timestamp, advances, pickup_frame ? 2 : 0,
        balls_left, SPECIES_LIST, STATS_DATA,
        pokemon, species_stats, filters, wild_advances_estimate,
        [&](const AdvRngFilters& f){ RNG_FILTERS.set(f); }
    );
    shiny_found = (catch_outcome == WildCatchOutcome::shiny);
    if (shiny_found){
        return true;
    }
    if (catch_outcome == WildCatchOutcome::failed){
        return false;
    }

    // For the wild step we only need to confirm the correct seed was hit; advance timing is
    // irrelevant and wild_history is largely unused, so result_determined just checks same_seeds.
    std::vector<AdvRngState> search_hits = refine_calibration_with_rare_candy(
        env, context, LANGUAGE, pokemon, filters, species_stats.base_stats,
        wild_uncertain_history, wild_history, calibrations,
        MAX_HISTORY_LENGTH, candies_left, AdvRngMethod::Any, false,
        stats.errors, NOTIFICATION_ERROR_RECOVERABLE,
        [&](AdvRngFilters& f){
            return get_wild_search_results(env.console, wild_searcher, f, SEED_VALUES, wild_advances_estimate, WILD_ADVANCES_RADIUS, species_stats.gender_threshold);
        },
        [&](const std::vector<AdvRngState>& h){
            if (pickup_frame){ PICKUP_CALIBRATION.set_hits(h); }else{ HELD_CALIBRATION.set_hits(h); }
        },
        [&](const AdvRngFilters& f){ RNG_FILTERS.set(f); },
        [](const std::vector<AdvRngState>& h){ return same_seeds(h); }
    );
    candies_left -= uint64_t(pokemon.stats.size() - 1);

    if (search_hits.size() == 0){
        env.log("No Wild RNG matches found. Resetting...");
        failed_searches++;
        return false;
    }else{
        env.log("Wild RNG search finished.");
        failed_searches = 0;
    }

    if (search_hits[0].seed != SEED_VALUES[SEED_POSITION]){
        env.log("Wrong seed hit.");
        if (pickup_frame || previously_hit_held_frame){
            return false;
        }
    }else{
        env.log("Hit target seed.");
    }

    current_seed = search_hits[0].seed;
    return true;
}

bool EggRng::held_frame_check(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    EggRng_Descriptor::Stats& stats,
    EggCalibrationHistories& hist,
    AdvRngEggSearcher& egg_searcher,
    const WallClock& timestamp,
    const uint16_t& current_seed,
    uint64_t& candies_left,
    uint16_t& failed_searches,
    bool& shiny_found,
    bool& previously_hit_held_frame,
    const uint16_t& TARGET_HELD_SEED,
    const RngStats& EGG_STATS,
    AdvIVs& PARENT_A,
    AdvIVs& PARENT_B
){
    RngCalibrations& calibrations = hist.calibrations;
    RngUncertainHistory& egg_uncertain_history = hist.egg_uncertain;
    RngCalibrationHistory& held_calibration_history = hist.held;

    static const uint64_t HELD_CHECK_ADVANCES_RADIUS = 8092;

    static const uint16_t MAX_HISTORY_LENGTH = 10;
    static const uint64_t INITIAL_ADVANCES_RADIUS = 128;

    uint64_t advances_radius = 
        previously_hit_held_frame ? 4 
        : get_advances_radius(env.console, held_calibration_history, INITIAL_ADVANCES_RADIUS);

    pbf_mash_button(context, BUTTON_B, 5000ms); // return to overworld
    walk_from_pond_to_daycare_man(env.console, context);

    // decide if it's a good idea to save and commit to this held frame
    bool locked_in = false;
    if (previously_hit_held_frame && (current_seed == TARGET_HELD_SEED)){
        env.log("Committing to this Held Frame and saving the game...");
        save_game_to_overworld(env.console, context);
        locked_in = true;
    }

    // collect and hatch egg
    egg_pickup(env.console, context);
    WallDuration elapsed = current_time() - timestamp;
    pbf_press_button(context, BUTTON_MINUS, 200ms, 800ms); // hop on bike
    hatch_daycare_egg(env.console, context);
    shiny_found = shiny_check_summary(env.console, context);
    if (shiny_found){
        return true;
    }

    // read stats
    AdvObservedPokemon observed_egg = read_summary(env.console, context, LANGUAGE);
    AdvRngFilters filters = observation_to_filters(observed_egg, EGG_STATS.base_stats, AdvRngMethod::Any);
    RNG_FILTERS.set(filters);

    // narrow down possible hits 
    auto elapsed_ms = std::chrono::duration_cast<Milliseconds>(elapsed);
    uint64_t advances_estimate = uint64_t(HELD_ADVANCES + 2 * elapsed_ms.count() / FRLG_FRAME_DURATION);
    env.log("Test pickup advances estimate: " + std::to_string(advances_estimate));    

    std::vector<AdvRngState> held_hits = refine_egg_calibration(
        env, context, LANGUAGE, observed_egg, filters, EGG_STATS.base_stats,
        egg_uncertain_history, held_calibration_history, calibrations,
        MAX_HISTORY_LENGTH, candies_left,
        EggRefineOptions{ true, 1, 0 },
        stats.errors, NOTIFICATION_ERROR_RECOVERABLE,
        [&](AdvRngFilters& f){
            return get_egg_search_results(
                env.console, egg_searcher, f,
                { current_seed }, { current_seed },
                HELD_ADVANCES, advances_radius, advances_estimate, HELD_CHECK_ADVANCES_RADIUS,
                PARENT_A, PARENT_B, COMPATIBILITY,
                EGG_STATS.gender_threshold, 0
            );
        },
        [&](const std::vector<AdvRngState>& h){ HELD_CALIBRATION.set_hits(h); },
        [&](const AdvRngFilters& f){ RNG_FILTERS.set(f); },
        [&](const std::vector<std::pair<AdvRngState, AdvRngState>>& pairs){
            return all_indistinguishable(pairs, egg_searcher, EGG_STATS.gender_threshold, PARENT_A, PARENT_B);
        }
    );

    candies_left -= uint64_t(observed_egg.stats.size() - 1);

    env.log("RNG Search finished");
    update_failed_searches(failed_searches, held_hits);

    bool possibly_hit_held_frame = false;
    if (egg_uncertain_history.results.size()){
        for (const AdvRngState& hit : held_hits){
            if (have_hit_target(TARGET_HELD_SEED, HELD_ADVANCES, hit)){
                possibly_hit_held_frame = true;
                break;
            }
        }
    }

    if (held_calibration_history.results.empty()){
        return false;
    }

    bool definitely_hit_held_frame = confirm_held_frame_hit(
        held_calibration_history, egg_uncertain_history, locked_in, TARGET_HELD_SEED, HELD_ADVANCES
    );

    if (locked_in && definitely_hit_held_frame){
        env.log("Hit the target held frame!");
    }else if (!locked_in && definitely_hit_held_frame){
        env.log("Confirmed hit on the target held frame!");
    }else if(possibly_hit_held_frame){
        env.log("Possibly hit target held frame.");
    }else{
        env.log("Missed the target held frame.");
    }

    if (locked_in && !(definitely_hit_held_frame || possibly_hit_held_frame)){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EggRng(): Missed held frame after saving. Restart the program after repeating the manual in-game setup.",
            env.console
        ); 
    }

    previously_hit_held_frame = definitely_hit_held_frame;

    return (locked_in && (definitely_hit_held_frame || possibly_hit_held_frame));
}


bool EggRng::pickup_frame_check(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    EggRng_Descriptor::Stats& stats,
    EggCalibrationHistories& hist,
    AdvRngEggSearcher& egg_searcher,
    uint64_t& candies_left,
    uint16_t& failed_searches,
    bool& shiny_found,
    const uint16_t& TARGET_HELD_SEED,
    const uint16_t& TARGET_PICKUP_SEED,
    const RngStats& EGG_STATS,
    AdvIVs& PARENT_A,
    AdvIVs& PARENT_B
){
    RngCalibrations& calibrations = hist.calibrations;
    RngUncertainHistory& egg_uncertain_history = hist.egg_uncertain;
    RngCalibrationHistory& pickup_calibration_history = hist.pickup;

    static const uint16_t MAX_HISTORY_LENGTH = 5;
    static const uint64_t INITIAL_ADVANCES_RADIUS = 128;

    uint64_t advances_radius = get_advances_radius(env.console, pickup_calibration_history, INITIAL_ADVANCES_RADIUS);

    pbf_mash_button(context, BUTTON_B, 5000ms); // return to overworld
    walk_from_pond_to_daycare_man(env.console, context);

    // egg already collected. Hatch...
    pbf_press_button(context, BUTTON_MINUS, 200ms, 800ms); // hop on bike
    hatch_daycare_egg(env.console, context);
    shiny_found = shiny_check_summary(env.console, context, 1);
    if (shiny_found){
        return true;
    }

    // ... read ...
    AdvObservedPokemon observed_egg = read_summary(env.console, context, LANGUAGE);
    AdvRngFilters filters = observation_to_filters(observed_egg, EGG_STATS.base_stats, AdvRngMethod::Any);
    RNG_FILTERS.set(filters);

    // ...and narrow down possible hits
    std::vector<AdvRngState> pickup_hits = refine_egg_calibration(
        env, context, LANGUAGE, observed_egg, filters, EGG_STATS.base_stats,
        egg_uncertain_history, pickup_calibration_history, calibrations,
        MAX_HISTORY_LENGTH, candies_left,
        EggRefineOptions{ false, 2, 1 },
        stats.errors, NOTIFICATION_ERROR_RECOVERABLE,
        [&](AdvRngFilters& f){
            return get_egg_search_results(
                env.console, egg_searcher, f,
                { TARGET_HELD_SEED }, { TARGET_PICKUP_SEED },
                HELD_ADVANCES, 0, PICKUP_ADVANCES, advances_radius,
                PARENT_A, PARENT_B, COMPATIBILITY,
                EGG_STATS.gender_threshold, 0
            );
        },
        [&](const std::vector<AdvRngState>& h){ PICKUP_CALIBRATION.set_hits(h); },
        [&](const AdvRngFilters& f){ RNG_FILTERS.set(f); },
        [&](const std::vector<std::pair<AdvRngState, AdvRngState>>& pairs){
            return all_indistinguishable(pairs, egg_searcher, EGG_STATS.gender_threshold, PARENT_A, PARENT_B);
        }
    );
    candies_left -= uint64_t(observed_egg.stats.size() - 1);

    env.log("RNG Search finished");
    update_failed_searches(failed_searches, pickup_hits);

    if (pickup_calibration_history.results.empty()){
        return false;
    }

    auto newest_result = pickup_calibration_history.results.back();
    return (
        newest_result.seed == TARGET_PICKUP_SEED
        && newest_result.advance == PICKUP_ADVANCES
    );
    
}


void EggRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    EggRng_Descriptor::Stats& stats = env.current_stats<EggRng_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    // prepare database of base stats and gender thresholds
    RngStatsDatabase STATS_DATA("PokemonFRLG/BaseStats.json");

    RngStats EGG_STATS;
    try{
        EGG_STATS = STATS_DATA.get_throw(EGG_SPECIES.slug());        
    }catch (const InternalProgramError& err){
        env.log(err.message());
        env.log(EGG_SPECIES.slug());
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            err.message(),
            env.console
        ); 
    }

    // get the relevant encounter slots
    EncountersDatabase encounters_data(GAME_VERSION == GameVersion::firered ? "PokemonFRLG/EncounterSlotsFR.json" : "PokemonFRLG/EncounterSlotsLG.json");
    std::map<std::string, std::vector<AdvEncounterSlot>> location_map = encounters_data.get_throw("surfing");
    std::vector<AdvEncounterSlot> ENCOUNTER_SLOTS = location_map.find("four_island")->second;
    env.log("Pond Encounter slots");
    for (size_t i=0; i<ENCOUNTER_SLOTS.size(); i++){
        AdvEncounterSlot& slot = ENCOUNTER_SLOTS[i];
        env.log("   Slot " + std::to_string(i) + ": " + slot.species + " " + std::to_string(slot.minlevel) + "-" + std::to_string(slot.maxlevel));
    }    

    std::set<std::string> SPECIES_LIST;
    for (auto slot : ENCOUNTER_SLOTS){
        SPECIES_LIST.emplace(slot.species);
    }

    // held timings
    const uint16_t TARGET_HELD_SEED = parse_seed(env.console, HELD_SEED);
    const std::vector<uint16_t> HELD_SEED_VALUES = parse_seed_list(env.console, HELD_SEED_LIST);
    const int16_t HELD_SEED_POSITION = seed_position_in_list(TARGET_HELD_SEED, HELD_SEED_VALUES);
    if (HELD_SEED_POSITION == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EggRng(): Held Seed is missing from the list of nearby seeds.",
            env.console
        ); 
    }
    env.log("Target Held Seed Value: " + to_hex_string(TARGET_HELD_SEED));

    // pickup timings
    const uint16_t TARGET_PICKUP_SEED = parse_seed(env.console, PICKUP_SEED);
    const std::vector<uint16_t> PICKUP_SEED_VALUES = parse_seed_list(env.console, PICKUP_SEED_LIST);
    const int16_t PICKUP_SEED_POSITION = seed_position_in_list(TARGET_PICKUP_SEED, PICKUP_SEED_VALUES);
    if (PICKUP_SEED_POSITION == -1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EggRng(): Pickup Seed is missing from the list of nearby seeds.",
            env.console
        ); 
    }
    env.log("Target Pickup Seed Value: " + to_hex_string(TARGET_PICKUP_SEED));

    EggFrameTargets frame_targets{
        { HELD_SEED_DELAY, HELD_SEED_VALUES, HELD_SEED_POSITION },
        { PICKUP_SEED_DELAY, PICKUP_SEED_VALUES, PICKUP_SEED_POSITION }
    };

    // searchers
    AdvRngWildSearcher wild_searcher(TARGET_HELD_SEED, HELD_ADVANCES, ENCOUNTER_SLOTS, AdvRngMethod::Any);
    AdvRngEggSearcher egg_searcher(TARGET_HELD_SEED, HELD_ADVANCES, TARGET_PICKUP_SEED, PICKUP_ADVANCES, AdvRngMethod::Any);

    ParentIVsRow& rowA = static_cast<ParentIVsRow&>(*PARENT_IVS.table()[0]);
    ParentIVsRow& rowB = static_cast<ParentIVsRow&>(*PARENT_IVS.table()[1]);
    AdvIVs PARENT_A = { rowA.hp, rowA.atk, rowA.def, rowA.spa, rowA.spd, rowA.spe };
    AdvIVs PARENT_B = { rowB.hp, rowB.atk, rowB.def, rowB.spa, rowB.spd, rowB.spe };

    AdvPokemonResult target_result = egg_searcher.generate_pokemon(PARENT_A, PARENT_B);
    RNG_TARGET.set_target(target_result, EGG_STATS.gender_threshold);
    log_target_pokemon(env.console, target_result, true);

    Milliseconds launch_delay = INITIAL_LAUNCH_DELAY;

    const RngCalibrations initial_held_calibrations = {
        HELD_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
        HELD_CALIBRATION.csf_calibration,
        HELD_CALIBRATION.advances_calibration
    };

    EggCalibrationHistories hist(initial_held_calibrations);
    if (STARTING_POINT == EggProgramState::pickup_calibration){
        hist.calibrations = {
            PICKUP_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
            PICKUP_CALIBRATION.csf_calibration,
            PICKUP_CALIBRATION.advances_calibration
        };
    }else{
        hist.calibrations = initial_held_calibrations;
    }

    log_calibrations(env.console, hist.calibrations, true);

    bool shiny_found = false;

    bool previously_hit_held_frame = false;

    uint64_t balls_left = MAX_BALL_THROWS;
    uint64_t candies_left = MAX_RARE_CANDIES;
    uint16_t failed_searches = 0;
    uint16_t times_not_held = 0;

    EggProgramState program_state = STARTING_POINT;

    
    // main loop
    while (true){
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

        if (hist.pickup.results.size() > 0){
            env.log("Checking for nonshiny target hit...");
            if (have_hit_target(TARGET_HELD_SEED, HELD_ADVANCES, hist.pickup.results.back())){
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

        bool hit_seed;
        bool held_finished;
        bool pickup_finished;

        WallClock timestamp; 

        uint16_t current_seed;

        uint64_t temp_balls_left;
        uint64_t temp_candies_left;

        switch (program_state){
        case EggProgramState::held_prep:
            prep_held_resets(env, context);
            hist.clear_uncertain();
            program_state = EggProgramState::held_calibration;
            STARTING_POINT.set(EggProgramState::held_calibration);
            continue;

        case EggProgramState::held_calibration:
            current_seed = 0;
            temp_balls_left = balls_left;
            temp_candies_left = candies_left;
            hit_seed = reset_and_check_seed(
                env, context, stats,
                hist, frame_targets, wild_searcher,
                timestamp, current_seed,
                temp_balls_left, temp_candies_left, failed_searches,
                times_not_held, shiny_found, previously_hit_held_frame,
                launch_delay,
                SPECIES_LIST, STATS_DATA,
                false
            );

            if (!hit_seed) { continue; }

            held_finished = held_frame_check(
                env, context, stats,
                hist, egg_searcher,
                timestamp, current_seed,
                temp_candies_left, failed_searches,
                shiny_found, previously_hit_held_frame,
                TARGET_HELD_SEED,
                EGG_STATS, PARENT_A, PARENT_B
            );
            stats.heldframes++;

            if (held_finished){
                env.log("Hit held frame after saving. Moving on to pickup frame calibration...");
                stats.balls += (balls_left - temp_balls_left);
                stats.candies += (candies_left - temp_candies_left);
                balls_left = temp_balls_left;
                candies_left = temp_candies_left;
                hist.clear_uncertain();
                program_state = EggProgramState::pickup_calibration;
                STARTING_POINT.set(EggProgramState::pickup_calibration);
                hist.calibrations = {
                    PICKUP_CALIBRATION.seed_calibration / FRLG_FRAME_DURATION,
                    PICKUP_CALIBRATION.csf_calibration,
                    PICKUP_CALIBRATION.advances_calibration
                };
            }
            continue;

        case EggProgramState::pickup_calibration:
            current_seed = 0; // unused here
            temp_balls_left = balls_left;
            temp_candies_left = candies_left;
            hit_seed = reset_and_check_seed(
                env, context, stats,
                hist, frame_targets, wild_searcher,
                timestamp, current_seed,
                temp_balls_left, temp_candies_left, failed_searches,
                times_not_held, shiny_found, previously_hit_held_frame,
                launch_delay,
                SPECIES_LIST, STATS_DATA,
                true
            );

            if (!hit_seed) { continue; }

            pickup_finished = pickup_frame_check(
                env, context, stats,
                hist, egg_searcher,
                temp_candies_left, failed_searches, shiny_found,
                TARGET_HELD_SEED, TARGET_PICKUP_SEED,
                EGG_STATS, PARENT_A, PARENT_B
            );
            stats.pickupframes++;

            if (pickup_finished){
                stats.balls += (balls_left - temp_balls_left);
                stats.candies += (candies_left - temp_candies_left);
                env.log("Hit pickup frame!");
                program_state = EggProgramState::finished;
                STARTING_POINT.set(EggProgramState::held_prep);
            }
            continue;

        case EggProgramState::finished:
        default:
            if (GO_HOME_WHEN_DONE){
                pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
            }
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
        }
    }

    // just in case
    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}



}
}
}
