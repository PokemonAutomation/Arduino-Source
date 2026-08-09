/*  BDSP Starter RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <cmath>
#include <memory>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_SummaryReader.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP_BlinkRecovery.h"
#include "PokemonBDSP_RngCalibration.h"
#include "PokemonBDSP_RngExecution.h"
#include "PokemonBDSP_SummaryNavigation.h"
#include "PokemonBDSP_RngTargets.h"
#include "PokemonBDSP_RngTimeline.h"
#include "PokemonBDSP_StarterRng.h"
#include "PokemonBDSP_TargetSelection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


const size_t LAKE_BLINKS_PRESS_RETRIES = 10;

const uint16_t RECOVERY_TIMEOUT_SECONDS = 600;

const double GAP_GUARD_SECONDS = 0.30;

const uint64_t HIT_SEARCH_RADIUS = 20;
const size_t MAX_SCHEDULES = 8;

const uint64_t SKIP_ARRIVAL_OVERSHOOT_ALLOWANCE = 200000;


const size_t CALIBRATION_MIN_SAMPLES = 3;
const double CALIBRATION_THRESHOLD = 1.0;


StarterRng_Descriptor::StarterRng_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:StarterRng",
        STRING_POKEMON + " BDSP", "Starter RNG",
        "",
        "Starter RNG manipulation using blinks.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct StarterRng_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , missed(m_stats["Missed"])
        , hits(m_stats["Hit"])
        , shinies(m_stats["Shiny"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back(Stat("Missed", HIDDEN_IF_ZERO));
        m_display_order.emplace_back(Stat("Hit", HIDDEN_IF_ZERO));
        m_display_order.emplace_back(Stat("Shiny", HIDDEN_IF_ZERO));
        m_display_order.emplace_back(Stat("Errors", HIDDEN_IF_ZERO));
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& missed;
    std::atomic<uint64_t>& hits;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> StarterRng_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


StarterRng::StarterRng()
    : m_aim(CALIBRATION_MIN_SAMPLES, CALIBRATION_THRESHOLD)
    , LANGUAGE(
        "<b>Game Language:</b>",
        summary_nature_languages(),
        LockMode::LOCK_WHILE_RUNNING, true
    )
    , START_POINT(
        "<b>Start point:</b><br>"
        "Where you have saved your game. Starting from the bedroom allows the possibility to skip advances quickly.",
        {
            {BdspStartPoint::Lakefront, "lakefront", "Verity Lakefront"},
            {BdspStartPoint::Bedroom,   "bedroom",   "Bedroom"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BdspStartPoint::Lakefront
    )
    , STARTER(
        "<b>Starter:</b><br>",
        {
            {BdspStarter::Turtwig,  "turtwig",  "Turtwig"},
            {BdspStarter::Chimchar, "chimchar", "Chimchar"},
            {BdspStarter::Piplup,   "piplup",   "Piplup"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BdspStarter::Turtwig
    )
    , FILTERS(
        "<b>Stop Conditions:</b><br>"
        "If the " + STRING_POKEMON + " matches any one of these filters, the program will "
        "stop.<br>",
        StatsHuntMiscFeatureFlags{
            /*action*/ false, /*shiny*/ true, /*gender*/ true, /*nature*/ true
        }
    )
    , COLLECTION_DISPLAY(true)
    , MAX_RESETS(
        "<b>Maximum resets:</b><br>"
        "Set this to zero to keep going until a shiny is caught.",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 1000
    )
    , MAX_TARGET_WAIT_MINUTES(
        "<b>Wait at most (minutes) for a target:</b><br>"
        "Time limit for waiting at Verity Lake. Rarer targets will require longer "
        "wait times.",
        LockMode::LOCK_WHILE_RUNNING,
        30, 5, 1440
    )
    , MAX_SKIP_MINUTES(
        "<b>Skip at most (minutes):</b><br>"
        "Time limit for skipping advances in Twinleaf Town.",
        LockMode::LOCK_WHILE_RUNNING,
        30, 5, 1440
    )
    , AUTO_CALIBRATE(
        "<b>Correct the timing automatically:</b><br>"
        "Shift the target advances when several attempts miss.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , USE_SOUND_DETECTION(
        "<b>Use sound detection:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>"
        "Record a video when the shiny is found.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_SHINY(
        "Shiny Starter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    {
        std::vector<std::unique_ptr<EditableTableRow>> defaults;
        auto row = std::make_unique<BdspRngFilterRow>(FILTERS);
        row->misc.shiny.set(StatsHuntShinyFilter::Shiny);
        defaults.emplace_back(std::move(row));
        FILTERS.set_default(std::move(defaults));
        FILTERS.restore_defaults();
    }

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(PLAYER_MODEL);
    PA_ADD_OPTION(START_POINT);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(FILTERS);
    PA_ADD_OPTION(COLLECTION_DISPLAY);
    PA_ADD_OPTION(STATE_DISPLAY);
    PA_ADD_OPTION(TARGET_DISPLAY);
    PA_ADD_OPTION(MAX_RESETS);
    PA_ADD_OPTION(MAX_TARGET_WAIT_MINUTES);
    PA_ADD_OPTION(MAX_SKIP_MINUTES);
    PA_ADD_OPTION(AUTO_CALIBRATE);
    PA_ADD_OPTION(USE_SOUND_DETECTION);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    START_POINT.add_listener(*this);
    StarterRng::on_config_value_changed(this);
}


