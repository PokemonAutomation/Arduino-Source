/*  BDSP Blink Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "PokemonBDSP_BlinkModel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  The fraction is 23 bits over 2^23 - 1.
static const double POKEMON_BLINK_FRACTION_SCALE = 8388607.0;
static const uint32_t POKEMON_BLINK_FRACTION_MASK = 0x7fffff;

//  Longest and shortest gap the game can produce, and the span between them.
static const double POKEMON_BLINK_LONGEST_SECONDS =
    BDSP_POKEMON_BLINK_MAX_SECONDS + BDSP_POKEMON_BLINK_OFFSET_SECONDS;
static const double POKEMON_BLINK_SPAN_SECONDS =
    BDSP_POKEMON_BLINK_MAX_SECONDS - BDSP_POKEMON_BLINK_MIN_SECONDS;


const char* blink_type_name(BlinkType type){
    switch (type){
    case BlinkType::Single: return "Single";
    case BlinkType::Double: return "Double";
    }
    return "?";
}

double bdsp_range_float(uint32_t roll, double minimum, double maximum){
    //  23 bits of mantissa divided by 2^23 - 1, not 2^23.
    double fraction = (double)(roll & POKEMON_BLINK_FRACTION_MASK) / POKEMON_BLINK_FRACTION_SCALE;
    return fraction * minimum + (1.0 - fraction) * maximum;
}

double bdsp_pokemon_blink_interval(uint32_t roll){
    return bdsp_range_float(roll, BDSP_POKEMON_BLINK_MIN_SECONDS, BDSP_POKEMON_BLINK_MAX_SECONDS)
        + BDSP_POKEMON_BLINK_OFFSET_SECONDS;
}


bool bdsp_pokemon_blink_fraction(double interval_seconds, uint32_t& fraction){
    //  bdsp_range_float runs backwards, so a longer gap means a smaller fraction.
    double scaled = (POKEMON_BLINK_LONGEST_SECONDS - interval_seconds) / POKEMON_BLINK_SPAN_SECONDS;

    //  The two extreme intervals are not exactly representable
    const double SLACK = 1e-9;
    if (!(scaled >= -SLACK) || scaled > 1.0 + SLACK){
        //  NaN is rejected rather than sliding through.
        return false;
    }
    scaled = scaled < 0.0 ? 0.0 : scaled;

    double value = scaled * POKEMON_BLINK_FRACTION_SCALE;
    fraction = value >= (double)POKEMON_BLINK_FRACTION_MASK
        ? POKEMON_BLINK_FRACTION_MASK
        : (uint32_t)value;
    return true;
}

bool bdsp_pokemon_blink_bucket_with_margin(
    double interval_seconds, uint32_t& bucket, double& margin_seconds
){
    uint32_t fraction = 0;
    if (!bdsp_pokemon_blink_fraction(interval_seconds, fraction)){
        return false;
    }

    const size_t SHIFT = 23 - BDSP_POKEMON_BLINK_KNOWN_BITS;
    const uint32_t BUCKET_WIDTH = (uint32_t)1 << SHIFT;
    bucket = fraction >> SHIFT;

    //  Distance to whichever end of the bucket is nearer, converted from fraction
    //  units back into seconds of timing error.
    uint32_t into_bucket = fraction - (bucket << SHIFT);
    uint32_t to_next = BUCKET_WIDTH - into_bucket;
    uint32_t nearest = into_bucket < to_next ? into_bucket : to_next;

    margin_seconds = (double)nearest
        * (BDSP_POKEMON_BLINK_MAX_SECONDS - BDSP_POKEMON_BLINK_MIN_SECONDS)
        / 8388607.0;

    //  An interval near either end of the range has nowhere to be wrong towards,
    //  so cap the margin by the distance to the range itself.
    double to_longest = POKEMON_BLINK_LONGEST_SECONDS - interval_seconds;
    double to_shortest = interval_seconds
        - (BDSP_POKEMON_BLINK_MIN_SECONDS + BDSP_POKEMON_BLINK_OFFSET_SECONDS);
    margin_seconds = std::min(margin_seconds, std::min(to_longest, to_shortest));
    margin_seconds = std::max(margin_seconds, 0.0);

    return true;
}

std::vector<BlinkTick> generate_blink_ticks(
    Pokemon::Xorshift128 rng,
    size_t ticks,
    uint8_t npcs,
    uint8_t slot
){
    if (npcs == 0){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "generate_blink_ticks(): There must be at least one NPC."
        );
    }
    if (slot >= npcs){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "generate_blink_ticks(): Slot is outside the per-tick order."
        );
    }

    std::vector<BlinkTick> ret;
    ret.reserve(ticks);

    //  Skip forward to the observed NPC's place
    rng.advance(slot);

    for (size_t c = 0; c < ticks; c++){
        uint32_t roll = rng.next();
        BlinkTick tick;
        tick.blinked = npc_blinks(roll);
        tick.type = NPC_blink_type(roll);
        ret.emplace_back(tick);

        rng.advance((uint64_t)npcs - 1);
    }

    return ret;
}




}
}
}
