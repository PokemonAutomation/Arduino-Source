/*  Waterfill Intrinsics (x64 AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Intrinsics_x64_AVX512_H
#define PokemonAutomation_Kernels_Waterfill_Intrinsics_x64_AVX512_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels/Kernels_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
namespace Intrinsics_x64_AVX512{


PA_FORCE_INLINE __m512i popcount_indexsum(__m512i& sum_index, __m512i x){
    //  1 -> 2
    __m512i sum_high;
    __m512i pop_high = _mm512_and_si512(_mm512_srli_epi16(x, 1), _mm512_set1_epi8(0x55));
    __m512i sumxaxis = pop_high;
    __m512i popcount = _mm512_add_epi8(_mm512_and_si512(x, _mm512_set1_epi8(0x55)), pop_high);

    //  2 -> 4
    sum_high = _mm512_and_si512(_mm512_srli_epi16(sumxaxis, 2), _mm512_set1_epi8(0x33));
    pop_high = _mm512_and_si512(_mm512_srli_epi16(popcount, 2), _mm512_set1_epi8(0x33));
    sumxaxis = _mm512_add_epi8(_mm512_and_si512(sumxaxis, _mm512_set1_epi8(0x33)), sum_high);
    sumxaxis = _mm512_add_epi8(sumxaxis, _mm512_slli_epi16(pop_high, 1));
    popcount = _mm512_add_epi8(_mm512_and_si512(popcount, _mm512_set1_epi8(0x33)), pop_high);

    //  4 -> 8
    sum_high = _mm512_and_si512(_mm512_srli_epi16(sumxaxis, 4), _mm512_set1_epi8(0x0f));
    pop_high = _mm512_and_si512(_mm512_srli_epi16(popcount, 4), _mm512_set1_epi8(0x0f));
    sumxaxis = _mm512_add_epi8(_mm512_and_si512(sumxaxis, _mm512_set1_epi8(0x0f)), sum_high);
    sumxaxis = _mm512_add_epi8(sumxaxis, _mm512_slli_epi16(pop_high, 2));
    popcount = _mm512_add_epi8(_mm512_and_si512(popcount, _mm512_set1_epi8(0x0f)), pop_high);

    //  8 -> 16
    sum_high = _mm512_srli_epi16(sumxaxis, 8);
    pop_high = _mm512_srli_epi16(popcount, 8);
    sumxaxis = _mm512_add_epi16(_mm512_and_si512(sumxaxis, _mm512_set1_epi16(0x00ff)), sum_high);
    sumxaxis = _mm512_add_epi16(sumxaxis, _mm512_slli_epi16(pop_high, 3));
    popcount = _mm512_add_epi16(_mm512_and_si512(popcount, _mm512_set1_epi16(0x00ff)), pop_high);

    //  16 -> 32
    sum_high = _mm512_srli_epi32(sumxaxis, 16);
    pop_high = _mm512_srli_epi32(popcount, 16);
    sumxaxis = _mm512_add_epi32(sumxaxis, sum_high);
    sumxaxis = _mm512_add_epi32(sumxaxis, _mm512_slli_epi32(pop_high, 4));
    popcount = _mm512_add_epi32(popcount, pop_high);

    //  32 -> 64
    sum_high = _mm512_srli_epi64(sumxaxis, 32);
    pop_high = _mm512_srli_epi64(popcount, 32);
    sumxaxis = _mm512_add_epi64(sumxaxis, sum_high);
    sumxaxis = _mm512_add_epi64(sumxaxis, _mm512_slli_epi64(pop_high, 5));
    popcount = _mm512_add_epi64(popcount, pop_high);

    sum_index = _mm512_and_si512(sumxaxis, _mm512_set1_epi64(0x000000000000ffff));
    return _mm512_and_si512(popcount, _mm512_set1_epi64(0x000000000000ffff));
}



PA_FORCE_INLINE __m512i bit_reverse(__m512i x){
    x = _mm512_shuffle_epi8(
        x,
        _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );

    __m512i r0, r1;
    r0 = _mm512_srli_epi32(x, 4);
    r1 = _mm512_slli_epi32(x, 4);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x0f), 0b11011000);

    r0 = _mm512_srli_epi32(r1, 2);
    r1 = _mm512_slli_epi32(r1, 2);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x33), 0b11011000);

    r0 = _mm512_srli_epi32(r1, 1);
    r1 = _mm512_slli_epi32(r1, 1);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x55), 0b11011000);

    return r1;
}


template <typename ProcessedMask>
PA_FORCE_INLINE void expand_forward(
    const ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3
){
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.m0), mask.m0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.m1), mask.m1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.m2), mask.m2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.m3), mask.m3, 0b11110010);
}
template <typename ProcessedMask>
PA_FORCE_INLINE void expand_forward(
    const ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.m0), mask.m0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.m1), mask.m1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.m2), mask.m2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.m3), mask.m3, 0b11110010);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_add_epi64(x4, mask.m4), mask.m4, 0b11110010);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_add_epi64(x5, mask.m5), mask.m5, 0b11110010);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_add_epi64(x6, mask.m6), mask.m6, 0b11110010);
    x7 = _mm512_ternarylogic_epi64(x7, _mm512_add_epi64(x7, mask.m7), mask.m7, 0b11110010);
}


template <typename ProcessedMask>
PA_FORCE_INLINE void expand_reverse(
    const ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3
){
    x0 = bit_reverse(x0);
    x1 = bit_reverse(x1);
    x2 = bit_reverse(x2);
    x3 = bit_reverse(x3);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.b0), mask.b0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.b1), mask.b1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.b2), mask.b2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.b3), mask.b3, 0b11110010);
    x0 = bit_reverse(x0);
    x1 = bit_reverse(x1);
    x2 = bit_reverse(x2);
    x3 = bit_reverse(x3);
}
template <typename ProcessedMask>
PA_FORCE_INLINE void expand_reverse(
    const ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    x0 = bit_reverse(x0);
    x1 = bit_reverse(x1);
    x2 = bit_reverse(x2);
    x3 = bit_reverse(x3);
    x4 = bit_reverse(x4);
    x5 = bit_reverse(x5);
    x6 = bit_reverse(x6);
    x7 = bit_reverse(x7);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.b0), mask.b0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.b1), mask.b1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.b2), mask.b2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.b3), mask.b3, 0b11110010);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_add_epi64(x4, mask.b4), mask.b4, 0b11110010);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_add_epi64(x5, mask.b5), mask.b5, 0b11110010);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_add_epi64(x6, mask.b6), mask.b6, 0b11110010);
    x7 = _mm512_ternarylogic_epi64(x7, _mm512_add_epi64(x7, mask.b7), mask.b7, 0b11110010);
    x0 = bit_reverse(x0);
    x1 = bit_reverse(x1);
    x2 = bit_reverse(x2);
    x3 = bit_reverse(x3);
    x4 = bit_reverse(x4);
    x5 = bit_reverse(x5);
    x6 = bit_reverse(x6);
    x7 = bit_reverse(x7);
}




}
}
}
}
#endif
