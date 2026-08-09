/*  BDSP Blink Recovery
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceSession.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_BlinkModel.h"
#include "PokemonBDSP_BlinkRecovery.h"
#include "PokemonBDSP_StateReidentifier.h"
#include "PokemonBDSP_StateSolver.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;
using namespace std::chrono_literals;


std::vector<std::shared_ptr<const ImageRGB32>> load_eye_templates(
    const std::vector<BdspEyeTemplate>& setups
){
    std::vector<std::shared_ptr<const ImageRGB32>> eyes;
    eyes.reserve(setups.size());
    for (const BdspEyeTemplate& setup : setups){
        eyes.emplace_back(std::make_shared<const ImageRGB32>(
            RESOURCE_PATH() + "PokemonBDSP/Rng/" + setup.asset
        ));
    }
    return eyes;
}


static bool blink_scene_ready(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<EyeBlinkDetector>& detectors,
    const BlinkSceneConfig& config
){
    WallClock deadline = current_time() + config.attempt_timeout;
    size_t streak = 0;
    double best_seen = -1;
    while (current_time() < deadline){
        pbf_wait(context, 250ms);
        context.wait_for_all_requests();

        VideoSnapshot frame = env.console.video().snapshot();
        if (config.frame_filter && !config.frame_filter(frame)){
            streak = 0;
            continue;
        }
        double worst = 1.0;
        for (const EyeBlinkDetector& detector : detectors){
            worst = std::min(worst, detector.match(frame));
        }
        best_seen = std::max(best_seen, worst);
        if (worst < config.minimum_match){
            streak = 0;
            continue;
        }
        if (++streak >= config.required_streak){
            env.log("Characters in position, worst eye match "
                + tostr_default(worst) + ".");
            return true;
        }
    }
    env.log("Not settled yet; best worst-eye match was " + tostr_default(best_seen) + ".");
    return false;
}

bool wait_for_blink_scene(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const std::vector<BdspEyeTemplate>& setups,
    const std::vector<std::shared_ptr<const ImageRGB32>>& eyes,
    const BlinkSceneConfig& config,
    const std::string& scene_name
){
    std::vector<EyeBlinkDetector> detectors;
    for (size_t c = 0; c < setups.size(); c++){
        detectors.emplace_back(eyes[c], setups[c].box);
    }

    //  Recover from possible dropped button presses
    for (size_t attempt = 0; attempt < config.press_retries; attempt++){
        if (blink_scene_ready(env, context, detectors, config)){
            return true;
        }
        env.log(scene_name + " position not reached. A press was probably dropped. "
            "Pressing again (" + std::to_string(attempt + 1) + " of "
            + std::to_string(config.press_retries) + ").", COLOR_ORANGE);
        pbf_press_button(context, config.retry_button, 100ms, 1200ms);
        context.wait_for_all_requests();
    }
    env.log("Never reached the " + scene_name + " position. Giving up on this attempt.",
        COLOR_RED);
    return false;
}

void make_blink_watchers(
    const std::vector<BdspEyeTemplate>& setups,
    const std::vector<std::shared_ptr<const ImageRGB32>>& eyes,
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<PeriodicInferenceCallback>& callbacks
){
    for (size_t c = 0; c < setups.size(); c++){
        watchers.emplace_back(std::make_unique<EyeBlinkWatcher>(
            setups[c].label, eyes[c], setups[c].box, c == 0 ? COLOR_CYAN : COLOR_YELLOW
        ));
    }
    for (std::unique_ptr<EyeBlinkWatcher>& watcher : watchers){
        callbacks.emplace_back(*watcher, 16ms);
    }
}


bool collect_blink_matches(
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<std::vector<BlinkMatchSample>>& matches,
    WallClock& origin
){
    matches.clear();
    origin = WallClock::max();
    for (std::unique_ptr<EyeBlinkWatcher>& watcher : watchers){
        matches.emplace_back(watcher->samples());
        if (matches.back().empty()){
            return false;
        }
        origin = std::min(origin, matches.back()[0].timestamp);
    }
    return true;
}

std::vector<std::vector<BlinkEvent>> build_blink_streams(
    const std::vector<std::vector<BlinkMatchSample>>& matches,
    const std::vector<double>& thresholds,
    WallClock origin
){
    std::vector<std::vector<BlinkEvent>> streams;
    for (size_t c = 0; c < matches.size(); c++){
        streams.emplace_back(group_blinks(extract_blinks(matches[c], thresholds[c], origin)));
    }
    return streams;
}

//  prevent the screen from dimming with the right joystick
void keep_awake_if_due(ProControllerContext& context, WallClock& next, Seconds interval){
    if (current_time() < next){
        return;
    }
    pbf_move_right_joystick(context, {1.0, 0.0}, 80ms, 0ms);
    context.wait_for_all_requests();
    next = current_time() + interval;
}


BlinkRecovery recover_state_from_blinks(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<PeriodicInferenceCallback>& callbacks,
    BlinkCollectionDisplay& display,
    const BlinkRecoveryConfig& config,
    uint16_t timeout_seconds
){
    BlinkRecovery ret;
    WallClock deadline = current_time() + std::chrono::seconds(timeout_seconds);

    std::vector<double> frozen_thresholds(watchers.size(), -1);

    std::vector<double> estimated_thresholds(watchers.size(), -1);
    std::vector<size_t> estimated_from(watchers.size(), 0);

    bool have_candidate = false;
    Xorshift128State candidate;
    size_t candidate_events = 0;
    size_t event_count = 0;

    WallClock next_nudge = current_time() + config.keep_awake_interval;

    //  Held across the whole loop
    CancellableHolder<CancellableScope> subcontext(static_cast<CancellableScope&>(context));
    InferenceSession session(subcontext, env.console, callbacks);

    while (true){
        if (current_time() >= deadline){
            ret.failure_reason = "no RNG state confirmed within "
                + std::to_string((int)timeout_seconds / 60) + " minutes";
            return ret;
        }
        try{
            subcontext.wait_until(std::min(deadline, current_time() + config.poll_interval));
        }catch (OperationCancelledException&){}
        subcontext.throw_if_cancelled_with_exception();
        context.throw_if_cancelled();

        keep_awake_if_due(context, next_nudge, config.keep_awake_interval);

        std::vector<std::vector<BlinkMatchSample>> matches;
        WallClock origin = WallClock::max();
        if (!collect_blink_matches(watchers, matches, origin)){
            continue;
        }

        std::vector<double> thresholds;
        for (size_t w = 0; w < watchers.size(); w++){
            double threshold = frozen_thresholds[w];
            if (threshold <= 0){
                size_t counted = estimated_from[w];
                if (estimated_thresholds[w] <= 0 || matches[w].size() >= counted + counted / 4){
                    estimated_thresholds[w] = auto_blink_threshold(matches[w]);
                    estimated_from[w] = matches[w].size();
                }
                threshold = estimated_thresholds[w];
            }
            if (!(threshold > 0)){
                display.set_note("Determining blink threshold...");
                break;
            }
            thresholds.emplace_back(threshold);
        }
        if (thresholds.size() != watchers.size()){
            continue;
        }
        std::vector<std::vector<BlinkEvent>> streams =
            build_blink_streams(matches, thresholds, origin);

        size_t events = 0;
        const std::vector<BlinkEvent>* longest = &streams[0];
        for (const std::vector<BlinkEvent>& stream : streams){
            events += stream.size();
            if (stream.size() > longest->size()){
                longest = &stream;
            }
        }
        display.set_progress(events, config.min_rolls_to_try + config.confirmation_events);

        //  No need to go further until another blink is recorded
        if (events == event_count){
            continue;
        }
        event_count = events;

        if (longest->size() < 3){
            continue;
        }

        TickFit fit = fit_tick_period(*longest);
        if (!(fit.period_seconds > 0)){
            continue;
        }
        if (have_candidate && events < candidate_events + config.confirmation_events){
            continue;
        }

        if (events < config.min_rolls_to_try){
            continue;
        }

        std::vector<uint8_t> slots(streams.size());
        for (uint8_t c = 0; c < (uint8_t)streams.size(); c++){
            slots[c] = c;
        }
        BlinkSolveResult solved;
        std::vector<uint8_t> winning_slots;
        for (size_t attempt = 0; attempt < streams.size(); attempt++){
            std::vector<BlinkSample> samples;
            std::string failure;
            if (build_samples(streams, slots, fit.period_seconds, samples, failure)){
                BlinkSolveResult attempt_result = solve_state_from_samples(samples, nullptr);
                if (attempt_result.success){
                    solved = attempt_result;
                    winning_slots = slots;
                    break;
                }
            }
            std::rotate(slots.begin(), slots.begin() + 1, slots.end());
        }
        if (!solved.success){
            env.log(std::to_string(events) + " rolls seen, no solution yet.");
            continue;
        }

        if (frozen_thresholds[0] <= 0){
            std::string report;
            for (size_t w = 0; w < watchers.size(); w++){
                frozen_thresholds[w] = thresholds[w];
                report += (report.empty() ? "" : ", ")
                    + watchers[w]->label() + " " + tostr_default(thresholds[w]);
            }
            env.log("Thresholds fixed so that blink numbering stops moving: " + report + ".");
        }
        if (!have_candidate){
            env.log("Provisional state " + solved.state.to_string() + " from "
                + std::to_string(events) + " rolls. Collecting "
                + std::to_string(config.confirmation_events) + " more to confirm.");
            have_candidate = true;
            candidate = solved.state;
            candidate_events = events;
            continue;
        }
        if (solved.state != candidate){
            env.log(
                "The RNG state changed as more blinks arrived, so the earlier one was "
                "wrong. Continuing.",
                COLOR_ORANGE
            );
            candidate = solved.state;
            candidate_events = events;
            continue;
        }

        uint64_t anchor_advance = 0;
        double anchor_seconds = 0;
        size_t anchor_stream = 0;
        if (!last_blink_anchor(
            streams, winning_slots, fit.period_seconds,
            anchor_advance, anchor_seconds, anchor_stream
        )){
            ret.failure_reason = "the RNG state was found but no blink could anchor the clock";
            return ret;
        }
        ret.anchor_stream = anchor_stream;
        ret.clock.anchor_advance = anchor_advance;
        ret.clock.anchor_time = origin + std::chrono::duration_cast<WallDuration>(
            std::chrono::duration<double>(anchor_seconds)
        );
        ret.clock.tick_seconds = fit.period_seconds;
        ret.clock.npcs = config.npcs;
        ret.state = solved.state;
        ret.events = events;
        ret.thresholds = frozen_thresholds;
        ret.slots = winning_slots;
        ret.success = true;
        return ret;
    }
}


bool reanchor_absolute(
    const BlinkRecovery& recovery,
    const std::vector<std::vector<BlinkEvent>>& streams,
    WallClock origin,
    const BlinkRecoveryConfig& config,
    AdvanceClock& clock,
    Logger& logger
){
    if (recovery.anchor_stream >= streams.size() || config.reanchor_blinks < 2){
        return false;
    }
    const std::vector<BlinkEvent>& stream = streams[recovery.anchor_stream];
    if (stream.size() < config.reanchor_blinks){
        return false;
    }
    size_t first = stream.size() - config.reanchor_blinks;

    auto time_of = [&](size_t index){
        return origin + std::chrono::duration_cast<WallDuration>(
            std::chrono::duration<double>(stream[index].seconds)
        );
    };

    std::vector<uint32_t> intervals;
    intervals.reserve(config.reanchor_blinks - 1);
    for (size_t c = first + 1; c < stream.size(); c++){
        double gap = (stream[c].seconds - stream[c - 1].seconds) / clock.tick_seconds;
        long long ticks = std::llround(gap);
        if (ticks < 1){
            //  Two events on one tick means the grouping is wrong
            return false;
        }
        intervals.emplace_back((uint32_t)ticks);
    }

    ReidentifyRequest request;
    request.base_state = recovery.state;
    request.npcs = clock.npcs;
    request.method = ReidentifyMethod::Intervals;
    request.intervals = intervals;

    uint64_t estimate = clock.advance_at(time_of(first));
    request.search_min = estimate > config.reanchor_search_radius
        ? estimate - config.reanchor_search_radius
        : 0;
    request.search_max = estimate + config.reanchor_search_radius;

    ReidentifyResult result = reidentify_advances(request);
    if (!result.success){
        logger.log(
            "Absolute re-anchor found nothing usable, so the stepped clock stands. "
            + result.failure_reason,
            COLOR_ORANGE
        );
        return false;
    }

    //  How far the clock had drifted
    WallClock last_time = time_of(stream.size() - 1);
    double drift_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(
        last_time - clock.time_of_advance(result.advances_to_last_blink)
    ).count();
    double seconds_per_advance = clock.npcs == 0
        ? clock.tick_seconds
        : clock.tick_seconds / (double)clock.npcs;
    double drift_advances = drift_seconds / seconds_per_advance;

    clock.anchor_advance = result.advances_to_last_blink;
    clock.anchor_time = last_time;

    std::string note = "Re-anchored at advance " + std::to_string(clock.anchor_advance)
        + ", " + tostr_fixed(drift_seconds, 3) + "s ("
        + tostr_fixed(drift_advances, 2) + " advances) off what the clock predicted.";
    //  Half an advance is where the aim would actually land somewhere else.
    logger.log(note, std::abs(drift_advances) >= 0.5 ? COLOR_ORANGE : COLOR_BLUE);
    return true;
}


void hold_and_reanchor(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<PeriodicInferenceCallback>& callbacks,
    BlinkRecovery& recovery,
    const BlinkRecoveryConfig& config,
    uint64_t press_advance,
    double lead_seconds,
    RngStateDisplay& display
){
    AdvanceClock& clock = recovery.clock;
    WallClock next_nudge = current_time() + config.keep_awake_interval;

    const uint64_t entry_advance = clock.anchor_advance;
    const WallClock entry_time = clock.anchor_time;
    size_t reanchors = 0;
    size_t consecutive_failures = 0;
    WallClock next_reanchor = current_time();

    CancellableHolder<CancellableScope> subcontext(static_cast<CancellableScope&>(context));
    InferenceSession session(subcontext, env.console, callbacks);

    while (true){
        //  Recomputed every pass, because re-anchoring is exactly what changes it.
        WallClock leave_at = clock.time_of_advance(press_advance)
            - std::chrono::duration_cast<WallDuration>(
                std::chrono::duration<double>(lead_seconds)
            );
        // the press is timed off this anchor, so this is the most valuable check
        bool leaving = current_time() >= leave_at;
        if (!leaving){
            try{
                subcontext.wait_until(std::min({
                    leave_at, next_reanchor, current_time() + config.poll_interval
                }));
            }catch (OperationCancelledException&){}
            subcontext.throw_if_cancelled_with_exception();
            context.throw_if_cancelled();
            keep_awake_if_due(context, next_nudge, config.keep_awake_interval);
        }

        display.set_advances(clock.advance_at(current_time()));
        if (!leaving && current_time() < next_reanchor){
            continue;
        }
        next_reanchor = current_time() + config.reanchor_interval;

        bool reanchored = false;
        std::vector<std::vector<BlinkMatchSample>> matches;
        WallClock origin = WallClock::max();
        if (collect_blink_matches(watchers, matches, origin)){
            WallClock keep_from = current_time() - config.blink_retention;
            for (std::unique_ptr<EyeBlinkWatcher>& watcher : watchers){
                watcher->discard_before(keep_from);
            }
            std::vector<std::vector<BlinkEvent>> streams =
                build_blink_streams(matches, recovery.thresholds, origin);
            reanchored = reanchor_absolute(
                recovery, streams, origin, config, clock, env.logger()
            );
        }

        if (reanchored){
            reanchors++;
            consecutive_failures = 0;
        }else{
            consecutive_failures++;
            if (consecutive_failures >= config.max_reanchor_failures){
                OperationFailedException::fire(
                    ErrorReport::NO_ERROR_REPORT,
                    std::to_string(consecutive_failures)
                    + " consecutive re-anchor failures: the blinks can no longer be read, "
                    "so the clock cannot be trusted to time the press.",
                    env.console
                );
            }
        }

        if (!leaving){
            continue;
        }
        if (current_time() < clock.time_of_advance(press_advance)
            - std::chrono::duration_cast<WallDuration>(
                std::chrono::duration<double>(lead_seconds)
            )
        ){
            continue;
        }

        if (reanchors == 0){
            env.log("Held without re-anchoring: the clock is still the one the "
                "recovery produced.", COLOR_BLUE);
            return;
        }
        int64_t moved = (int64_t)clock.anchor_advance - (int64_t)entry_advance;
        double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(
            clock.anchor_time - entry_time
        ).count();
        std::string note = "Re-anchored " + std::to_string(reanchors) + " time(s) over "
            + tostr_fixed(seconds, 1) + "s, moving the anchor "
            + std::to_string(moved) + " advance(s)";
        if (moved > 0 && clock.npcs != 0){
            double implied = seconds / ((double)moved / (double)clock.npcs);
            note += ": implied tick " + tostr_fixed(implied, 5)
                + "s against the fitted " + tostr_fixed(clock.tick_seconds, 5)
                + "s (" + tostr_fixed(100 * (implied / clock.tick_seconds - 1), 3) + "%)";
        }
        env.log(note + ".", COLOR_BLUE);
        return;
    }
}




}
}
}