void StarterRng::on_config_value_changed(void*){
    MAX_SKIP_MINUTES.set_visibility(
        START_POINT == BdspStartPoint::Bedroom
            ? ConfigOptionState::ENABLED
            : ConfigOptionState::HIDDEN
    );
}


bool StarterRng::wanted(
    const BdspRngFilterSnapshot& filters, const BdspPokemonResult& pokemon
){
    auto exactly = [](uint8_t iv){ return IvRange{(int8_t)iv, (int8_t)iv}; };
    IvRanges ivs;
    ivs.hp      = exactly(pokemon.ivs.hp);
    ivs.attack  = exactly(pokemon.ivs.attack);
    ivs.defense = exactly(pokemon.ivs.defense);
    ivs.spatk   = exactly(pokemon.ivs.spatk);
    ivs.spdef   = exactly(pokemon.ivs.spdef);
    ivs.speed   = exactly(pokemon.ivs.speed);

    StatsHuntGenderFilter gender = StatsHuntGenderFilter::Genderless;
    switch (pokemon.gender){
    case BdspGender::Male:   gender = StatsHuntGenderFilter::Male;   break;
    case BdspGender::Female: gender = StatsHuntGenderFilter::Female; break;
    default: break;
    }

    return filters.get_action(
        pokemon.shiny != BdspShiny::None,
        gender,
        bdsp_nature_to_checker_value(pokemon.nature),
        pokemon.height,
        ivs
    ) != StatsHuntAction::Discard;
}


