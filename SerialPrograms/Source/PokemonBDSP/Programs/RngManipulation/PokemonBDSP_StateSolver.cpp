/*  BDSP RNG State Solver
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Pokemon/Pokemon_Gf2Matrix.h"
#include "PokemonBDSP_StateSolver.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


//  Bits 3, 2, 1 and 0, which is what an NPC blink reveals.
const size_t BLINK_BIT_ROW_FIRST = 124;

//  Bits 22, 21, 20 and 19: the top of the fraction a Pokemon blink interval encodes.
const size_t POKEMON_BLINK_BIT_ROW_FIRST = 105;


const size_t MAX_ENUMERATED_BITS = 12;


template <typename Verifier>
static size_t count_verified_candidates(
    const Gf2SolveResult& solved, Verifier&& verifier, Xorshift128State& winner
){
    size_t combinations = (size_t)1 << solved.null_space_dimension;
    size_t verified = 0;
    for (size_t mask = 0; mask < combinations; mask++){
        Gf2Vec128 vector = solved.solution;
        for (size_t bit = 0; bit < solved.null_space_dimension; bit++){
            if (((mask >> bit) & 1) != 0){
                vector ^= solved.null_space_basis[bit];
            }
        }
        Xorshift128State candidate = xorshift128_state_from_vector(vector);
        if (verifier(candidate)){
            winner = candidate;
            verified++;
            //  Two survivors already means the answer is not pinned down.
            if (verified > 1){
                break;
            }
        }
    }
    return verified;
}


//  Double check observations against a candidate state
static bool verify_samples(
    const Xorshift128State& state,
    const std::vector<BlinkSample>& samples,
    std::string& failure_reason
){
    Xorshift128 rng(state);
    uint64_t at = 0;
    for (const BlinkSample& sample : samples){
        while (at < sample.advance){
            rng.next();
            at++;
        }
        uint32_t roll = rng.next();
        at++;

        bool blinked = npc_blinks(roll);
        if (blinked != sample.blinked){
            failure_reason = blinked
                ? "Recovered state blinks at advance " + std::to_string(sample.advance)
                    + ", where nothing was seen."
                : "Recovered state does not blink at advance " + std::to_string(sample.advance) + ".";
            return false;
        }
        if (blinked && sample.type.has_value() && NPC_blink_type(roll) != *sample.type){
            failure_reason = "Recovered state disagrees with the blink type at advance "
                + std::to_string(sample.advance) + ".";
            return false;
        }
    }
    return true;
}


BlinkSolveResult solve_state_from_samples(const std::vector<BlinkSample>& samples, Logger* logger){
    BlinkSolveResult result;

    if (samples.empty()){
        result.failure_reason = "No rolls were watched.";
        return result;
    }
    if (samples[0].advance != 0){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "solve_state_from_samples(): Samples must start at advance zero."
        );
    }

    //  Each blink constrains the four low bits of one roll.
    std::vector<Gf2Vec128> equations;
    std::vector<bool> constants;
    Gf2Matrix128 transition = xorshift128_transition_matrix();
    uint64_t at = 0;
    size_t blinks = 0;

    for (const BlinkSample& sample : samples){
        if (sample.advance < at){
            throw InternalProgramError(
                nullptr, PA_CURRENT_FUNCTION,
                "solve_state_from_samples(): Samples are not sorted by advance."
            );
        }
        if (!sample.blinked){
            continue;
        }
        if (sample.advance > at){
            transition = xorshift128_transition_power(sample.advance - at) * transition;
            at = sample.advance;
        }
        blinks++;

        //  Bits 3, 2 and 1 are zero whenever a blink happens at all.
        for (size_t bit = 0; bit < 3; bit++){
            equations.emplace_back(transition[BLINK_BIT_ROW_FIRST + bit]);
            constants.emplace_back(false);
        }
        //  Bit 0 is the single/double distinction, which the detector may not have.
        if (sample.type.has_value()){
            equations.emplace_back(transition[BLINK_BIT_ROW_FIRST + 3]);
            constants.emplace_back(*sample.type == BlinkType::Double);
        }
    }
    result.equations_used = equations.size();

    if (equations.size() < 128){
        result.failure_reason = "Only " + std::to_string(equations.size())
            + " equations from " + std::to_string(blinks)
            + " blinks. At least 128 are needed.";
        return result;
    }

    Gf2SolveResult solved = gf2_solve_128(equations, constants);
    result.null_space_dimension = solved.null_space_dimension;

    if (!solved.consistent){
        result.failure_reason =
            "The observations contradict each other. A blink was probably missed or invented.";
        return result;
    }
    if (solved.null_space_dimension > MAX_ENUMERATED_BITS){
        result.failure_reason = "The observations leave "
            + std::to_string(solved.null_space_dimension)
            + " unknown bits, too many to resolve by checking. Collect more blinks.";
        return result;
    }

    Xorshift128State winner;
    std::string first_rejection;
    size_t verified = count_verified_candidates(
        solved,
        [&](const Xorshift128State& candidate){
            std::string reason;
            bool ok = verify_samples(candidate, samples, reason);
            if (!ok && first_rejection.empty()){
                first_rejection = std::move(reason);
            }
            return ok;
        },
        winner
    );

    if (verified == 0){
        result.failure_reason = first_rejection.empty()
            ? "No candidate state reproduces the observed blinks."
            : first_rejection;
        return result;
    }
    if (verified > 1){
        result.failure_reason =
            "More than one state reproduces the observed blinks. Collect more blinks.";
        return result;
    }

    result.state = winner;
    for (const BlinkSample& sample : samples){
        if (sample.blinked){
            result.advances_to_last_blink = sample.advance;
        }
    }
    result.success = true;
    if (logger != nullptr){
        logger->log(
            "Recovered RNG state " + result.state.to_string()
            + " from " + std::to_string(blinks)
            + " blinks over " + std::to_string(samples.size())
            + " watched rolls (" + std::to_string(result.equations_used) + " equations).",
            COLOR_BLUE
        );
    }
    return result;
}



size_t recommended_pokemon_blink_count(double tolerance_seconds){
    double bucket_seconds =
        (BDSP_POKEMON_BLINK_MAX_SECONDS - BDSP_POKEMON_BLINK_MIN_SECONDS)
        / (double)((size_t)1 << BDSP_POKEMON_BLINK_KNOWN_BITS);

    double usable_fraction = 1.0 - 2.0 * tolerance_seconds / bucket_seconds;
    if (!(usable_fraction > 0.1)){
        return 1000;
    }

    //  40 observations is 160 (not necessarily independent) equations for 128 unknowns,
    //  which is usually enough
    const size_t TARGET_USABLE = 40;
    return (size_t)((double)TARGET_USABLE / usable_fraction * 1.1) + 1;
}


static bool verify_pokemon_solution(
    const Xorshift128State& state,
    const std::vector<double>& intervals,
    double tolerance_seconds,
    double& worst_residual_seconds,
    size_t& mistimed,
    std::string& failure_reason
){
    Xorshift128 rng(state);
    worst_residual_seconds = 0;
    mistimed = 0;

    //  One in eight, and never fewer than two, so that a short capture is not held
    //  to a stricter standard than a long one.
    size_t allowed = intervals.size() / 8;
    allowed = allowed < 2 ? 2 : allowed;

    for (size_t c = 0; c < intervals.size(); c++){
        double expected = bdsp_pokemon_blink_interval(rng.next());
        double residual = std::abs(intervals[c] - expected);
        if (residual > tolerance_seconds){
            mistimed++;
            continue;
        }
        //  Reported over the agreeing intervals only, so that it measures the
        //  timing precision rather than the size of an outlier.
        if (residual > worst_residual_seconds){
            worst_residual_seconds = residual;
        }
    }

    if (mistimed > allowed){
        failure_reason = std::to_string(mistimed) + " of " + std::to_string(intervals.size())
            + " intervals disagree with the recovered state, which is too many to blame on"
            " mistimed blinks.";
        return false;
    }
    return true;
}


PokemonBlinkSolveResult solve_state_from_pokemon_blinks(
    const PokemonBlinkSolveRequest& request,
    Logger* logger
){
    PokemonBlinkSolveResult result;

    if (request.intervals.empty()){
        result.failure_reason = "No intervals were given.";
        return result;
    }
    if (!(request.tolerance_seconds > 0.0)){
        result.failure_reason = "The tolerance must be positive.";
        return result;
    }

    struct Reading{
        size_t index;
        uint32_t bucket;
        double margin;
    };
    std::vector<Reading> readings;
    readings.reserve(request.intervals.size());

    for (size_t c = 0; c < request.intervals.size(); c++){
        Reading reading{c, 0, 0.0};
        if (!bdsp_pokemon_blink_bucket_with_margin(request.intervals[c], reading.bucket, reading.margin)){
            //  Not a gap the game could ever have produced.
            result.observations_discarded++;
            continue;
        }
        readings.emplace_back(reading);
    }

    std::sort(
        readings.begin(), readings.end(),
        [](const Reading& a, const Reading& b){
            //  Index breaks ties so the choice of subset is reproducible.
            return a.margin != b.margin ? a.margin > b.margin : a.index < b.index;
        }
    );

    const size_t MINIMUM = 128 / BDSP_POKEMON_BLINK_KNOWN_BITS;
    size_t confident = 0;
    while (confident < readings.size() && readings[confident].margin >= request.tolerance_seconds){
        confident++;
    }

    if (readings.size() < MINIMUM){
        result.observations_discarded = request.intervals.size() - readings.size();
        result.failure_reason = "Only " + std::to_string(readings.size())
            + " of " + std::to_string(request.intervals.size())
            + " intervals were usable at all. At least " + std::to_string(MINIMUM)
            + " are needed. Collect more blinks.";
        return result;
    }

    size_t use = confident > MINIMUM ? confident : MINIMUM;

    const Gf2Matrix128& step = xorshift128_transition_matrix();
    std::vector<Gf2Matrix128> transitions;
    transitions.reserve(request.intervals.size());
    {
        //  Every blink costs exactly one advance,
        // so intervals[c] comes from the (c + 1)th roll
        Gf2Matrix128 transition = step;
        for (size_t c = 0; c < request.intervals.size(); c++){
            if (c != 0){
                transition = step * transition;
            }
            transitions.emplace_back(transition);
        }
    }

    //  Try the whole confident set, and if that fails, try leaving out one
    //  observation at a time.

    std::string last_failure;
    for (size_t trial = 0; trial < request.max_attempts; trial++){
        bool leave_one_out = trial > 0;
        if (leave_one_out && (use <= MINIMUM || trial > use)){
            //  Nothing left to spare without going under the minimum.
            break;
        }
        //  On trial n>0, skip the nth least confident reading in the used set.
        size_t omitted = leave_one_out ? use - trial : use;
        result.attempts = trial + 1;

        std::vector<Gf2Vec128> equations;
        std::vector<bool> constants;
        equations.reserve(use * BDSP_POKEMON_BLINK_KNOWN_BITS);
        constants.reserve(use * BDSP_POKEMON_BLINK_KNOWN_BITS);

        double weakest = 0;
        size_t count = 0;
        for (size_t c = 0; c < use; c++){
            if (c == omitted){
                continue;
            }
            const Reading& reading = readings[c];
            const Gf2Matrix128& transition = transitions[reading.index];
            //  The bucket's bits, most significant first, matching the row order.
            for (size_t bit = 0; bit < BDSP_POKEMON_BLINK_KNOWN_BITS; bit++){
                equations.emplace_back(transition[POKEMON_BLINK_BIT_ROW_FIRST + bit]);
                size_t shift = BDSP_POKEMON_BLINK_KNOWN_BITS - 1 - bit;
                constants.emplace_back(((reading.bucket >> shift) & 1) != 0);
            }
            weakest = reading.margin;
            count++;
        }

        Gf2SolveResult solved = gf2_solve_128(equations, constants);

        if (!solved.consistent){
            last_failure = "The intervals contradict each other.";
            continue;
        }
        if (solved.null_space_dimension > MAX_ENUMERATED_BITS){
            //  Leaving more out can only make this worse.
            result.null_space_dimension = solved.null_space_dimension;
            last_failure = "The intervals leave " + std::to_string(solved.null_space_dimension)
                + " unknown bits, too many to resolve by checking.";
            break;
        }

        Xorshift128State candidate;
        double residual = 0;
        size_t mistimed = 0;
        size_t verified = count_verified_candidates(
            solved,
            [&](const Xorshift128State& trial_state){
                double r = 0;
                size_t m = 0;
                std::string reason;
                if (!verify_pokemon_solution(trial_state, request.intervals,
                                             request.tolerance_seconds, r, m, reason)){
                    if (last_failure.empty()){
                        last_failure = std::move(reason);
                    }
                    return false;
                }
                residual = r;
                mistimed = m;
                return true;
            },
            candidate
        );
        if (verified != 1){
            if (verified > 1){
                last_failure = "More than one state fits these intervals.";
            }
            continue;
        }

        result.state = candidate;
        result.advances_to_last_interval = request.intervals.size() - 1;
        result.observations_used = count;
        result.observations_discarded = request.intervals.size() - count;
        result.equations_used = equations.size();
        result.weakest_margin_used = weakest;
        result.null_space_dimension = 0;
        result.worst_residual_seconds = residual;
        result.mistimed_intervals = mistimed;
        result.success = true;
        break;
    }

    if (!result.success){
        result.failure_reason = last_failure.empty()
            ? "Could not recover a state from these intervals."
            : last_failure + " Tried " + std::to_string(result.attempts)
                + " subsets of the most confident observations without success. "
                "At least one interval was probably mistimed; collect more blinks.";
        return result;
    }

    if (logger != nullptr){
        logger->log(
            "Recovered RNG state " + result.state.to_string()
            + " from " + std::to_string(result.observations_used)
            + " of " + std::to_string(request.intervals.size())
            + " intervals (worst residual " + std::to_string(result.worst_residual_seconds)
            + "s, weakest reading had " + std::to_string(result.weakest_margin_used)
            + "s of room).",
            COLOR_BLUE
        );
    }
    return result;
}




}
}
}
