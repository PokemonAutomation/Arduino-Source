/*  Waterfill Intrinsics (x64 AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/
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



PA_FORCE_INLINE void transpose_64x8x4_forward(
    __m512i z0, __m512i z1, __m512i z2, __m512i z3,
    __m256i& y0, __m256i& y1, __m256i& y2, __m256i& y3, __m256i& y4, __m256i& y5, __m256i& y6, __m256i& y7
){
    __m512i s0, s1, s2, s3;
    s0 = _mm512_unpacklo_epi64(z0, z1);
    s1 = _mm512_unpackhi_epi64(z0, z1);
    s2 = _mm512_unpacklo_epi64(z2, z3);
    s3 = _mm512_unpackhi_epi64(z2, z3);
    z0 = _mm512_mask_permutex_epi64(s0, 0xcc, s2, 78);
    z1 = _mm512_mask_permutex_epi64(s1, 0xcc, s3, 78);
    z2 = _mm512_mask_permutex_epi64(s2, 0x33, s0, 78);
    z3 = _mm512_mask_permutex_epi64(s3, 0x33, s1, 78);
    y0 = _mm512_castsi512_si256(z0);
    y1 = _mm512_castsi512_si256(z1);
    y2 = _mm512_castsi512_si256(z2);
    y3 = _mm512_castsi512_si256(z3);
    y4 = _mm512_extracti64x4_epi64(z0, 1);
    y5 = _mm512_extracti64x4_epi64(z1, 1);
    y6 = _mm512_extracti64x4_epi64(z2, 1);
    y7 = _mm512_extracti64x4_epi64(z3, 1);
}
PA_FORCE_INLINE void transpose_64x8x4_inverse(
    __m512i& z0, __m512i& z1, __m512i& z2, __m512i& z3,
    __m256i y0, __m256i y1, __m256i y2, __m256i y3, __m256i y4, __m256i y5, __m256i y6, __m256i y7
){
    __m512i s0, s1, s2, s3;
    z0 = _mm512_inserti64x4(_mm512_castsi256_si512(y0), y4, 1);
    z1 = _mm512_inserti64x4(_mm512_castsi256_si512(y1), y5, 1);
    z2 = _mm512_inserti64x4(_mm512_castsi256_si512(y2), y6, 1);
    z3 = _mm512_inserti64x4(_mm512_castsi256_si512(y3), y7, 1);
    s0 = _mm512_mask_permutex_epi64(z0, 0xcc, z2, 78);
    s1 = _mm512_mask_permutex_epi64(z1, 0xcc, z3, 78);
    s2 = _mm512_mask_permutex_epi64(z2, 0x33, z0, 78);
    s3 = _mm512_mask_permutex_epi64(z3, 0x33, z1, 78);
    z0 = _mm512_unpacklo_epi64(s0, s1);
    z1 = _mm512_unpackhi_epi64(s0, s1);
    z2 = _mm512_unpacklo_epi64(s2, s3);
    z3 = _mm512_unpackhi_epi64(s2, s3);
}



PA_FORCE_INLINE __m512i transpose_1x8x8x8(__m512i x){
    __m512i r, L, H;

//    r = _mm512_shuffle_epi32(x, 78);
    r = _mm512_shuffle_epi32(x, _MM_PERM_BADC);
    L = _mm512_slli_epi64(r, 1);
    H = _mm512_srli_epi64(r, 1);
    r = _mm512_ternarylogic_epi64(L, H, _mm512_set1_epi8(0x55), 0xd8);
    x = _mm512_ternarylogic_epi64(x, r, _mm512_setr_epi64(0xaaaaaaaaaaaaaaaa, 0x5555555555555555, 0xaaaaaaaaaaaaaaaa, 0x5555555555555555, 0xaaaaaaaaaaaaaaaa, 0x5555555555555555, 0xaaaaaaaaaaaaaaaa, 0x5555555555555555), 0xd8);

    r = _mm512_shuffle_i64x2(x, x, 177);
    L = _mm512_slli_epi64(r, 2);
    H = _mm512_srli_epi64(r, 2);
    r = _mm512_ternarylogic_epi64(L, H, _mm512_set1_epi8(0x33), 0xd8);
    x = _mm512_ternarylogic_epi64(x, r, _mm512_setr_epi64(0xcccccccccccccccc, 0xcccccccccccccccc, 0x3333333333333333, 0x3333333333333333, 0xcccccccccccccccc, 0xcccccccccccccccc, 0x3333333333333333, 0x3333333333333333), 0xd8);

    r = _mm512_shuffle_i64x2(x, x, 78);
    L = _mm512_slli_epi64(r, 4);
    H = _mm512_srli_epi64(r, 4);
    r = _mm512_ternarylogic_epi64(L, H, _mm512_set1_epi8(0x0f), 0xd8);
    x = _mm512_ternarylogic_epi64(x, r, _mm512_setr_epi64(0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f), 0xd8);

    return x;
}
PA_FORCE_INLINE __m512i transpose_1x8x8x8_bitreverse_in(__m512i x){
    const __m512i INDEX = _mm512_setr_epi8(
        7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
        7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
        7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
        7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
    );
    x = _mm512_shuffle_epi8(x, INDEX);
    x = transpose_1x8x8x8(x);
    x = _mm512_permutexvar_epi64(_mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0), x);
    return x;
}

PA_FORCE_INLINE void transpose_8x2x2x4(__m512i& r0, __m512i& r1){
    __m512i s0 = r0;
    r0 = _mm512_mask_alignr_epi8(r0, 0xaaaaaaaaaaaaaaaa, r1, r1, 15);
    r1 = _mm512_mask_alignr_epi8(r1, 0x5555555555555555, s0, s0, 1);
}

PA_FORCE_INLINE void transpose_16x2x2x2(__m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3){
    __m512i s0, s1;
    s0 = r0;
    s1 = r1;
    r0 = _mm512_mask_alignr_epi8(s0, 0xcccccccccccccccc, r2, r2, 14);
    r1 = _mm512_mask_alignr_epi8(s1, 0xcccccccccccccccc, r3, r3, 14);
    r2 = _mm512_mask_alignr_epi8(r2, 0x3333333333333333, s0, s0, 2);
    r3 = _mm512_mask_alignr_epi8(r3, 0x3333333333333333, s1, s1, 2);
}

PA_FORCE_INLINE void transpose_1x64x32(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3
){
    r0 = transpose_1x8x8x8(r0);
    r1 = transpose_1x8x8x8(r1);
    r2 = transpose_1x8x8x8(r2);
    r3 = transpose_1x8x8x8(r3);
    transpose_8x2x2x4(r0, r1);
    transpose_8x2x2x4(r2, r3);
    transpose_16x2x2x2(r0, r1, r2, r3);
}
PA_FORCE_INLINE void transpose_1x64x32_bitreverse_in(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3
){
    r0 = transpose_1x8x8x8_bitreverse_in(r0);
    r1 = transpose_1x8x8x8_bitreverse_in(r1);
    r2 = transpose_1x8x8x8_bitreverse_in(r2);
    r3 = transpose_1x8x8x8_bitreverse_in(r3);
    transpose_8x2x2x4(r0, r1);
    transpose_8x2x2x4(r2, r3);
    transpose_16x2x2x2(r0, r1, r2, r3);
}



PA_FORCE_INLINE void has_neighbors(__m512i& t, __m512i x, __m512i m, __m512i mH){
    m = _mm512_ternarylogic_epi64(
        _mm512_slli_epi64(m, 1),
        _mm512_alignr_epi64(m, _mm512_setzero_si512(), 7),
        _mm512_alignr_epi64(mH, m, 1),
        254
    );
    t = _mm512_and_si512(m, x);
}
PA_FORCE_INLINE void has_neighbors(__m512i& t, __m512i x, __m512i mL, __m512i m, __m512i mH){
    m = _mm512_ternarylogic_epi64(
        _mm512_slli_epi64(m, 1),
        _mm512_alignr_epi64(m, mL, 7),
        _mm512_alignr_epi64(mH, m, 1),
        254
    );
//    t = _mm512_or_si512(t, _mm512_and_si512(m, x));
    t = _mm512_ternarylogic_epi64(t, m, x, 0b11111000);
}
template <typename ProcessedMask>
PA_FORCE_INLINE bool keep_going(
    const ProcessedMask& mask,
    __m512i& m0, __m512i& m1, __m512i& m2, __m512i& m3,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3
){
    m0 = _mm512_andnot_si512(x0, mask.m0);
    m1 = _mm512_andnot_si512(x1, mask.m1);
    m2 = _mm512_andnot_si512(x2, mask.m2);
    m3 = _mm512_andnot_si512(x3, mask.m3);
    __m512i changed;
    has_neighbors(changed, x0, m0, m1);
    has_neighbors(changed, x1, m0, m1, m2);
    has_neighbors(changed, x2, m1, m2, m3);
    has_neighbors(changed, x3, m2, m3, _mm512_setzero_si512());
    return _mm512_test_epi64_mask(changed, changed);
}
template <typename ProcessedMask>
PA_FORCE_INLINE bool keep_going(
    const ProcessedMask& mask,
    __m512i& m0, __m512i& m1, __m512i& m2, __m512i& m3, __m512i& m4, __m512i& m5, __m512i& m6, __m512i& m7,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3, __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    m0 = _mm512_andnot_si512(x0, mask.m0);
    m1 = _mm512_andnot_si512(x1, mask.m1);
    m2 = _mm512_andnot_si512(x2, mask.m2);
    m3 = _mm512_andnot_si512(x3, mask.m3);
    m4 = _mm512_andnot_si512(x4, mask.m4);
    m5 = _mm512_andnot_si512(x5, mask.m5);
    m6 = _mm512_andnot_si512(x6, mask.m6);
    m7 = _mm512_andnot_si512(x7, mask.m7);
    __m512i changed;
    has_neighbors(changed, x0, m0, m1);
    has_neighbors(changed, x1, m0, m1, m2);
    has_neighbors(changed, x2, m1, m2, m3);
    has_neighbors(changed, x3, m2, m3, m4);
    has_neighbors(changed, x4, m3, m4, m5);
    has_neighbors(changed, x5, m4, m5, m6);
    has_neighbors(changed, x6, m5, m6, m7);
    has_neighbors(changed, x7, m6, m7, _mm512_setzero_si512());
    return _mm512_test_epi64_mask(changed, changed);
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
