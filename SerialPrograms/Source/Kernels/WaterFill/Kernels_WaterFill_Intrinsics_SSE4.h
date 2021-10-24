/*  Water Fill Intrinsics (SSE4)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_WaterFill_Intrinsics_SSE4_H
#define PokemonAutomation_Kernels_WaterFill_Intrinsics_SSE4_H

#include <stdint.h>
#include <smmintrin.h>
#include "Common/Compiler.h"
#include "Kernels_WaterFill_Intrinsics_u64.h"

#if _MSC_VER
#include <intrin.h>
#endif

namespace PokemonAutomation{
namespace Kernels{
namespace WaterFill{
struct Intrinsics_SSE4{


using Word = __m128i;

static std::string to_str(Word x){
    std::string str;
    str += Intrinsics_u64::to_str(_mm_cvtsi128_si64(x));
    str += Intrinsics_u64::to_str(_mm_extract_epi64(x, 1));
    return str;
}

static PA_FORCE_INLINE Word LOW_BIT(){
    return _mm_setr_epi32(1, 0, 0, 0);
}
static PA_FORCE_INLINE Word HIGH_BIT(){
    return _mm_setr_epi32(0, 0, 0, 0x80000000);
}

static PA_FORCE_INLINE bool low_bit(Word x){
    return !_mm_test_all_zeros(x, LOW_BIT());
}
static PA_FORCE_INLINE bool high_bit(Word x){
    return !_mm_test_all_zeros(x, HIGH_BIT());
}

static PA_FORCE_INLINE size_t least_significant_bit(__m128i x){
    uint64_t lo = _mm_cvtsi128_si64(x);
    uint64_t hi = _mm_extract_epi64(x, 1);
    return lo != 0
        ? Intrinsics_u64::least_significant_bit(lo)
        : Intrinsics_u64::least_significant_bit(hi) + 64;
}
static PA_FORCE_INLINE size_t most_significant_bit(__m128i x){
    uint64_t lo = _mm_cvtsi128_si64(x);
    uint64_t hi = _mm_extract_epi64(x, 1);
    return hi == 0
        ? Intrinsics_u64::most_significant_bit(lo)
        : Intrinsics_u64::most_significant_bit(hi) + 64;
}

static PA_FORCE_INLINE void popcount_sumindex(
    uint64_t& pop_count, uint64_t& sum_index,
    Word x
){
    //  1 -> 2
    __m128i sum_high;
    __m128i pop_high = _mm_and_si128(_mm_srli_epi16(x, 1), _mm_set1_epi8(0x55));
    __m128i sumxaxis = pop_high;
    __m128i popcount = _mm_add_epi8(_mm_and_si128(x, _mm_set1_epi8(0x55)), pop_high);

    //  2 -> 4
    sum_high = _mm_and_si128(_mm_srli_epi16(sumxaxis, 2), _mm_set1_epi8(0x33));
    pop_high = _mm_and_si128(_mm_srli_epi16(popcount, 2), _mm_set1_epi8(0x33));
    sumxaxis = _mm_add_epi8(_mm_and_si128(sumxaxis, _mm_set1_epi8(0x33)), sum_high);
    sumxaxis = _mm_add_epi8(sumxaxis, _mm_slli_epi16(pop_high, 1));
    popcount = _mm_add_epi8(_mm_and_si128(popcount, _mm_set1_epi8(0x33)), pop_high);

    //  4 -> 8
    sum_high = _mm_and_si128(_mm_srli_epi16(sumxaxis, 4), _mm_set1_epi8(0x0f));
    pop_high = _mm_and_si128(_mm_srli_epi16(popcount, 4), _mm_set1_epi8(0x0f));
    sumxaxis = _mm_add_epi8(_mm_and_si128(sumxaxis, _mm_set1_epi8(0x0f)), sum_high);
    sumxaxis = _mm_add_epi8(sumxaxis, _mm_slli_epi16(pop_high, 2));
    popcount = _mm_add_epi8(_mm_and_si128(popcount, _mm_set1_epi8(0x0f)), pop_high);

    //  8 -> 16
    sum_high = _mm_srli_epi16(sumxaxis, 8);
    pop_high = _mm_srli_epi16(popcount, 8);
    sumxaxis = _mm_add_epi16(_mm_and_si128(sumxaxis, _mm_set1_epi16(0x00ff)), sum_high);
    sumxaxis = _mm_add_epi16(sumxaxis, _mm_slli_epi16(pop_high, 3));
    popcount = _mm_add_epi16(_mm_and_si128(popcount, _mm_set1_epi16(0x00ff)), pop_high);

    //  16 -> 32
    sum_high = _mm_srli_epi32(sumxaxis, 16);
    pop_high = _mm_srli_epi32(popcount, 16);
    sumxaxis = _mm_add_epi32(sumxaxis, sum_high);
    sumxaxis = _mm_add_epi32(sumxaxis, _mm_slli_epi32(pop_high, 4));
    popcount = _mm_add_epi32(popcount, pop_high);

    //  32 -> 64
    sum_high = _mm_srli_epi64(sumxaxis, 32);
    pop_high = _mm_srli_epi64(popcount, 32);
    sumxaxis = _mm_add_epi64(sumxaxis, sum_high);
    sumxaxis = _mm_add_epi64(sumxaxis, _mm_slli_epi64(pop_high, 5));
    popcount = _mm_add_epi64(popcount, pop_high);

    //  64 -> 128
    uint64_t sum = _mm_extract_epi64(sumxaxis, 1);
    uint64_t pop = _mm_extract_epi64(popcount, 1);
    sum = _mm_cvtsi128_si64(sumxaxis) + sum;
    sum += pop << 6;
    pop = _mm_cvtsi128_si64(popcount) + pop;

    pop_count = sum & 0xffff;
    sum_index = pop & 0xffff;
}

static PA_FORCE_INLINE Word bit_reverse(Word x){
    __m128i r0, r1;

    x = _mm_shuffle_epi8(x, _mm_setr_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

    r0 = _mm_srli_epi32(x, 4);
    r1 = _mm_slli_epi32(x, 4);
    r0 = _mm_and_si128(r0, _mm_set1_epi8((uint8_t)0x0f));
    r1 = _mm_and_si128(r1, _mm_set1_epi8((uint8_t)0xf0));
    r1 = _mm_or_si128(r0, r1);

    r0 = _mm_srli_epi32(r1, 2);
    r1 = _mm_slli_epi32(r1, 2);
    r0 = _mm_and_si128(r0, _mm_set1_epi8((uint8_t)0x33));
    r1 = _mm_and_si128(r1, _mm_set1_epi8((uint8_t)0xcc));
    r1 = _mm_or_si128(r0, r1);

    r0 = _mm_srli_epi32(r1, 1);
    r1 = _mm_slli_epi32(r1, 1);
    r0 = _mm_and_si128(r0, _mm_set1_epi8((uint8_t)0x55));
    r1 = _mm_and_si128(r1, _mm_set1_epi8((uint8_t)0xaa));
    r1 = _mm_or_si128(r0, r1);

    return r1;
}

static PA_FORCE_INLINE Word add(Word x, Word y){
    __m128i s = _mm_add_epi64(x, y);
    __m128i c = _mm_cmpgt_epi64(
        _mm_xor_si128(x, _mm_set1_epi64x(0x8000000000000000)),
        _mm_xor_si128(s, _mm_set1_epi64x(0x8000000000000000))
    );
    c = _mm_unpacklo_epi64(_mm_setzero_si128(), c);
    s = _mm_sub_epi64(s, c);
    return s;
}

static PA_FORCE_INLINE bool expand(Word& x, Word& m){
    __m128i m0 = m;
    __m128i x0 = _mm_and_si128(x, m0);
    if (_mm_test_all_zeros(x, x)){
        x = x0;
        return false;
    }

    __m128i m1 = bit_reverse(m0);
    __m128i x1 = bit_reverse(x0);

    __m128i s0 = add(x0, m0);
    __m128i s1 = add(x1, m1);
    s0 = _mm_xor_si128(s0, m0);
    s1 = _mm_xor_si128(s1, m1);
    s0 = _mm_and_si128(s0, m0);
    s1 = _mm_and_si128(s1, m1);
    s0 = _mm_or_si128(s0, x0);
    s1 = _mm_or_si128(s1, x1);

    s1 = bit_reverse(s1);
    x0 = _mm_or_si128(s0, s1);
    m0 = _mm_andnot_si128(x0, m0);

    x = x0;
    m = m0;
    return true;
}





};
}
}
}
#endif
