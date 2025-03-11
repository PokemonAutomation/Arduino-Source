/*  Waterfill Core (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64x32_x64_AVX512_H
#define PokemonAutomation_Kernels_Waterfill_Core_64x32_x64_AVX512_H

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x32_x64_AVX512.h"
#include "Kernels_Waterfill_Intrinsics_x64_AVX512.h"
#include "Kernels_Waterfill_Intrinsics_x64_AVX512-GF.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


#if 0
struct Waterfill_64x32_x64_AVX512_ProcessedMask{
    __m512i m0, m1, m2, m3; //  Copy of the masks.
    __m512i b0, b1, b2, b3; //  Bit-reversed copy of the masks.
    __m512i f0, f1, f2, f3; //  Forward transposed.
    __m512i r0, r1, r2, r3; //  Reverse transposed.

    PA_FORCE_INLINE Waterfill_64x32_x64_AVX512_ProcessedMask(
        const BinaryTile_64x32_x64_AVX512& m,
        __m512i x0, __m512i x1, __m512i x2, __m512i x3
    ){
        m0 = _mm512_or_si512(x0, m.vec[0]);
        m1 = _mm512_or_si512(x1, m.vec[1]);
        m2 = _mm512_or_si512(x2, m.vec[2]);
        m3 = _mm512_or_si512(x3, m.vec[3]);

        b0 = Intrinsics_x64_AVX512::bit_reverse(m0);
        b1 = Intrinsics_x64_AVX512::bit_reverse(m1);
        b2 = Intrinsics_x64_AVX512::bit_reverse(m2);
        b3 = Intrinsics_x64_AVX512::bit_reverse(m3);

        f0 = m0;
        f1 = m1;
        f2 = m2;
        f3 = m3;
        Intrinsics_x64_AVX512::transpose_1x64x32(f0, f1, f2, f3);
        r0 = Intrinsics_x64_AVX512::bit_reverse(f0);
        r1 = Intrinsics_x64_AVX512::bit_reverse(f1);
        r2 = Intrinsics_x64_AVX512::bit_reverse(f2);
        r3 = Intrinsics_x64_AVX512::bit_reverse(f3);
    }
};



PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64x32_x64_AVX512_ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3
){
    Intrinsics_x64_AVX512::transpose_1x64x32(x0, x1, x2, x3);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi32(x0, mask.f0), mask.f0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi32(x1, mask.f1), mask.f1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi32(x2, mask.f2), mask.f2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi32(x3, mask.f3), mask.f3, 0b11110010);
    x0 = Intrinsics_x64_AVX512::bit_reverse(x0);
    x1 = Intrinsics_x64_AVX512::bit_reverse(x1);
    x2 = Intrinsics_x64_AVX512::bit_reverse(x2);
    x3 = Intrinsics_x64_AVX512::bit_reverse(x3);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi32(x0, mask.r0), mask.r0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi32(x1, mask.r1), mask.r1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi32(x2, mask.r2), mask.r2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi32(x3, mask.r3), mask.r3, 0b11110010);
    Intrinsics_x64_AVX512::transpose_1x64x32_bitreverse_in(x0, x1, x2, x3);
}
#else
struct Waterfill_64x32_x64_AVX512_ProcessedMask{
    __m512i m0, m1, m2, m3; //  Copy of the masks.
    __m512i b0, b1, b2, b3; //  Bit-reversed copy of the masks.
    __m256i t0, t1, t2, t3, t4, t5, t6, t7; //  Transposed masks.
    __m512i f1, f2, f3;     //  Forward-carry mask.
    __m512i r0, r1, r2;     //  Reverse-carry mask.

    PA_FORCE_INLINE Waterfill_64x32_x64_AVX512_ProcessedMask(
        const BinaryTile_64x32_x64_AVX512& m,
        __m512i x0, __m512i x1, __m512i x2, __m512i x3
    ){
        m0 = _mm512_or_si512(x0, m.vec[0]);
        m1 = _mm512_or_si512(x1, m.vec[1]);
        m2 = _mm512_or_si512(x2, m.vec[2]);
        m3 = _mm512_or_si512(x3, m.vec[3]);

        b0 = Intrinsics_x64_AVX512::bit_reverse(m0);
        b1 = Intrinsics_x64_AVX512::bit_reverse(m1);
        b2 = Intrinsics_x64_AVX512::bit_reverse(m2);
        b3 = Intrinsics_x64_AVX512::bit_reverse(m3);

        Intrinsics_x64_AVX512::transpose_64x8x4_forward(
            m0, m1, m2, m3,
            t0, t1, t2, t3, t4, t5, t6, t7
        );

        __m256i q0, q1, q2, q3, q4, q5, q6, q7;
        __m512i qx;

        //  Forward carry
        q0 = t0;
        q1 = _mm256_and_si256(q0, t1);
        q2 = _mm256_and_si256(q1, t2);
        q3 = _mm256_and_si256(q2, t3);
        q4 = _mm256_and_si256(q3, t4);
        q5 = _mm256_and_si256(q4, t5);
        q6 = _mm256_and_si256(q5, t6);
        q7 = _mm256_and_si256(q6, t7);
        Intrinsics_x64_AVX512::transpose_64x8x4_inverse(qx, f1, f2, f3, q0, q1, q2, q3, q4, q5, q6, q7);

        //  Reverse carry
        q7 = t7;
        q6 = _mm256_and_si256(q7, t6);
        q5 = _mm256_and_si256(q6, t5);
        q4 = _mm256_and_si256(q5, t4);
        q3 = _mm256_and_si256(q4, t3);
        q2 = _mm256_and_si256(q3, t2);
        q1 = _mm256_and_si256(q2, t1);
        q0 = _mm256_and_si256(q1, t0);
        Intrinsics_x64_AVX512::transpose_64x8x4_inverse(r0, r1, r2, qx, q0, q1, q2, q3, q4, q5, q6, q7);
    }
};



PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64x32_x64_AVX512_ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3
){
    //  Carry across adjacent rows.
    __m256i y0, y1, y2, y3, y4, y5, y6, y7;
    Intrinsics_x64_AVX512::transpose_64x8x4_forward(x0, x1, x2, x3, y0, y1, y2, y3, y4, y5, y6, y7);
    y1 = _mm256_ternarylogic_epi64(y1, y0, mask.t1, 0b11111000);
    y6 = _mm256_ternarylogic_epi64(y6, y7, mask.t6, 0b11111000);
    y2 = _mm256_ternarylogic_epi64(y2, y1, mask.t2, 0b11111000);
    y5 = _mm256_ternarylogic_epi64(y5, y6, mask.t5, 0b11111000);
    y3 = _mm256_ternarylogic_epi64(y3, y2, mask.t3, 0b11111000);
    y4 = _mm256_ternarylogic_epi64(y4, y5, mask.t4, 0b11111000);
    y4 = _mm256_ternarylogic_epi64(y4, y3, mask.t4, 0b11111000);
    y3 = _mm256_ternarylogic_epi64(y3, y4, mask.t3, 0b11111000);
    y5 = _mm256_ternarylogic_epi64(y5, y4, mask.t5, 0b11111000);
    y2 = _mm256_ternarylogic_epi64(y2, y3, mask.t2, 0b11111000);
    y6 = _mm256_ternarylogic_epi64(y6, y5, mask.t6, 0b11111000);
    y1 = _mm256_ternarylogic_epi64(y1, y2, mask.t1, 0b11111000);
    y7 = _mm256_ternarylogic_epi64(y7, y6, mask.t7, 0b11111000);
    y0 = _mm256_ternarylogic_epi64(y0, y1, mask.t0, 0b11111000);
    Intrinsics_x64_AVX512::transpose_64x8x4_inverse(x0, x1, x2, x3, y0, y1, y2, y3, y4, y5, y6, y7);

    //  Carry across groups of 4 rows.
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x0), mask.f1, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x3)), mask.r2, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x1), mask.f2, 0b11111000);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x2)), mask.r1, 0b11111000);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x2), mask.f3, 0b11111000);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x1)), mask.r0, 0b11111000);
}
#endif







struct Waterfill_64x32_x64_AVX512{



static PA_FORCE_INLINE __m512i vec_or(const BinaryTile_64x32_x64_AVX512& tile){
    __m512i v0 = _mm512_or_si512(tile.vec[0], tile.vec[1]);
    __m512i v1 = _mm512_or_si512(tile.vec[2], tile.vec[3]);
    v0 = _mm512_or_si512(v0, v1);
    return v0;
}
static PA_FORCE_INLINE uint64_t row_or(const BinaryTile_64x32_x64_AVX512& tile){
    return _mm512_reduce_or_epi64(vec_or(tile));
}


//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static PA_FORCE_INLINE bool find_bit(size_t& x, size_t& y, const BinaryTile_64x32_x64_AVX512& tile){
    __m512i anything = vec_or(tile);
    if (!_mm512_test_epi64_mask(anything, anything)){
        return false;
    }
    __m512i row = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    for (size_t c = 0; c < 4; c++){
        __m512i vec = tile.vec[c];
        __mmask8 mask = _mm512_test_epi64_mask(vec, vec);
        if (mask){
            vec = _mm512_maskz_compress_epi64(mask, vec);
            row = _mm512_maskz_compress_epi64(mask, row);
            uint64_t word = _mm_cvtsi128_si64(_mm512_castsi512_si128(vec));
            y = _mm_cvtsi128_si64(_mm512_castsi512_si128(row));
            trailing_zeros(x, word);
            return true;
        }
        row = _mm512_add_epi64(row, _mm512_set1_epi64(8));
    }
    return false;
}



//  Finds the boundaries of the one-bits inside the tile.
//  Max values are one past the end.
//  Behavior is undefined if tile is zero.
static PA_FORCE_INLINE void boundaries(
    const BinaryTile_64x32_x64_AVX512& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
){
    uint64_t all_or = row_or(tile);
    trailing_zeros(min_x, all_or);
    max_x = bitlength(all_or);

    __m512i row = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i min = _mm512_set1_epi64(-1);
    __m512i max = _mm512_setzero_si512();
    for (size_t c = 0; c < 4; c++){
        __m512i vec = tile.vec[c];
        __mmask8 mask = _mm512_cmpneq_epu64_mask(vec, _mm512_setzero_si512());
        min = _mm512_mask_min_epu64(min, mask, min, row);
        max = _mm512_mask_max_epu64(max, mask, max, row);
        row = _mm512_add_epi64(row, _mm512_set1_epi64(8));
    }
    min_y = _mm512_reduce_min_epu64(min);
    max_y = _mm512_reduce_max_epu64(max) + 1;
}



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
static PA_FORCE_INLINE uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_64x32_x64_AVX512& tile
){
    __m512i sum_p, sum_x, sum_y;
    __m512i offsets = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    {
        __m512i pop, sum;
        pop = Intrinsics_x64_AVX512::popcount_indexsum(sum, tile.vec[0]);
        sum_p = pop;
        sum_x = sum;
        sum_y = _mm512_mul_epu32(pop, offsets);
    }
    for (size_t c = 1; c < 4; c++){
        __m512i pop, sum;
        pop = Intrinsics_x64_AVX512::popcount_indexsum(sum, tile.vec[c]);
        sum_p = _mm512_add_epi64(sum_p, pop);
        sum_x = _mm512_add_epi64(sum_x, sum);
        offsets = _mm512_add_epi64(offsets, _mm512_set1_epi64(8));
        sum_y = _mm512_add_epi64(sum_y, _mm512_mul_epu32(pop, offsets));
    }
    sum_xcoord = _mm512_reduce_add_epi64(sum_x);
    sum_ycoord = _mm512_reduce_add_epi64(sum_y);
    return _mm512_reduce_add_epi64(sum_p);
}



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x". Clear bits of object from "m".
PA_FORCE_INLINE static void waterfill_expand(BinaryTile_64x32_x64_AVX512& m, BinaryTile_64x32_x64_AVX512& x){
    __m512i x0 = x.vec[0];
    __m512i x1 = x.vec[1];
    __m512i x2 = x.vec[2];
    __m512i x3 = x.vec[3];

    Waterfill_64x32_x64_AVX512_ProcessedMask mask(m, x0, x1, x2, x3);
    Intrinsics_x64_AVX512::expand_forward(mask, x0, x1, x2, x3);

    __m512i m0, m1, m2, m3;
    do{
        expand_vertical(mask, x0, x1, x2, x3);
        Intrinsics_x64_AVX512::expand_reverse(mask, x0, x1, x2, x3);
        Intrinsics_x64_AVX512::expand_forward(mask, x0, x1, x2, x3);
    }while (Intrinsics_x64_AVX512::keep_going(
        mask,
        m0, m1, m2, m3,
        x0, x1, x2, x3
    ));
    x.vec[0] = x0;
    x.vec[1] = x1;
    x.vec[2] = x2;
    x.vec[3] = x3;
    m.vec[0] = m0;
    m.vec[1] = m1;
    m.vec[2] = m2;
    m.vec[3] = m3;
}



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
static PA_FORCE_INLINE bool waterfill_touch_top(
    const BinaryTile_64x32_x64_AVX512& mask,
    BinaryTile_64x32_x64_AVX512& tile,
    const BinaryTile_64x32_x64_AVX512& border
){
    uint64_t available = mask.top() & ~tile.top();
    uint64_t new_bits = available & border.bottom();
    if (new_bits == 0){
        return false;
    }
    tile.top() |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_bottom(
    const BinaryTile_64x32_x64_AVX512& mask,
    BinaryTile_64x32_x64_AVX512& tile,
    const BinaryTile_64x32_x64_AVX512& border
){
    uint64_t available = mask.bottom() & ~tile.bottom();
    uint64_t new_bits = available & border.top();
    if (new_bits == 0){
        return false;
    }
    tile.bottom() |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_left(
    const BinaryTile_64x32_x64_AVX512& mask,
    BinaryTile_64x32_x64_AVX512& tile,
    const BinaryTile_64x32_x64_AVX512& border
){
    __m512i changed = _mm512_setzero_si512();
    for (size_t c = 0; c < 4; c++){
        __m512i available = _mm512_andnot_si512(tile.vec[c], mask.vec[c]);
        __m512i new_bits = _mm512_and_si512(available, _mm512_srli_epi64(border.vec[c], 63));
        changed = _mm512_or_si512(changed, new_bits);
        tile.vec[c] = _mm512_or_si512(tile.vec[c], new_bits);
    }
    return _mm512_test_epi64_mask(changed, changed);
}
static PA_FORCE_INLINE bool waterfill_touch_right(
    const BinaryTile_64x32_x64_AVX512& mask,
    BinaryTile_64x32_x64_AVX512& tile,
    const BinaryTile_64x32_x64_AVX512& border
){
    __m512i changed = _mm512_setzero_si512();
    for (size_t c = 0; c < 4; c++){
        __m512i available = _mm512_andnot_si512(tile.vec[c], mask.vec[c]);
        __m512i new_bits = _mm512_and_si512(available, _mm512_slli_epi64(border.vec[c], 63));
        changed = _mm512_or_si512(changed, new_bits);
        tile.vec[c] = _mm512_or_si512(tile.vec[c], new_bits);
    }
    return _mm512_test_epi64_mask(changed, changed);
}





};



}
}
}
#endif