BdspSkipResult StarterRng::skip_advances_from_bedroom(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    BdspSkipResult skip;
    const BdspRngTargetInfo& target_info = bdsp_rng_target_info(BdspRngTarget::Starter);

    //  Expected wait duration at the lake after a long skip
    double longest_skip = MAX_SKIP_MINUTES * BDSP_SKIP_ADVANCES_PER_MINUTE;
    double lake_minutes = bdsp_lake_advances_needed(longest_skip)
        * BDSP_NPC_TICK_SECONDS / (target_info.observation_npcs * 60.0);
    if (lake_minutes > MAX_TARGET_WAIT_MINUTES){
        throw UserSetupError(env.logger(),
            "A skip of up to " + std::to_string((uint16_t)MAX_SKIP_MINUTES)
            + " minutes can leave up to "
            + tostr_fixed(lake_minutes, 1) + " minutes of waiting at the lake, but the target "
            "wait is capped at " + std::to_string((uint16_t)MAX_TARGET_WAIT_MINUTES)
            + ". Raise the target wait to at least "
            + std::to_string((uint16_t)std::ceil(lake_minutes))
            + " minutes, or lower the skip limit."
        );
    }

    env.log("Starting in the bedroom.");
    pbf_press_button(context, BUTTON_A, 100ms, 2000ms); // open dialogue box
    context.wait_for_all_requests();

    std::vector<BdspEyeTemplate> setups = bedroom_eye_templates(PLAYER_MODEL.model_number());
    std::vector<std::shared_ptr<const ImageRGB32>> eyes = load_eye_templates(setups);

    std::vector<std::unique_ptr<EyeBlinkWatcher>> watchers;
    std::vector<PeriodicInferenceCallback> callbacks;
    make_blink_watchers(setups, eyes, watchers, callbacks);

    BlinkRecoveryConfig bedroom_config;
    bedroom_config.npcs = 1;

    BlinkRecovery recovery = recover_state_from_blinks(
        env, context, watchers, callbacks, COLLECTION_DISPLAY, bedroom_config,
        RECOVERY_TIMEOUT_SECONDS
    );
    if (!recovery.success){
        skip.failure_reason = "could not determine the RNG state in the bedroom: "
            + recovery.failure_reason;
        return skip;
    }
    skip.state = recovery.state;
    env.log("Bedroom RNG state: " + recovery.state.to_string(), COLOR_BLUE);
    STATE_DISPLAY.set_state(recovery.state, recovery.clock.anchor_advance);
    STATE_DISPLAY.set_confidence_unique();

    //  take into account remaning navigation time and the desired buffer
    uint64_t soonest = recovery.clock.advance_at(current_time())
        + (uint64_t)BDSP_SKIP_NAVIGATION_ADVANCES + bdsp_skip_buffer(0);
    uint64_t furthest = soonest
        + (uint64_t)(MAX_SKIP_MINUTES * BDSP_SKIP_ADVANCES_PER_MINUTE);

    BdspStaticSearcher searcher(recovery.state, target_info.pokemon, 0);
    BdspRngFilterSnapshot filters = FILTERS.make_snapshot();
    std::vector<BdspRngHit> hits = searcher.scan(
        soonest, furthest,
        [&filters](const BdspPokemonResult& result){ return wanted(filters, result); },
        true, &context
    );
    if (hits.empty()){
        skip.failure_reason = "no target within " + std::to_string((uint16_t)MAX_SKIP_MINUTES)
            + " minutes of skipping";
        return skip;
    }
    const BdspRngHit& hit = hits[0];
    skip.target_advance = hit.advances;
    TARGET_DISPLAY.set_target(hit.result, hit.advances);

    uint64_t start = recovery.clock.advance_at(current_time());
    
    //  The buffer depends on how far the skip runs, so size it from the raw distance.
    double raw_skip = (double)hit.advances - (double)start - BDSP_SKIP_NAVIGATION_ADVANCES;
    uint64_t buffer = bdsp_skip_buffer(raw_skip);
    double to_skip = raw_skip - (double)buffer;
    if (to_skip < 0){
        to_skip = 0;
    }
    double wait_seconds = to_skip * 60 / BDSP_SKIP_ADVANCES_PER_MINUTE;
    skip.buffer = buffer;

    env.log("Target advance " + std::to_string(hit.advances) + ": " + hit.result.to_string(),
        COLOR_BLUE);
    env.log("Skipping " + std::to_string((uint64_t)to_skip) + " advances from "
        + std::to_string(start) + ", which is " + tostr_fixed(wait_seconds, 1)
        + "s on the spot, stopping " + std::to_string(buffer)
        + " advances short to avoid overshooting.", COLOR_BLUE);

    navigate_bedroom_to_skip_spot(env.logger(), context);


    WallClock leave_at = current_time() + std::chrono::duration_cast<WallDuration>(
        std::chrono::duration<double>(wait_seconds)
    );

    while (current_time() + 2s < leave_at){
        WallClock now = current_time();
        WallClock until = std::min(leave_at - 2s, now + 180s);
        pbf_wait(context, std::chrono::duration_cast<Milliseconds>(until - now));
        context.wait_for_all_requests();
    }
    WallClock now = current_time();
    if (now < leave_at){
        pbf_wait(context, std::chrono::duration_cast<Milliseconds>(leave_at - now));
        context.wait_for_all_requests();
    }

    navigate_skip_spot_to_lakefront(env.logger(), context);
    skip.success = true;
    return skip;
}


