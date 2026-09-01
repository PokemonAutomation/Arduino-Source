/*  BDSP Blink Extraction
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "PokemonBDSP_BlinkExtraction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const double THRESHOLD_LOWEST = 0.04;
const double THRESHOLD_HIGHEST = 0.60;
const size_t THRESHOLD_STEPS = 40;
const size_t MINIMUM_PLATEAU_BLINKS = 20;
const double MINIMUM_SECONDS_FOR_THRESHOLD = 180;


static std::vector<double> blink_depths(const std::vector<BlinkMatchSample>& samples){
    std::vector<double> matches;
    matches.reserve(samples.size());
    for (const BlinkMatchSample& sample : samples){
        matches.emplace_back(sample.match);
    }
    std::vector<double> sorted = matches;
    std::sort(sorted.begin(), sorted.end());
    double resting = sorted.empty() ? 0.0 : sorted[sorted.size() / 2];
    if (resting <= 0){
        return std::vector<double>(samples.size(), 0.0);
    }

    std::vector<double> depths;
    depths.reserve(matches.size());
    for (double value : matches){
        double depth = (resting - value) / resting;
        depths.emplace_back(depth < 0 ? 0.0 : (depth > 1 ? 1.0 : depth));
    }
    return depths;
}

static size_t count_blinks(const std::vector<double>& depths, double threshold){
    size_t count = 0;
    bool shut = false;
    for (double depth : depths){
        if (depth > threshold){
            if (!shut){
                shut = true;
                count++;
            }
        }else{
            shut = false;
        }
    }
    return count;
}


double auto_blink_threshold(const std::vector<BlinkMatchSample>& samples){
    if (samples.empty()){
        return -1;
    }
    double watched = std::chrono::duration_cast<std::chrono::duration<double>>(
        samples.back().timestamp - samples.front().timestamp
    ).count();
    if (watched < MINIMUM_SECONDS_FOR_THRESHOLD){
        return -1;
    }
    std::vector<double> depths = blink_depths(samples);

    std::vector<double> thresholds(THRESHOLD_STEPS);
    std::vector<size_t> counts(THRESHOLD_STEPS);
    double low = std::log(THRESHOLD_LOWEST);
    double high = std::log(THRESHOLD_HIGHEST);
    for (size_t c = 0; c < THRESHOLD_STEPS; c++){
        thresholds[c] = std::exp(low + (high - low) * (double)c / (double)(THRESHOLD_STEPS - 1));
        counts[c] = count_blinks(depths, thresholds[c]);
    }

    //  Widest band over which the blink count barely moves.
    size_t best_span = 0;
    size_t best_start = 0;
    size_t best_end = 0;
    bool found = false;
    size_t c = 0;
    while (c < THRESHOLD_STEPS){
        if (counts[c] < MINIMUM_PLATEAU_BLINKS){
            c++;
            continue;
        }
        size_t end = c;
        while (end + 1 < THRESHOLD_STEPS
            && counts[end + 1] >= MINIMUM_PLATEAU_BLINKS
            && (counts[end + 1] > counts[c] ? counts[end + 1] - counts[c] : counts[c] - counts[end + 1]) <= 1
        ){
            end++;
        }
        //  Strictly wider, so that a tie keeps the lower-threshold plateau.
        if (!found || end - c > best_span){
            best_span = end - c;
            best_start = c;
            best_end = end;
            found = true;
        }
        c = end + 1;
    }

    if (!found){
        return -1;
    }
    return std::exp((std::log(thresholds[best_start]) + std::log(thresholds[best_end])) / 2);
}


std::vector<Blink> extract_blinks(
    const std::vector<BlinkMatchSample>& samples, double threshold, WallClock origin_time
){
    std::vector<Blink> blinks;
    if (samples.size() < 2 || threshold <= 0){
        return blinks;
    }
    std::vector<double> depths = blink_depths(samples);
    if (origin_time == WallClock::min()){
        origin_time = samples[0].timestamp;
    }
    double origin = std::chrono::duration_cast<std::chrono::duration<double>>(
        origin_time.time_since_epoch()
    ).count();

    size_t start = 0;
    bool shut = false;
    for (size_t c = 0; c <= depths.size(); c++){
        bool now_shut = c < depths.size() && depths[c] > threshold;
        if (now_shut && !shut){
            shut = true;
            start = c;
            continue;
        }
        if (now_shut || !shut){
            continue;
        }
        shut = false;


        size_t from = start >= 2 ? start - 2 : 0;
        size_t to = std::min(c + 2, depths.size());
        double weight = 0;
        double weighted_time = 0;
        double peak = 0;
        for (size_t i = from; i < to; i++){
            double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(
                samples[i].timestamp.time_since_epoch()
            ).count() - origin;
            weight += depths[i];
            weighted_time += depths[i] * seconds;
            peak = std::max(peak, depths[i]);
        }
        if (weight <= 0){
            continue;
        }
        Blink blink;
        blink.seconds = weighted_time / weight;
        blink.depth = peak;
        blink.frames = c - start;
        blinks.emplace_back(blink);
    }
    return blinks;
}


std::vector<BlinkEvent> group_blinks(
    const std::vector<Blink>& blinks, double double_blink_seconds
){
    std::vector<BlinkEvent> events;
    size_t c = 0;
    while (c < blinks.size()){
        BlinkEvent event;
        event.seconds = blinks[c].seconds;
        if (c + 1 < blinks.size()
            && blinks[c + 1].seconds - blinks[c].seconds < double_blink_seconds
        ){
            event.type = BlinkType::Double;
            c += 2;
        }else{
            event.type = BlinkType::Single;
            c += 1;
        }
        events.emplace_back(event);
    }
    return events;
}


TickFit fit_tick_period(
    const std::vector<BlinkEvent>& events, double lowest_seconds, double highest_seconds
){
    TickFit fit;
    if (events.size() < 3){
        return fit;
    }
    std::vector<double> gaps;
    gaps.reserve(events.size() - 1);
    for (size_t c = 1; c < events.size(); c++){
        gaps.emplace_back(events[c].seconds - events[c - 1].seconds);
    }

    const size_t STEPS = 400;
    for (size_t c = 0; c <= STEPS; c++){
        double period = lowest_seconds + (highest_seconds - lowest_seconds) * (double)c / (double)STEPS;
        double total = 0;
        double worst = 0;
        for (double gap : gaps){
            double ticks = gap / period;
            double error = std::abs(ticks - std::round(ticks));
            total += error * error;
            worst = std::max(worst, error);
        }
        double rms = std::sqrt(total / (double)gaps.size());
        if (fit.period_seconds == 0 || rms < fit.rms_ticks){
            fit.period_seconds = period;
            fit.rms_ticks = rms;
            fit.worst_ticks = worst;
        }
    }
    return fit;
}


static std::vector<uint64_t> assign_tick_indices(
    const std::vector<BlinkEvent>& events,
    double period_seconds,
    double origin_seconds,
    double& phase_ticks
){
    phase_ticks = 0;
    std::vector<uint64_t> ticks;
    if (events.empty() || !(period_seconds > 0)){
        return ticks;
    }

    std::vector<double> deviations;
    deviations.reserve(events.size());
    for (const BlinkEvent& event : events){
        double position = (event.seconds - origin_seconds) / period_seconds;
        deviations.emplace_back(position - std::round(position));
    }
    std::vector<double> sorted = deviations;
    std::sort(sorted.begin(), sorted.end());
    phase_ticks = sorted[sorted.size() / 2];

    ticks.reserve(events.size());
    for (const BlinkEvent& event : events){
        double position = (event.seconds - origin_seconds) / period_seconds - phase_ticks;
        long long index = (long long)std::llround(position);
        ticks.emplace_back(index < 0 ? 0 : (uint64_t)index);
    }
    return ticks;
}


bool build_samples(
    const std::vector<std::vector<BlinkEvent>>& streams,
    const std::vector<uint8_t>& slots,
    double period_seconds,
    std::vector<BlinkSample>& samples,
    std::string& failure_reason
){
    samples.clear();
    if (streams.empty() || streams.size() != slots.size()){
        failure_reason = "Each stream needs a slot.";
        return false;
    }
    if (!(period_seconds > 0)){
        failure_reason = "No tick period was fitted.";
        return false;
    }
    uint8_t npcs = (uint8_t)streams.size();

    //  Every stream is placed on one grid, anchored at whichever blinked first.
    double earliest = streams[0].empty() ? 0 : streams[0][0].seconds;
    for (const std::vector<BlinkEvent>& stream : streams){
        if (stream.empty()){
            failure_reason = "One of the watchers saw no blinks at all.";
            return false;
        }
        earliest = std::min(earliest, stream[0].seconds);
    }

    struct Placed{
        uint64_t tick;
        BlinkType type;
    };
    std::vector<std::vector<Placed>> placed(streams.size());
    uint64_t last_tick = 0;

    for (size_t s = 0; s < streams.size(); s++){
        //  Every stream is measured from the one shared origin, so each one's
        //  offset from the grid is removed in the same step that places it.
        double phase = 0;
        std::vector<uint64_t> ticks = assign_tick_indices(streams[s], period_seconds, earliest, phase);
        for (size_t c = 0; c < ticks.size(); c++){
            uint64_t tick = ticks[c];
            if (c != 0 && tick <= placed[s].back().tick){
                failure_reason = "Two blinks from the same watcher landed on one tick, so the "
                    "double-blink grouping is wrong.";
                return false;
            }
            placed[s].emplace_back(Placed{tick, streams[s][c].type});
            last_tick = std::max(last_tick, tick);
        }
    }

    //  Every roll from the first tick to the last was watched by every stream.
    for (uint64_t tick = 0; tick <= last_tick; tick++){
        for (size_t s = 0; s < streams.size(); s++){
            BlinkSample sample;
            sample.advance = tick * npcs + slots[s];
            //  Streams are short and in order, so a linear scan is cheap enough.
            for (const Placed& item : placed[s]){
                if (item.tick == tick){
                    sample.blinked = true;
                    sample.type = item.type;
                    break;
                }
                if (item.tick > tick){
                    break;
                }
            }
            samples.emplace_back(sample);
        }
    }

    std::sort(
        samples.begin(), samples.end(),
        [](const BlinkSample& a, const BlinkSample& b){ return a.advance < b.advance; }
    );
    uint64_t base = samples.front().advance; //  Always zero in practice
    for (BlinkSample& sample : samples){
        sample.advance -= base;
    }
    return true;
}


bool last_blink_anchor(
    const std::vector<std::vector<BlinkEvent>>& streams,
    const std::vector<uint8_t>& slots,
    double period_seconds,
    uint64_t& advance,
    double& seconds,
    size_t& stream_index
){
    stream_index = 0;
    if (streams.empty() || streams.size() != slots.size() || !(period_seconds > 0)){
        return false;
    }
    uint8_t npcs = (uint8_t)streams.size();

    //  The same shared origin build_samples() uses. Placing events against a
    //  different one would shift every tick index.
    double earliest = streams[0].empty() ? 0 : streams[0][0].seconds;
    for (const std::vector<BlinkEvent>& stream : streams){
        if (stream.empty()){
            return false;
        }
        earliest = std::min(earliest, stream[0].seconds);
    }

    bool found = false;
    for (size_t s = 0; s < streams.size(); s++){
        double phase = 0;
        std::vector<uint64_t> ticks = assign_tick_indices(streams[s], period_seconds, earliest, phase);
        if (ticks.empty()){
            continue;
        }
        double when = streams[s].back().seconds;
        if (found && when <= seconds){
            continue;
        }

        seconds = when;
        advance = ticks.back() * npcs + slots[s];
        stream_index = s;
        found = true;
    }
    return found;
}


bool step_blink_anchor(
    double elapsed_seconds,
    double period_seconds,
    uint8_t npcs,
    uint64_t previous_advance,
    uint64_t& advance
){
    advance = previous_advance;
    if (!(period_seconds > 0) || npcs == 0 || !(elapsed_seconds > 0)){
        return false;
    }
    long long ticks = std::llround(elapsed_seconds / period_seconds);
    if (ticks <= 0){
        return false;
    }
    advance = previous_advance + (uint64_t)ticks * npcs;
    return true;
}




}
}
}
