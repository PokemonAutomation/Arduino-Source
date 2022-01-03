/*  Waterfill Intrinsics (64-bit integer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Intrinsics_u64_H
#define PokemonAutomation_Kernels_Waterfill_Intrinsics_u64_H

#include <stdint.h>
#include <string>
#include "Common/Compiler.h"

#if _MSC_VER
#include <intrin.h>
#endif

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
struct Intrinsics_u64{


using Word = uint64_t;

static std::string to_str(Word x){
    std::string str;
    for (size_t c = 0; c < 64; c++){
        str += ((x >> c) & 1) ? "1" : "0";
    }
    return str;
}

static PA_FORCE_INLINE Word LOW_BIT(){
    return 1;
}
static PA_FORCE_INLINE Word HIGH_BIT(){
    return 0x8000000000000000;
}

static PA_FORCE_INLINE bool low_bit(Word x){
    return x & 0x0000000000000001;
}
static PA_FORCE_INLINE bool high_bit(Word x){
    return x & 0x8000000000000000;
}

static PA_FORCE_INLINE size_t least_significant_bit(Word x){
#if _MSC_VER
    unsigned long tmp;
    _BitScanForward64(&tmp, x);
    return tmp;
#elif __GNUC__
    return __builtin_ctz(x);
#else
#error "No intrinsic for this compiler."
#endif
}
static PA_FORCE_INLINE size_t most_significant_bit(Word x){
#if _MSC_VER
    unsigned long tmp;
    _BitScanReverse64(&tmp, x);
    return tmp;
#elif __GNUC__
    return 63 - __builtin_clzll(x);
#else
#error "No intrinsic for this compiler."
#endif
}

static PA_FORCE_INLINE void popcount_sumindex(
    uint64_t& pop_count, uint64_t& sum_index,
    Word x
){
    //  1 -> 2
    uint64_t sum_high;
    uint64_t pop_high = (x >> 1) & 0x5555555555555555;
    uint64_t sumxaxis = pop_high;
    uint64_t popcount = (x & 0x5555555555555555) + pop_high;

    //  2 -> 4
    sum_high = (sumxaxis >> 2) & 0x3333333333333333;
    pop_high = (popcount >> 2) & 0x3333333333333333;
    sumxaxis = (sumxaxis & 0x3333333333333333) + sum_high;
    sumxaxis += pop_high << 1;
    popcount = (popcount & 0x3333333333333333) + pop_high;

    //  4 -> 8
    sum_high = (sumxaxis >> 4) & 0x0f0f0f0f0f0f0f0f;
    pop_high = (popcount >> 4) & 0x0f0f0f0f0f0f0f0f;
    sumxaxis = (sumxaxis & 0x0f0f0f0f0f0f0f0f) + sum_high;
    sumxaxis += pop_high << 2;
    popcount = (popcount & 0x0f0f0f0f0f0f0f0f) + pop_high;

    //  8 -> 16
    sum_high = (sumxaxis >> 8) & 0x00ff00ff00ff00ff;
    pop_high = (popcount >> 8) & 0x00ff00ff00ff00ff;
    sumxaxis = (sumxaxis & 0x00ff00ff00ff00ff) + sum_high;
    sumxaxis += pop_high << 3;
    popcount = (popcount & 0x00ff00ff00ff00ff) + pop_high;

    //  16 -> 32
    sum_high = (sumxaxis >> 16) & 0x0000ffff0000ffff;
    pop_high = (popcount >> 16) & 0x0000ffff0000ffff;
    sumxaxis = (sumxaxis & 0x0000ffff0000ffff) + sum_high;
    sumxaxis += pop_high << 4;
    popcount = (popcount & 0x0000ffff0000ffff) + pop_high;

    //  32 -> 64
    sum_high = sumxaxis >> 32;
    pop_high = popcount >> 32;
    sumxaxis += sum_high;
    sumxaxis += pop_high << 5;
    popcount += pop_high;

    pop_count = (uint32_t)popcount;
    sum_index = (uint32_t)sumxaxis;
}

static PA_FORCE_INLINE Word bit_reverse(Word x){
    uint64_t r0, r1;

#if 0
#elif __GNUC__
    r0 = __builtin_bswap64(x);
#elif __INTEL_COMPILER
    r0 = _bswap64(x);
#elif _MSC_VER
    r0 = _byteswap_uint64(x);
#else
#error "No byte-swap for this compiler."
#endif

    r1 = r0 >> 4;
    r0 = r0 << 4;
    r1 &= 0x0f0f0f0f0f0f0f0full;
    r0 &= 0xf0f0f0f0f0f0f0f0ull;
    r0 |= r1;

    r1 = r0 >> 2;
    r0 = r0 << 2;
    r1 &= 0x3333333333333333ull;
    r0 &= 0xccccccccccccccccull;
    r0 |= r1;

    r1 = r0 >> 1;
    r0 = r0 << 1;
    r1 &= 0x5555555555555555ull;
    r0 &= 0xaaaaaaaaaaaaaaaaull;
    r0 |= r1;

    return r0;
}

static PA_FORCE_INLINE bool expand(Word& x, Word& m){
    uint64_t m0 = m;
    uint64_t x0 = x & m0;
    if (x == 0){
        x = x0;
        return false;
    }

    uint64_t m1 = bit_reverse(m0);
    uint64_t x1 = bit_reverse(x0);

    uint64_t s0 = x0 + m0;
    uint64_t s1 = x1 + m1;
    s0 ^= m0;
    s1 ^= m1;
    s0 &= m0;
    s1 &= m1;
    s0 |= x0;
    s1 |= x1;

    x0 = s0 | bit_reverse(s1);
    m0 &= ~x0;

    x = x0;
    m = m0;
    return true;
}






};
}
}
}
#endif
