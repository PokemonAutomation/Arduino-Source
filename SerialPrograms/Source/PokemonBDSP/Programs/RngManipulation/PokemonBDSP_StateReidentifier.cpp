/*  BDSP RNG State Reidentifier
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "PokemonBDSP_StateReidentifier.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;

const uint64_t CANCEL_CHECK_INTERVAL = 65536;

const size_t MAX_MATCHES_TRACKED = 8;


//  bits 1-3 say whether a blink happened, bit 0 says which kind.
static std::vector<uint8_t> generate_nibbles(const Xorshift128State& base_state, uint64_t count){
    std::vector<uint8_t> ret;
    ret.reserve((size_t)count);
    Xorshift128 rng(base_state);
    for (uint64_t c = 0; c < count; c++){
        ret.emplace_back((uint8_t)(rng.next() & 0x0f));
    }
    return ret;
}
static bool nibble_blinked(uint8_t nibble){
    return (nibble & 0x0e) == 0;
}
static BlinkType nibble_type(uint8_t nibble){
    return (BlinkType)(nibble & 1);
}


static ReidentifyResult reidentify_by_intervals(
    const ReidentifyRequest& request,
    Logger* logger
){
    ReidentifyResult result;

    uint64_t stride = request.npcs;
    uint64_t span_ticks = 0;
    for (uint32_t interval : request.intervals){
        if (interval == 0){
            result.failure_reason = "An interval of zero was given. Two blinks cannot share a tick.";
            return result;
        }
        span_ticks += interval;
    }
    uint64_t span = span_ticks * stride;

    std::vector<uint8_t> nibbles = generate_nibbles(request.base_state, request.search_max + span + 1);

    uint64_t first_match = 0;
    for (uint64_t start = request.search_min; start <= request.search_max; start++){
        if (!nibble_blinked(nibbles[(size_t)start])){
            continue;
        }

        //  Every observed gap has to match exactly: a blink at each end, and none in any of the ticks between
        bool matched = true;
        uint64_t position = start;
        for (uint32_t interval : request.intervals){
            for (uint32_t tick = 1; tick < interval; tick++){
                if (nibble_blinked(nibbles[(size_t)(position + (uint64_t)tick * stride)])){
                    matched = false;
                    break;
                }
            }
            if (!matched){
                break;
            }
            position += (uint64_t)interval * stride;
            if (!nibble_blinked(nibbles[(size_t)position])){
                matched = false;
                break;
            }
        }
        if (!matched){
            continue;
        }

        if (result.match_count == 0){
            first_match = start;
        }
        result.match_count++;
        if (result.match_count >= MAX_MATCHES_TRACKED){
            break;
        }
    }

    if (result.match_count == 0){
        result.failure_reason = "No position in the search range fits the observed blinks.";
        return result;
    }
    if (result.match_count > 1){
        result.ambiguous = true;
        result.failure_reason = "The observed blinks fit " + std::to_string(result.match_count)
            + " positions. Narrow the search range or collect more blinks.";
        return result;
    }

    result.success = true;
    result.advances_to_first_blink = first_match;
    result.advances_to_last_blink = first_match + span;
    if (logger != nullptr){
        logger->log(
            "Reidentified: " + std::to_string(result.advances_to_last_blink)
            + " advances since the known state.",
            COLOR_BLUE
        );
    }
    return result;
}


static ReidentifyResult reidentify_by_types(
    const ReidentifyRequest& request,
    Logger* logger
){
    ReidentifyResult result;

    size_t observed = request.types.size();
    uint64_t range = request.search_max - request.search_min + 1;
    //  One bit per blink
    if (observed < 64 && ((uint64_t)1 << observed) < range){
        result.failure_reason = "Only " + std::to_string(observed)
            + " blink types for a range of " + std::to_string(range)
            + " advances. This cannot identify a unique position.";
        return result;
    }

    //  Blinks average one in eight ticks; allow enough time for uncommonly spaced out blinks
    uint64_t stride = request.npcs;
    uint64_t reach = (uint64_t)observed * 64 * stride;
    std::vector<uint8_t> nibbles = generate_nibbles(request.base_state, request.search_max + reach + 1);

    uint64_t first_match = 0;
    uint64_t last_match_end = 0;

    for (uint64_t phase = 0; phase < stride; phase++){
        std::vector<uint64_t> positions;
        std::vector<BlinkType> observed_types;
        uint64_t scanned = 0;
        for (uint64_t index = phase; index < nibbles.size(); index += stride, scanned++){
            if (nibble_blinked(nibbles[(size_t)index])){
                positions.emplace_back(index);
                observed_types.emplace_back(nibble_type(nibbles[(size_t)index]));
            }
        }
        if (positions.size() < observed){
            continue;
        }

        for (size_t start = 0; start + observed <= positions.size(); start++){
            if (positions[start] < request.search_min || positions[start] > request.search_max){
                continue;
            }
            bool matched = true;
            for (size_t c = 0; c < observed; c++){
                if (observed_types[start + c] != request.types[c]){
                    matched = false;
                    break;
                }
            }
            if (!matched){
                continue;
            }

            if (result.match_count == 0){
                first_match = positions[start];
                last_match_end = positions[start + observed - 1];
            }
            result.match_count++;
        }
        if (result.match_count >= MAX_MATCHES_TRACKED){
            break;
        }
    }

    if (result.match_count == 0){
        result.failure_reason = "No position in the search range fits the observed blink types.";
        return result;
    }
    if (result.match_count > 1){
        result.ambiguous = true;
        result.failure_reason = "The observed blink types fit " + std::to_string(result.match_count)
            + " positions. Narrow the search range or collect more blinks.";
        return result;
    }

    result.success = true;
    result.advances_to_first_blink = first_match;
    result.advances_to_last_blink = last_match_end;
    if (logger != nullptr){
        logger->log(
            "Reidentified: " + std::to_string(result.advances_to_last_blink)
            + " advances since the known state.",
            COLOR_BLUE
        );
    }
    return result;
}


ReidentifyResult reidentify_advances(
    const ReidentifyRequest& request,
    Logger* logger
){
    ReidentifyResult result;

    if (request.npcs == 0){
        result.failure_reason = "There must be at least one NPC on screen.";
        return result;
    }
    if (request.search_min > request.search_max){
        result.failure_reason = "The search range is empty.";
        return result;
    }

    switch (request.method){
    case ReidentifyMethod::Intervals:
        if (request.intervals.empty()){
            result.failure_reason = "No intervals were given.";
            return result;
        }
        return reidentify_by_intervals(request, logger);

    case ReidentifyMethod::Types:
        if (request.types.empty()){
            result.failure_reason = "No blink types were given.";
            return result;
        }
        return reidentify_by_types(request, logger);
    }

    result.failure_reason = "Unknown reidentification method.";
    return result;
}




}
}
}
