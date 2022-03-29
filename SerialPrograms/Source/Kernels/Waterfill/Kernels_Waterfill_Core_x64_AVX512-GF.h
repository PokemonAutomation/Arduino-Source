/*  Waterfill Core (AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512GF_H
#define PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512GF_H

#include "Kernels_Waterfill_Core_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


template <> PA_FORCE_INLINE __m512i bit_reverse<true>(__m512i x){
    x = _mm512_shuffle_epi8(
        x,
        _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );
    return _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);
}


PA_FORCE_INLINE __m512i transpose_forward(__m512i x){
    const __m512i INDEX = _mm512_setr_epi8(
        56, 48, 40, 32, 24, 16,  8,  0,
        57, 49, 41, 33, 25, 17,  9,  1,
        58, 50, 42, 34, 26, 18, 10,  2,
        59, 51, 43, 35, 27, 19, 11,  3,
        60, 52, 44, 36, 28, 20, 12,  4,
        61, 53, 45, 37, 29, 21, 13,  5,
        62, 54, 46, 38, 30, 22, 14,  6,
        63, 55, 47, 39, 31, 23, 15,  7
    );
    x = _mm512_permutexvar_epi8(INDEX, x);
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
    return x;
}
PA_FORCE_INLINE __m512i transpose_inverse(__m512i x){
    const __m512i INDEX = _mm512_setr_epi8(
         7, 15, 23, 31, 39, 47, 55, 63,
         6, 14, 22, 30, 38, 46, 54, 62,
         5, 13, 21, 29, 37, 45, 53, 61,
         4, 12, 20, 28, 36, 44, 52, 60,
         3, 11, 19, 27, 35, 43, 51, 59,
         2, 10, 18, 26, 34, 42, 50, 58,
         1,  9, 17, 25, 33, 41, 49, 57,
         0,  8, 16, 24, 32, 40, 48, 56
    );
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x0102040810204080), x, 0);
    x = _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);
    x = _mm512_permutexvar_epi8(INDEX, x);
    return x;
}
PA_FORCE_INLINE __m512i transpose_inverse_reversed(__m512i x){
    const __m512i INDEX = _mm512_setr_epi8(
         7, 15, 23, 31, 39, 47, 55, 63,
         6, 14, 22, 30, 38, 46, 54, 62,
         5, 13, 21, 29, 37, 45, 53, 61,
         4, 12, 20, 28, 36, 44, 52, 60,
         3, 11, 19, 27, 35, 43, 51, 59,
         2, 10, 18, 26, 34, 42, 50, 58,
         1,  9, 17, 25, 33, 41, 49, 57,
         0,  8, 16, 24, 32, 40, 48, 56
    );
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
    x = _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);
    x = _mm512_permutexvar_epi8(INDEX, x);
    return x;
}
PA_FORCE_INLINE __m512i transpose_1x8x8x8(__m512i x){
    const __m512i INDEX0 = _mm512_setr_epi8(
        56, 48, 40, 32, 24, 16,  8,  0,
        57, 49, 41, 33, 25, 17,  9,  1,
        58, 50, 42, 34, 26, 18, 10,  2,
        59, 51, 43, 35, 27, 19, 11,  3,
        60, 52, 44, 36, 28, 20, 12,  4,
        61, 53, 45, 37, 29, 21, 13,  5,
        62, 54, 46, 38, 30, 22, 14,  6,
        63, 55, 47, 39, 31, 23, 15,  7
    );
    const __m512i INDEX1 = _mm512_setr_epi8(
         0,  8, 16, 24, 32, 40, 48, 56,
         1,  9, 17, 25, 33, 41, 49, 57,
         2, 10, 18, 26, 34, 42, 50, 58,
         3, 11, 19, 27, 35, 43, 51, 59,
         4, 12, 20, 28, 36, 44, 52, 60,
         5, 13, 21, 29, 37, 45, 53, 61,
         6, 14, 22, 30, 38, 46, 54, 62,
         7, 15, 23, 31, 39, 47, 55, 63
    );
    x = _mm512_permutexvar_epi8(INDEX0, x);
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
    x = _mm512_permutexvar_epi8(INDEX1, x);
    return x;
}
PA_FORCE_INLINE __m512i transpose_1x8x8x8_bitreverse_in(__m512i x){
    const __m512i INDEX0 = _mm512_setr_epi8(
        63, 55, 47, 39, 31, 23, 15,  7,
        62, 54, 46, 38, 30, 22, 14,  6,
        61, 53, 45, 37, 29, 21, 13,  5,
        60, 52, 44, 36, 28, 20, 12,  4,
        59, 51, 43, 35, 27, 19, 11,  3,
        58, 50, 42, 34, 26, 18, 10,  2,
        57, 49, 41, 33, 25, 17,  9,  1,
        56, 48, 40, 32, 24, 16,  8,  0
    );
    const __m512i INDEX1 = _mm512_setr_epi8(
         7, 15, 23, 31, 39, 47, 55, 63,
         6, 14, 22, 30, 38, 46, 54, 62,
         5, 13, 21, 29, 37, 45, 53, 61,
         4, 12, 20, 28, 36, 44, 52, 60,
         3, 11, 19, 27, 35, 43, 51, 59,
         2, 10, 18, 26, 34, 42, 50, 58,
         1,  9, 17, 25, 33, 41, 49, 57,
         0,  8, 16, 24, 32, 40, 48, 56
    );
    x = _mm512_permutexvar_epi8(INDEX0, x);
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
    x = _mm512_permutexvar_epi8(INDEX1, x);
    return x;
}
PA_FORCE_INLINE void transpose_1x16x16x4(__m512i& r0, __m512i& r1){
    const __m512i INDEX = _mm512_setr_epi8(
        56, 48, 40, 32, 24, 16,  8,  0,
        57, 49, 41, 33, 25, 17,  9,  1,
        58, 50, 42, 34, 26, 18, 10,  2,
        59, 51, 43, 35, 27, 19, 11,  3,
        60, 52, 44, 36, 28, 20, 12,  4,
        61, 53, 45, 37, 29, 21, 13,  5,
        62, 54, 46, 38, 30, 22, 14,  6,
        63, 55, 47, 39, 31, 23, 15,  7
    );
    __m512i s0, s1;
    r0 = _mm512_permutexvar_epi8(INDEX, r0);
    r1 = _mm512_permutexvar_epi8(INDEX, r1);
    s0 = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), r0, 0);
    s1 = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), r1, 0);
    const __m512i INDEX0 = _mm512_setr_epi8(
         0, 64, 16, 80, 32, 96, 48,112,
         1, 65, 17, 81, 33, 97, 49,113,
         2, 66, 18, 82, 34, 98, 50,114,
         3, 67, 19, 83, 35, 99, 51,115,
         4, 68, 20, 84, 36,100, 52,116,
         5, 69, 21, 85, 37,101, 53,117,
         6, 70, 22, 86, 38,102, 54,118,
         7, 71, 23, 87, 39,103, 55,119
    );
    const __m512i INDEX1 = _mm512_setr_epi8(
         8, 72, 24, 88, 40,104, 56,120,
         9, 73, 25, 89, 41,105, 57,121,
        10, 74, 26, 90, 42,106, 58,122,
        11, 75, 27, 91, 43,107, 59,123,
        12, 76, 28, 92, 44,108, 60,124,
        13, 77, 29, 93, 45,109, 61,125,
        14, 78, 30, 94, 46,110, 62,126,
        15, 79, 31, 95, 47,111, 63,127
    );
    r0 = _mm512_permutex2var_epi8(s0, INDEX0, s1);
    r1 = _mm512_permutex2var_epi8(s0, INDEX1, s1);
}
PA_FORCE_INLINE void transpose_1x16x16x4_bitreverse_in(__m512i& r0, __m512i& r1){
    const __m512i INDEX = _mm512_setr_epi8(
        63, 55, 47, 39, 31, 23, 15,  7,
        62, 54, 46, 38, 30, 22, 14,  6,
        61, 53, 45, 37, 29, 21, 13,  5,
        60, 52, 44, 36, 28, 20, 12,  4,
        59, 51, 43, 35, 27, 19, 11,  3,
        58, 50, 42, 34, 26, 18, 10,  2,
        57, 49, 41, 33, 25, 17,  9,  1,
        56, 48, 40, 32, 24, 16,  8,  0
    );
    __m512i s0, s1;
    r0 = _mm512_permutexvar_epi8(INDEX, r0);
    r1 = _mm512_permutexvar_epi8(INDEX, r1);
    s0 = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), r0, 0);
    s1 = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), r1, 0);
    const __m512i INDEX0 = _mm512_setr_epi8(
         7, 71, 23, 87, 39,103, 55,119,
         6, 70, 22, 86, 38,102, 54,118,
         5, 69, 21, 85, 37,101, 53,117,
         4, 68, 20, 84, 36,100, 52,116,
         3, 67, 19, 83, 35, 99, 51,115,
         2, 66, 18, 82, 34, 98, 50,114,
         1, 65, 17, 81, 33, 97, 49,113,
         0, 64, 16, 80, 32, 96, 48,112
    );
    const __m512i INDEX1 = _mm512_setr_epi8(
        15, 79, 31, 95, 47,111, 63,127,
        14, 78, 30, 94, 46,110, 62,126,
        13, 77, 29, 93, 45,109, 61,125,
        12, 76, 28, 92, 44,108, 60,124,
        11, 75, 27, 91, 43,107, 59,123,
        10, 74, 26, 90, 42,106, 58,122,
         9, 73, 25, 89, 41,105, 57,121,
         8, 72, 24, 88, 40,104, 56,120
    );
    r0 = _mm512_permutex2var_epi8(s0, INDEX0, s1);
    r1 = _mm512_permutex2var_epi8(s0, INDEX1, s1);
}
PA_FORCE_INLINE void transpose_16x2x2x2(__m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3){
    const __m512i INDEX0 = _mm512_setr_epi16(
         0, 32,  2, 34,
         4, 36,  6, 38,
         8, 40, 10, 42,
        12, 44, 14, 46,
        16, 48, 18, 50,
        20, 52, 22, 54,
        24, 56, 26, 58,
        28, 60, 30, 62
    );
    const __m512i INDEX1 = _mm512_setr_epi16(
         1, 33,  3, 35,
         5, 37,  7, 39,
         9, 41, 11, 43,
        13, 45, 15, 47,
        17, 49, 19, 51,
        21, 53, 23, 55,
        25, 57, 27, 59,
        29, 61, 31, 63
    );
    __m512i s0, s1, s2, s3;
    s0 = r0;
    s1 = r1;
    s2 = r2;
    s3 = r3;
    r0 = _mm512_permutex2var_epi16(s0, INDEX0, s2);
    r1 = _mm512_permutex2var_epi16(s1, INDEX0, s3);
    r2 = _mm512_permutex2var_epi16(s0, INDEX1, s2);
    r3 = _mm512_permutex2var_epi16(s1, INDEX1, s3);
}
PA_FORCE_INLINE void transpose_32x2x2(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3,
    __m512i& r4, __m512i& r5, __m512i& r6, __m512i& r7
){
    const __m512i INDEX0 = _mm512_setr_epi32(
         0, 16,
         2, 18,
         4, 20,
         6, 22,
         8, 24,
        10, 26,
        12, 28,
        14, 30
    );
    const __m512i INDEX1 = _mm512_setr_epi32(
         1, 17,
         3, 19,
         5, 21,
         7, 23,
         9, 25,
        11, 27,
        13, 29,
        15, 31
    );
    __m512i s0, s1, s2, s3, s4, s5, s6, s7;
    s0 = r0;
    s1 = r1;
    s2 = r2;
    s3 = r3;
    s4 = r4;
    s5 = r5;
    s6 = r6;
    s7 = r7;
    r0 = _mm512_permutex2var_epi32(s0, INDEX0, s4);
    r1 = _mm512_permutex2var_epi32(s1, INDEX0, s5);
    r2 = _mm512_permutex2var_epi32(s2, INDEX0, s6);
    r3 = _mm512_permutex2var_epi32(s3, INDEX0, s7);
    r4 = _mm512_permutex2var_epi32(s0, INDEX1, s4);
    r5 = _mm512_permutex2var_epi32(s1, INDEX1, s5);
    r6 = _mm512_permutex2var_epi32(s2, INDEX1, s6);
    r7 = _mm512_permutex2var_epi32(s3, INDEX1, s7);
}
PA_FORCE_INLINE void transpose_1x64x64(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3,
    __m512i& r4, __m512i& r5, __m512i& r6, __m512i& r7
){
    transpose_1x16x16x4(r0, r1);
    transpose_1x16x16x4(r2, r3);
    transpose_1x16x16x4(r4, r5);
    transpose_1x16x16x4(r6, r7);
    transpose_16x2x2x2(r0, r1, r2, r3);
    transpose_16x2x2x2(r4, r5, r6, r7);
    transpose_32x2x2(r0, r1, r2, r3, r4, r5, r6, r7);
}


template <>
struct Waterfill_x64_AVX512_ProcessedMask<true>{
    __m512i m0, m1, m2, m3, m4, m5, m6, m7; //  Copy of the masks.
    __m512i b0, b1, b2, b3, b4, b5, b6, b7; //  Bit-reversed copy of the masks.
    __m512i f0, f1, f2, f3, f4, f5, f6, f7; //  Forward transposed.
    __m512i r0, r1, r2, r3, r4, r5, r6, r7; //  Reverse transposed.

    PA_FORCE_INLINE Waterfill_x64_AVX512_ProcessedMask(
        const BinaryTile_AVX512& m,
        __m512i x0, __m512i x1, __m512i x2, __m512i x3,
        __m512i x4, __m512i x5, __m512i x6, __m512i x7
    ){
        m0 = _mm512_or_si512(x0, m.vec[0]);
        m1 = _mm512_or_si512(x1, m.vec[1]);
        m2 = _mm512_or_si512(x2, m.vec[2]);
        m3 = _mm512_or_si512(x3, m.vec[3]);
        m4 = _mm512_or_si512(x4, m.vec[4]);
        m5 = _mm512_or_si512(x5, m.vec[5]);
        m6 = _mm512_or_si512(x6, m.vec[6]);
        m7 = _mm512_or_si512(x7, m.vec[7]);

        b0 = bit_reverse<true>(m0);
        b1 = bit_reverse<true>(m1);
        b2 = bit_reverse<true>(m2);
        b3 = bit_reverse<true>(m3);
        b4 = bit_reverse<true>(m4);
        b5 = bit_reverse<true>(m5);
        b6 = bit_reverse<true>(m6);
        b7 = bit_reverse<true>(m7);

        f0 = m0;
        f1 = m1;
        f2 = m2;
        f3 = m3;
        f4 = m4;
        f5 = m5;
        f6 = m6;
        f7 = m7;
        transpose_1x64x64(f0, f1, f2, f3, f4, f5, f6, f7);
        r0 = bit_reverse<true>(f0);
        r1 = bit_reverse<true>(f1);
        r2 = bit_reverse<true>(f2);
        r3 = bit_reverse<true>(f3);
        r4 = bit_reverse<true>(f4);
        r5 = bit_reverse<true>(f5);
        r6 = bit_reverse<true>(f6);
        r7 = bit_reverse<true>(f7);
    }
};



template <>
PA_FORCE_INLINE void expand_vertical<true>(
    const Waterfill_x64_AVX512_ProcessedMask<true>& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    transpose_1x64x64(x0, x1, x2, x3, x4, x5, x6, x7);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.f0), mask.f0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.f1), mask.f1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.f2), mask.f2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.f3), mask.f3, 0b11110010);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_add_epi64(x4, mask.f4), mask.f4, 0b11110010);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_add_epi64(x5, mask.f5), mask.f5, 0b11110010);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_add_epi64(x6, mask.f6), mask.f6, 0b11110010);
    x7 = _mm512_ternarylogic_epi64(x7, _mm512_add_epi64(x7, mask.f7), mask.f7, 0b11110010);
    x0 = bit_reverse<true>(x0);
    x1 = bit_reverse<true>(x1);
    x2 = bit_reverse<true>(x2);
    x3 = bit_reverse<true>(x3);
    x4 = bit_reverse<true>(x4);
    x5 = bit_reverse<true>(x5);
    x6 = bit_reverse<true>(x6);
    x7 = bit_reverse<true>(x7);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.r0), mask.r0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.r1), mask.r1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.r2), mask.r2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.r3), mask.r3, 0b11110010);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_add_epi64(x4, mask.r4), mask.r4, 0b11110010);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_add_epi64(x5, mask.r5), mask.r5, 0b11110010);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_add_epi64(x6, mask.r6), mask.r6, 0b11110010);
    x7 = _mm512_ternarylogic_epi64(x7, _mm512_add_epi64(x7, mask.r7), mask.r7, 0b11110010);
    transpose_1x16x16x4_bitreverse_in(x0, x1);
    transpose_1x16x16x4_bitreverse_in(x2, x3);
    transpose_1x16x16x4_bitreverse_in(x4, x5);
    transpose_1x16x16x4_bitreverse_in(x6, x7);
    transpose_16x2x2x2(x0, x1, x2, x3);
    transpose_16x2x2x2(x4, x5, x6, x7);
    transpose_32x2x2(x0, x1, x2, x3, x4, x5, x6, x7);
}







struct Waterfill_x64_AVX512GF : public Waterfill_x64_AVX512{



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
PA_FORCE_INLINE static void waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x){
    Waterfill::waterfill_expand<true>(m, x);
}




};



}
}
}
#endif
