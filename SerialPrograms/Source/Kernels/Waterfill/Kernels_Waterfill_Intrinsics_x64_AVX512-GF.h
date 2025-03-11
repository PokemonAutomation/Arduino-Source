/*  Waterfill Intrinsics (x64 AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Intrinsics_x64_AVX512GF_H
#define PokemonAutomation_Kernels_Waterfill_Intrinsics_x64_AVX512GF_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_Waterfill_Intrinsics_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
namespace Intrinsics_x64_AVX512GF{



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
    return _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);
}


#if 0
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
#endif

#if 0
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
#endif

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
#if 0
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
#else
    const __m512i INDEX0 = _mm512_setr_epi8(
         0,  8, 16, 24, 32, 40, 48, 56,
         1,  9, 17, 25, 33, 41, 49, 57,
         2, 10, 18, 26, 34, 42, 50, 58,
         3, 11, 19, 27, 35, 43, 51, 59,
         4, 12, 20, 28, 36, 44, 52, 60,
         5, 13, 21, 29, 37, 45, 53, 61,
         6, 14, 22, 30, 38, 46, 54, 62,
         7, 15, 23, 31, 39, 47, 55, 63
    );
    const __m512i INDEX1 = _mm512_setr_epi8(
         8,  0, 24, 16, 40, 32, 56, 48,
         9,  1, 25, 17, 41, 33, 57, 49,
        10,  2, 26, 18, 42, 34, 58, 50,
        11,  3, 27, 19, 43, 35, 59, 51,
        12,  4, 28, 20, 44, 36, 60, 52,
        13,  5, 29, 21, 45, 37, 61, 53,
        14,  6, 30, 22, 46, 38, 62, 54,
        15,  7, 31, 23, 47, 39, 63, 55
    );
    s0 = _mm512_permutexvar_epi8(INDEX0, s0);
    s1 = _mm512_permutexvar_epi8(INDEX1, s1);
    r0 = _mm512_ternarylogic_epi64(s0, s1, _mm512_set1_epi16((uint16_t)0xff00), 0xd8);
    r1 = _mm512_ternarylogic_epi64(s1, s0, _mm512_set1_epi16((uint16_t)0xff00), 0xd8);
    r1 = _mm512_shuffle_epi8(r1, _mm512_setr_epi8(
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14
    ));
#endif
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
#if 0
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
#else
    const __m512i INDEX0 = _mm512_setr_epi8(
         7, 15, 23, 31, 39, 47, 55, 63,
         6, 14, 22, 30, 38, 46, 54, 62,
         5, 13, 21, 29, 37, 45, 53, 61,
         4, 12, 20, 28, 36, 44, 52, 60,
         3, 11, 19, 27, 35, 43, 51, 59,
         2, 10, 18, 26, 34, 42, 50, 58,
         1,  9, 17, 25, 33, 41, 49, 57,
         0,  8, 16, 24, 32, 40, 48, 56
    );
    const __m512i INDEX1 = _mm512_setr_epi8(
        15,  7, 31, 23, 47, 39, 63, 55,
        14,  6, 30, 22, 46, 38, 62, 54,
        13,  5, 29, 21, 45, 37, 61, 53,
        12,  4, 28, 20, 44, 36, 60, 52,
        11,  3, 27, 19, 43, 35, 59, 51,
        10,  2, 26, 18, 42, 34, 58, 50,
         9,  1, 25, 17, 41, 33, 57, 49,
         8,  0, 24, 16, 40, 32, 56, 48
    );
    s0 = _mm512_permutexvar_epi8(INDEX0, s0);
    s1 = _mm512_permutexvar_epi8(INDEX1, s1);
    r0 = _mm512_ternarylogic_epi64(s0, s1, _mm512_set1_epi16((uint16_t)0xff00), 0xd8);
    r1 = _mm512_ternarylogic_epi64(s1, s0, _mm512_set1_epi16((uint16_t)0xff00), 0xd8);
    r1 = _mm512_shuffle_epi8(r1, _mm512_setr_epi8(
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14
    ));
#endif
}

PA_FORCE_INLINE void transpose_1x64x32(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3
){
    transpose_1x16x16x4(r0, r1);
    transpose_1x16x16x4(r2, r3);
    Intrinsics_x64_AVX512::transpose_16x2x2x2(r0, r1, r2, r3);
}
PA_FORCE_INLINE void transpose_1x64x32_bitreverse_in(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3
){
    transpose_1x16x16x4_bitreverse_in(r0, r1);
    transpose_1x16x16x4_bitreverse_in(r2, r3);
    Intrinsics_x64_AVX512::transpose_16x2x2x2(r0, r1, r2, r3);
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
    Intrinsics_x64_AVX512::transpose_16x2x2x2(r0, r1, r2, r3);
    Intrinsics_x64_AVX512::transpose_16x2x2x2(r4, r5, r6, r7);
    transpose_32x2x2(r0, r1, r2, r3, r4, r5, r6, r7);
}
PA_FORCE_INLINE void transpose_1x64x64_bitreverse_in(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3,
    __m512i& r4, __m512i& r5, __m512i& r6, __m512i& r7
){
    transpose_1x16x16x4_bitreverse_in(r0, r1);
    transpose_1x16x16x4_bitreverse_in(r2, r3);
    transpose_1x16x16x4_bitreverse_in(r4, r5);
    transpose_1x16x16x4_bitreverse_in(r6, r7);
    Intrinsics_x64_AVX512::transpose_16x2x2x2(r0, r1, r2, r3);
    Intrinsics_x64_AVX512::transpose_16x2x2x2(r4, r5, r6, r7);
    transpose_32x2x2(r0, r1, r2, r3, r4, r5, r6, r7);
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