void StarterRng::report_skip_arrival(
    SingleSwitchProgramEnvironment& env,
    const BdspSkipResult& skip,
    const BlinkRecovery& arrival
) const{
    //  Generous enough to catch an overshoot, and a match is unique anyway.
    uint64_t search_max = skip.target_advance + SKIP_ARRIVAL_OVERSHOOT_ALLOWANCE;
    uint64_t travelled = 0;
    if (!advances_between(skip.state, arrival.state, search_max, travelled)){
        env.log(
            "Could not place the lake state against the bedroom one, so the skip "
            "cannot be measured this attempt.",
            COLOR_ORANGE
        );
        return;
    }

    uint64_t here = travelled + arrival.clock.advance_at(current_time());
    int64_t short_by = (int64_t)skip.target_advance - (int64_t)here;
    env.log(
        "Skip landed on advance " + std::to_string(here) + " of "
        + std::to_string(skip.target_advance) + ": "
        + (short_by >= 0
            ? std::to_string(short_by) + " short, aimed for " + std::to_string(skip.buffer)
            : "OVERSHOT by " + std::to_string(-short_by)),
        short_by >= 0 ? COLOR_BLUE : COLOR_ORANGE
    );
}


BdspAttemptOutcome StarterRng::run_attempt(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    StarterRng_Descriptor::Stats& stats = env.current_stats<StarterRng_Descriptor::Stats>();
    COLLECTION_DISPLAY.reset();
    STATE_DISPLAY.reset();
    TARGET_DISPLAY.reset();

    auto abandon = [&](const std::string& reason) -> BdspAttemptOutcome{
        env.log("Abandoning this attempt: " + reason, COLOR_ORANGE);
        TARGET_DISPLAY.set_note(reason);
        return BdspAttemptOutcome::Abandoned;
    };

    const BdspRngTargetInfo& target_info = bdsp_rng_target_info(BdspRngTarget::Starter);

    BdspSkipResult skip;
    if (START_POINT == BdspStartPoint::Bedroom){
        skip = skip_advances_from_bedroom(env, context);
        if (!skip.success){
            return abandon(skip.failure_reason);
        }
    }

    navigate_to_lake_blinks(env.logger(), context);

    std::vector<BdspEyeTemplate> setups = lake_eye_templates(PLAYER_MODEL.model_number());
    std::vector<std::shared_ptr<const ImageRGB32>> eyes = load_eye_templates(setups);

    //  The rival's dialogue box being up is what says the pair have stopped walking.
    ShortDialogDetector dialog;
    BlinkSceneConfig scene;
    scene.frame_filter = [&dialog](const ImageViewRGB32& frame){ return dialog.detect(frame); };
    scene.press_retries = LAKE_BLINKS_PRESS_RETRIES;

    if (!wait_for_blink_scene(env, context, setups, eyes, scene, "Lake blinks")){
        return abandon("never reached the lake blink position");
    }

    // Now blink watchers can be created safely
    std::vector<std::unique_ptr<EyeBlinkWatcher>> watchers;
    std::vector<PeriodicInferenceCallback> callbacks;
    make_blink_watchers(setups, eyes, watchers, callbacks);

    BlinkRecoveryConfig blink_config;
    blink_config.npcs = target_info.observation_npcs;

    BlinkRecovery recovery = recover_state_from_blinks(
        env, context, watchers, callbacks, COLLECTION_DISPLAY, blink_config,
        RECOVERY_TIMEOUT_SECONDS
    );
    if (!recovery.success){
        return abandon("could not determine the RNG state: " + recovery.failure_reason);
    }

    uint64_t seed0 = 0;
    uint64_t seed1 = 0;
    xorshift128_state_to_seed_pair(recovery.state, seed0, seed1);
    env.log("RNG state: " + recovery.state.to_string(), COLOR_BLUE);
    env.log("PokeFinder seeds: " + tostr_hex_padded(16, seed0)
        + " " + tostr_hex_padded(16, seed1), COLOR_BLUE);
    env.log("Tick fitted at " + tostr_default(recovery.clock.tick_seconds) + "s over "
        + std::to_string(recovery.events) + " rolls.", COLOR_BLUE);
    env.log("Anchored on advance " + std::to_string(recovery.clock.anchor_advance)
        + "; now at advance " + std::to_string(recovery.clock.advance_at(current_time()))
        + ".", COLOR_BLUE);
    STATE_DISPLAY.set_state(recovery.state, recovery.clock.anchor_advance);
    STATE_DISPLAY.set_confidence_unique();

    if (skip.success){
        report_skip_arrival(env, skip, recovery);
    }


    const BdspTimelineContext& timeline = target_info.timeline;

    NavigationTimings timings;
    timings.ready_seconds = seconds_from_briefcase_to_starly_ready();
    timings.move_seconds = seconds_to_move_to_starter(STARTER) + 0.25; // small buffer
    timings.prompt_seconds = seconds_from_select_to_confirm();
    timings.guard_seconds = GAP_GUARD_SECONDS;

    BdspStaticSearcher searcher(recovery.state, target_info.pokemon, 0);

    //  A press cannot be sooner than the walk to the pre-briefcase position takes
    double lead_seconds = seconds_to_pre_briefcase() + 15;
    uint64_t lead_advances = (uint64_t)(
        lead_seconds * recovery.clock.npcs / recovery.clock.tick_seconds
    ) + 1;

    TargetSearchRequest search;
    search.state = recovery.state;
    search.timeline = timeline;
    search.timings = timings;
    search.pokemon = target_info.pokemon;
    search.npcs = recovery.clock.npcs;
    search.tick_seconds = recovery.clock.tick_seconds;
    search.bias = m_aim.bias();
    search.first_press = recovery.clock.advance_at(current_time()) + lead_advances;
    search.window_advances = (uint64_t)(
        (double)MAX_TARGET_WAIT_MINUTES * 60 * recovery.clock.npcs / recovery.clock.tick_seconds
    );
    search.max_schedules = MAX_SCHEDULES;
    BdspRngFilterSnapshot filters = FILTERS.make_snapshot();
    search.wanted = [&filters](const BdspPokemonResult& result){ return wanted(filters, result); };

    TargetSelectionResult selection = select_target(search);

    if (selection.matches_found != 0){
        env.log(
            std::to_string(selection.matches_found) + " matching advance(s) in "
            + std::to_string(selection.advances_scanned),
            COLOR_BLUE
        );
    }
    if (!selection.success){
        return abandon(selection.failure_reason);
    }

    uint64_t press_advance = selection.press_advance;
    uint64_t target_advance = selection.target_advance;
    const PressSchedule& schedule = selection.schedule;
    const BdspPokemonResult& target = selection.target;

    double wait_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(
        recovery.clock.time_of_advance(press_advance) - current_time()
    ).count();
    env.log("Target advance " + std::to_string(target_advance) + ": " + target.to_string(),
        COLOR_BLUE);
    env.log("Pressing at advance " + std::to_string(press_advance) + ", in "
        + tostr_default(wait_seconds) + "s.", COLOR_BLUE);
    env.log("Schedule spans " + std::to_string(schedule.span) + " advances, confirming after "
        + std::to_string(schedule.blinks_before_confirm) + " Starly blink(s), "
        + tostr_default(schedule.confirm_seconds) + "s in"
        + m_aim.describe_correction() + ".", COLOR_BLUE);
    TARGET_DISPLAY.set_target(target, target_advance);

    hold_and_reanchor(
        env, context, watchers, callbacks, recovery, blink_config,
        press_advance, lead_seconds, STATE_DISPLAY
    );

    navigate_to_pre_briefcase(env.logger(), context);

    // FIRST PRESS, centred in the advance rather than aimed right at the beginning of it
    WallClock briefcase_time = recovery.clock.middle_of_advance(press_advance);

    while (current_time() + 3s < briefcase_time){
        STATE_DISPLAY.set_advances(recovery.clock.advance_at(current_time()));
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();
    }

    STATE_DISPLAY.set_advances(press_advance);

    env.log("Timeline starts at advance " + std::to_string(press_advance)
        + ". Blind from here: Starly press at " + tostr_default(schedule.starly_seconds)
        + "s, selecting at " + tostr_default(schedule.select_seconds)
        + "s, confirming at " + tostr_default(schedule.confirm_seconds)
        + "s, on advance " + std::to_string(target_advance - timeline.advances_after_accept)
        + ".", COLOR_BLUE);

    std::string sequence_failure;
    if (!wait_until_moment(context, env.logger(), briefcase_time)){
        return abandon("the briefcase press moment passed before it could be sent");
    }
    if (!issue_starter_sequence(
        context, STARTER,
        schedule.starly_seconds, schedule.select_seconds, schedule.confirm_seconds,
        sequence_failure
    )){
        return abandon(sequence_failure);
    }
    env.log("Timeline finished; the starter should be in hand.", COLOR_BLUE);

    STATE_DISPLAY.set_advances(target_advance);


    //  Check what was actually obtained.
    DoublesShinyDetection wild;
    ShinyDetectionResult own;
    detect_shiny_battle(
        env, env.console, context,
        wild, own,
        NOTIFICATION_ERROR_RECOVERABLE,
        YOUR_POKEMON,
        30s,
        USE_SOUND_DETECTION
    );
    bool shiny_known = own.shiny_type != ShinyType::UNKNOWN;

    if (is_likely_shiny(own.shiny_type)){
        stats.shinies++;
        send_program_notification(
            env, NOTIFICATION_SHINY,
            COLOR_STAR_SHINY, "Shiny Starter",
            {{"Advance", std::to_string(target_advance)}, {"Details", target.to_string()}},
            "", own.get_best_screenshot()
        );
        if (TAKE_VIDEO){
            pbf_wait(context, 5000ms);
            pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 5000ms);
            context.wait_for_all_requests();
        }
        return BdspAttemptOutcome::Hit;
    }

    BdspObservedStarter observed;
    if (!clear_starter_battle(env, context)
        || !read_observed_pokemon(
            env, context, LANGUAGE, starter_base_stats(STARTER), 5, false, shiny_known, observed
        )
    ){
        stats.errors++;
        return BdspAttemptOutcome::Unverifiable;
    }

    BdspHitIdentification hit = identify_hit_advance(
        recovery.state, target_info.pokemon, target_advance, HIT_SEARCH_RADIUS, observed
    );
    if (!hit.success){
        env.log("Could not tell which advance that was: " + hit.failure_reason, COLOR_ORANGE);
        if (consistent_with(target, observed)){
            env.log("What was read matches the target, so it counts as a hit.", COLOR_BLUE);
            stats.hits++;
            return BdspAttemptOutcome::Hit;
        }
        env.log("What was read is not the target, so it counts as a miss.", COLOR_ORANGE);
        stats.missed++;
        return BdspAttemptOutcome::Missed;
    }
    if (m_aim.record_offset(env.logger(), hit.offset, AUTO_CALIBRATE)){
        TARGET_DISPLAY.set_correction(m_aim.bias());
    }

    if (hit.offset == 0){
        env.log("Landed on advance " + std::to_string(target_advance) + ", as aimed.",
            COLOR_BLUE);
        stats.hits++;
        return BdspAttemptOutcome::Hit;
    }

    env.log(
        "Landed on advance " + std::to_string(hit.advance) + " instead of "
        + std::to_string(target_advance) + " — "
        + (hit.offset > 0 ? "late by " : "early by ")
        + std::to_string(hit.offset > 0 ? hit.offset : -hit.offset) + " advance(s).",
        COLOR_ORANGE
    );
    stats.missed++;

    if (wanted(filters, searcher.generate(hit.advance))){
        env.log("It passes the filter anyway, so this one will do. Stopping.", COLOR_BLUE);
        return BdspAttemptOutcome::Hit;
    }
    return BdspAttemptOutcome::Missed;
}


void StarterRng::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StarterRng_Descriptor::Stats& stats = env.current_stats<StarterRng_Descriptor::Stats>();

    require_player(env.console, context, BUTTON_B);

    m_aim.reset();

    bool reset = false;
    uint64_t resets = 0;
    while (true){
        if (MAX_RESETS != 0 && resets >= MAX_RESETS){
            env.log("Reached the reset limit.", COLOR_ORANGE);
            break;
        }
        resets++;

        bool started = true;
        if (reset){
            go_home(env.console, context);
            if (!reset_game_from_home(
                env, env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            )){
                started = false;
            }
        }

        bool done = false;
        if (started){
            reset = true;
            done = run_attempt(env, context) == BdspAttemptOutcome::Hit;
        }

        stats.resets++;
        env.update_stats();

        if (done){
            break;
        }
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
