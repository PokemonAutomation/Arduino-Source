/*  BDSP Intro Seed Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <chrono>
#include <memory>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_BlinkExtraction.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_EyeBlinkDetector.h"
#include "PokemonBDSP_IntroSeedFinder.h"
#include "PokemonBDSP_StateSolver.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


const Seconds KEEP_AWAKE_INTERVAL = 180s;

//  Each usable gap pins four bits, so 128 bits needs 32 of them.
const size_t MIN_USABLE_TO_TRY = 32;

const size_t CONFIRMATION_BLINKS = 6;


IntroSeedFinder_Descriptor::IntroSeedFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:IntroSeedFinder",
        STRING_POKEMON + " BDSP", "Intro Seed Finder",
        "",
        "Recover the RNG seed from the intro cutscene by watching Munchlax blink, so that the "
        "secret ID can be worked out from the trainer card afterwards.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}


IntroSeedFinder::IntroSeedFinder()
    : TOLERANCE_SECONDS(
        "<b>Timing tolerance (seconds):</b><br>"
        "How far off a measured gap might be. Here the gap itself is the value being read, "
        "so this decides which readings are trusted. Measured error end to end on real "
        "hardware is about 0.05 s, which makes 0.05 too tight to be safe and 0.07 roughly "
        "the floor.",
        LockMode::LOCK_WHILE_RUNNING,
        0.10, 0.02, 0.30
    )
    , GIVE_UP_SECONDS(
        "<b>Give up after (seconds):</b><br>"
        "A backstop, not a target. The program stops as soon as it has recovered the seed and "
        "confirmed it, which usually takes eight to ten minutes. This is how long it waits "
        "before concluding that something is wrong with the capture rather than that it needs "
        "more of it.",
        LockMode::LOCK_WHILE_RUNNING,
        1500, 120, 5400
    )
{
    PA_ADD_OPTION(TOLERANCE_SECONDS);
    PA_ADD_OPTION(GIVE_UP_SECONDS);
    PA_ADD_OPTION(COLLECTION_DISPLAY);
    PA_ADD_OPTION(STATE_DISPLAY);
}


//  some gaps are too near a bucket boundary to be read (roughly a third at the default tolerance)
static size_t count_usable(const std::vector<double>& intervals, double tolerance){
    size_t usable = 0;
    for (double gap : intervals){
        uint32_t bucket = 0;
        double margin = 0;
        if (bdsp_pokemon_blink_bucket_with_margin(gap, bucket, margin) && margin >= tolerance){
            usable++;
        }
    }
    return usable;
}

static std::vector<double> intervals_of(const std::vector<Blink>& blinks){
    std::vector<double> intervals;
    if (blinks.size() < 2){
        return intervals;
    }
    intervals.reserve(blinks.size() - 1);
    for (size_t c = 1; c < blinks.size(); c++){
        intervals.emplace_back(blinks[c].seconds - blinks[c - 1].seconds);
    }
    return intervals;
}


void IntroSeedFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    STATE_DISPLAY.reset();
    COLLECTION_DISPLAY.reset();

    double fps = env.console.video().fps_source();
    if (fps > 0){
        env.log("Capture is running at " + tostr_default(fps) + " fps.");
        if (fps < 25){
            env.log(
                "That is low for this. A gap has to be measured to about a tenth of a second, "
                "and below roughly 30 fps the timing error alone approaches that.",
                COLOR_ORANGE
            );
        }
    }

    std::shared_ptr<const ImageRGB32> eye = std::make_shared<const ImageRGB32>(
        RESOURCE_PATH() + "PokemonBDSP/Rng/MunchlaxEye-Intro.png"
    );
    EyeBlinkWatcher watcher("Munchlax", eye, {0.3828, 0.6306, 0.0208, 0.0481}, COLOR_CYAN);
    std::vector<PeriodicInferenceCallback> callbacks;
    callbacks.emplace_back(watcher, 16ms);

    size_t expected = recommended_pokemon_blink_count(TOLERANCE_SECONDS);
    env.log("Watching Munchlax. Around " + std::to_string(expected)
        + " blinks are usually enough, but this stops when the answer is confirmed rather "
        "than at a fixed count.");

    //  how many blinks are needed comes down to luck
    WallClock deadline = current_time() + std::chrono::seconds(GIVE_UP_SECONDS);
    WallClock next_nudge = current_time() + KEEP_AWAKE_INTERVAL;

    bool have_candidate = false;
    Xorshift128State candidate;
    size_t candidate_blinks = 0;
    double frozen_threshold = -1;

    PokemonBlinkSolveResult confirmed;
    size_t confirmed_blinks = 0;

    while (true){
        if (current_time() >= deadline){
            throw UserSetupError(env.logger(),
                "Gave up after " + std::to_string((int)GIVE_UP_SECONDS) + " seconds without a "
                "confirmed seed. Check the overlay box is on Munchlax's eye and that the log "
                "shows blinks arriving every few seconds."
            );
        }

        wait_until(
            env.console, context,
            std::min(deadline, current_time() + 10s),
            callbacks
        );

        if (current_time() >= next_nudge){
            pbf_move_right_joystick(context, {1.0, 0.0}, 80ms, 0ms);
            context.wait_for_all_requests();
            next_nudge = current_time() + KEEP_AWAKE_INTERVAL;
        }

        std::vector<BlinkMatchSample> samples = watcher.samples();
        if (samples.empty()){
            continue;
        }
        double threshold = frozen_threshold > 0 ? frozen_threshold : auto_blink_threshold(samples);
        if (!(threshold > 0)){
            COLLECTION_DISPLAY.set_progress(0, expected);
            continue;
        }
        std::vector<Blink> blinks = extract_blinks(samples, threshold);
        std::vector<double> intervals = intervals_of(blinks);
        size_t usable = count_usable(intervals, TOLERANCE_SECONDS);

        COLLECTION_DISPLAY.set_progress(blinks.size(), expected);
        if (!intervals.empty()){
            COLLECTION_DISPLAY.set_last_interval(intervals.back());
        }

        if (blinks.size() >= 2 && blinks.size() < expected){
            double elapsed = blinks.back().seconds - blinks.front().seconds;
            double per_blink = elapsed / (double)(blinks.size() - 1);
            COLLECTION_DISPLAY.set_estimated_remaining(per_blink * (double)(expected - blinks.size()));
        }

        if (usable < MIN_USABLE_TO_TRY){
            env.log(std::to_string(blinks.size()) + " blinks, " + std::to_string(usable)
                + " of " + std::to_string(MIN_USABLE_TO_TRY) + " confident gaps needed before "
                "solving is worth trying.");
            continue;
        }
        if (frozen_threshold <= 0){
            frozen_threshold = threshold;
            env.log("Threshold fixed at " + tostr_default(threshold)
                + " for the rest of the run, so that blink numbering stops moving.");
        }

        if (have_candidate && blinks.size() < candidate_blinks + CONFIRMATION_BLINKS){
            env.log(std::to_string(blinks.size()) + " blinks, " + std::to_string(usable)
                + " usable. Waiting for "
                + std::to_string(candidate_blinks + CONFIRMATION_BLINKS - blinks.size())
                + " more to confirm.");
            continue;
        }

        PokemonBlinkSolveRequest request;
        request.intervals = intervals;
        request.tolerance_seconds = TOLERANCE_SECONDS;
        PokemonBlinkSolveResult result = solve_state_from_pokemon_blinks(request, nullptr);
        if (!result.success){
            env.log(std::to_string(blinks.size()) + " blinks, " + std::to_string(usable)
                + " usable, no solution yet.");
            continue;
        }

        if (have_candidate && result.state == candidate){
            confirmed = result;
            confirmed_blinks = blinks.size();
            break;
        }
        if (have_candidate){
            //  Two different answers means at least one came from a bad reading
            env.log(
                "The recovered state changed as more blinks arrived, so the earlier one was "
                "wrong. Continuing.",
                COLOR_ORANGE
            );
        }else{
            env.log("Provisional state " + result.state.to_string()
                + " from " + std::to_string(result.observations_used)
                + " gaps. Collecting " + std::to_string(CONFIRMATION_BLINKS)
                + " more blinks to confirm it.");
        }
        have_candidate = true;
        candidate = result.state;
        candidate_blinks = blinks.size();
    }

    uint64_t seed0 = 0;
    uint64_t seed1 = 0;
    xorshift128_state_to_seed_pair(confirmed.state, seed0, seed1);

    env.log("--------");
    env.log("Seed confirmed over " + std::to_string(confirmed_blinks) + " blinks.", COLOR_BLUE);
    env.log("State: " + confirmed.state.to_string(), COLOR_BLUE);
    env.log("PokeFinder seeds: " + tostr_hex_padded(16, seed0)
        + " " + tostr_hex_padded(16, seed1), COLOR_BLUE);
    env.log("Used " + std::to_string(confirmed.observations_used) + " gaps over "
        + std::to_string(confirmed.attempts) + " attempt(s); worst residual "
        + tostr_default(confirmed.worst_residual_seconds) + "s, weakest reading had "
        + tostr_default(confirmed.weakest_margin_used) + "s of room, "
        + std::to_string(confirmed.mistimed_intervals) + " gaps disagreed.", COLOR_BLUE);

    STATE_DISPLAY.set_state(confirmed.state, confirmed_blinks);
    STATE_DISPLAY.set_confidence_unique();
    COLLECTION_DISPLAY.set_progress(confirmed_blinks, confirmed_blinks);


    env.log("--------");
    env.log(
        "Now finish the intro however you like — the timing no longer matters. Then open the "
        "trainer card and note the ID No.",
        COLOR_BLUE
    );
    env.log(
        "Put the two seeds above into PokeFinder's Gen 8 ID search and look for the advance "
        "whose displayed TID matches your card. That row gives your SID, and TID xor SID is "
        "the TSV you will want for shiny hunting afterwards.",
        COLOR_BLUE
    );
    env.log(
        "If two advances in range share your displayed TID, search a narrower range or check "
        "which one matches a Pokemon you have caught.",
        COLOR_BLUE
    );
}




}
}
}
