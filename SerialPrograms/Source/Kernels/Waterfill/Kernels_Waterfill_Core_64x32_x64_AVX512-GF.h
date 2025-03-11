/*  Waterfill Core (x64 AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64x32_x64_AVX512GF_H
#define PokemonAutomation_Kernels_Waterfill_Core_64x32_x64_AVX512GF_H

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x32_x64_AVX512.h"
#include "Kernels_Waterfill_Intrinsics_x64_AVX512.h"
#include "Kernels_Waterfill_Intrinsics_x64_AVX512-GF.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


struct Waterfill_64x32_x64_AVX512GF_ProcessedMask{
    __m512i m0, m1, m2, m3; //  Copy of the masks.
    __m512i b0, b1, b2, b3; //  Bit-reversed copy of the masks.
    __m512i f0, f1, f2, f3; //  Forward transposed.
    __m512i r0, r1, r2, r3; //  Reverse transposed.

    PA_FORCE_INLINE Waterfill_64x32_x64_AVX512GF_ProcessedMask(
        const BinaryTile_64x32_x64_AVX512& m,
        __m512i x0, __m512i x1, __m512i x2, __m512i x3
    ){
        m0 = _mm512_or_si512(x0, m.vec[0]);
        m1 = _mm512_or_si512(x1, m.vec[1]);
        m2 = _mm512_or_si512(x2, m.vec[2]);
        m3 = _mm512_or_si512(x3, m.vec[3]);

        b0 = Intrinsics_x64_AVX512GF::bit_reverse(m0);
        b1 = Intrinsics_x64_AVX512GF::bit_reverse(m1);
        b2 = Intrinsics_x64_AVX512GF::bit_reverse(m2);
        b3 = Intrinsics_x64_AVX512GF::bit_reverse(m3);

        f0 = m0;
        f1 = m1;
        f2 = m2;
        f3 = m3;
        Intrinsics_x64_AVX512GF::transpose_1x64x32(f0, f1, f2, f3);
        r0 = Intrinsics_x64_AVX512GF::bit_reverse(f0);
        r1 = Intrinsics_x64_AVX512GF::bit_reverse(f1);
        r2 = Intrinsics_x64_AVX512GF::bit_reverse(f2);
        r3 = Intrinsics_x64_AVX512GF::bit_reverse(f3);
    }
};



PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64x32_x64_AVX512GF_ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3
){
    Intrinsics_x64_AVX512GF::transpose_1x64x32(x0, x1, x2, x3);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi32(x0, mask.f0), mask.f0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi32(x1, mask.f1), mask.f1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi32(x2, mask.f2), mask.f2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi32(x3, mask.f3), mask.f3, 0b11110010);
    x0 = Intrinsics_x64_AVX512GF::bit_reverse(x0);
    x1 = Intrinsics_x64_AVX512GF::bit_reverse(x1);
    x2 = Intrinsics_x64_AVX512GF::bit_reverse(x2);
    x3 = Intrinsics_x64_AVX512GF::bit_reverse(x3);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi32(x0, mask.r0), mask.r0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi32(x1, mask.r1), mask.r1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi32(x2, mask.r2), mask.r2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi32(x3, mask.r3), mask.r3, 0b11110010);
    Intrinsics_x64_AVX512GF::transpose_1x64x32_bitreverse_in(x0, x1, x2, x3);
}







struct Waterfill_64x32_x64_AVX512GF{



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
//  Save result back into "x".
PA_FORCE_INLINE static void waterfill_expand(BinaryTile_64x32_x64_AVX512& m, BinaryTile_64x32_x64_AVX512& x){
    __m512i x0 = x.vec[0];
    __m512i x1 = x.vec[1];
    __m512i x2 = x.vec[2];
    __m512i x3 = x.vec[3];

    Waterfill_64x32_x64_AVX512GF_ProcessedMask mask(m, x0, x1, x2, x3);
    Intrinsics_x64_AVX512::expand_forward(mask, x0, x1, x2, x3);

    __m512i m0, m1, m2, m3;
    do{
        expand_vertical(mask, x0, x1, x2, x3);
        Intrinsics_x64_AVX512GF::expand_reverse(mask, x0, x1, x2, x3);
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
