/*  Waterfill Core (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64x8_x64_SSE41_H
#define PokemonAutomation_Kernels_Waterfill_Core_64x8_x64_SSE41_H

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x8_x64_SSE42.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


PA_FORCE_INLINE __m128i bit_reverse(__m128i x){
    __m128i r0, r1;

    x = _mm_shuffle_epi8(x, _mm_setr_epi8(7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8));

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



struct Waterfill_64x8_x64_SSE42_ProcessedMask{
    __m128i m0, m1, m2, m3; //  Copy of the masks.
    __m128i b0, b1, b2, b3; //  Bit-reversed copy of the masks.
    __m128i t0, t1, t2, t3; //  Transposed masks.
    __m128i f1, f2, f3;     //  Forward-carry mask.
    __m128i r0, r1, r2;     //  Reverse-carry mask.

    PA_FORCE_INLINE Waterfill_64x8_x64_SSE42_ProcessedMask(
        const BinaryTile_64x8_x64_SSE42& m,
        __m128i x0, __m128i x1, __m128i x2, __m128i x3
    ){
        m0 = _mm_or_si128(x0, m.vec[0]);
        m1 = _mm_or_si128(x1, m.vec[1]);
        m2 = _mm_or_si128(x2, m.vec[2]);
        m3 = _mm_or_si128(x3, m.vec[3]);

        b0 = bit_reverse(m0);
        b1 = bit_reverse(m1);
        b2 = bit_reverse(m2);
        b3 = bit_reverse(m3);

        t0 = m0;
        t1 = m1;
        t2 = m2;
        t3 = m3;
        transpose_i64_2x2_SSE2(t0, t1);
        transpose_i64_2x2_SSE2(t2, t3);

        //  Forward carry
        __m128i f0 = t0;
        f1 = _mm_and_si128(f0, t1);
        transpose_i64_2x2_SSE2(f0, f1);
        f2 = t2;
        f3 = _mm_and_si128(f2, t3);
        transpose_i64_2x2_SSE2(f2, f3);

        //  Reverse carry
        __m128i r3 = t3;
        r2 = _mm_and_si128(r3, t2);
        transpose_i64_2x2_SSE2(r2, r3);
        r1 = t1;
        r0 = _mm_and_si128(r1, t0);
        transpose_i64_2x2_SSE2(r0, r1);
    }
};



PA_FORCE_INLINE bool keep_going(
    const Waterfill_64x8_x64_SSE42_ProcessedMask& mask,
    __m128i& m0, __m128i& m1, __m128i& m2, __m128i& m3,
    __m128i& x0, __m128i& x1, __m128i& x2, __m128i& x3
){
    m0 = _mm_andnot_si128(x0, mask.m0);
    m1 = _mm_andnot_si128(x1, mask.m1);
    m2 = _mm_andnot_si128(x2, mask.m2);
    m3 = _mm_andnot_si128(x3, mask.m3);

    __m128i r0;

    r0 = _mm_slli_epi64(m0, 1);
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(m0), _mm_castsi128_pd(m1), 1)));
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_setzero_pd(), _mm_castsi128_pd(m0), 1)));
    __m128i changed = _mm_and_si128(r0, x0);

    r0 = _mm_slli_epi64(m1, 1);
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(m1), _mm_castsi128_pd(m2), 1)));
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(m0), _mm_castsi128_pd(m1), 1)));
    r0 = _mm_and_si128(r0, x1);
    changed = _mm_or_si128(changed, r0);

    r0 = _mm_slli_epi64(m2, 1);
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(m2), _mm_castsi128_pd(m3), 1)));
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(m1), _mm_castsi128_pd(m2), 1)));
    r0 = _mm_and_si128(r0, x2);
    changed = _mm_or_si128(changed, r0);

    r0 = _mm_slli_epi64(m3, 1);
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(m3), _mm_setzero_pd(), 1)));
    r0 = _mm_or_si128(r0, _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(m2), _mm_castsi128_pd(m3), 1)));
    r0 = _mm_and_si128(r0, x3);
    changed = _mm_or_si128(changed, r0);

    return !_mm_test_all_zeros(changed, changed);
}



PA_FORCE_INLINE void expand_forward(
    const Waterfill_64x8_x64_SSE42_ProcessedMask& mask,
    __m128i& x0, __m128i& x1, __m128i& x2, __m128i& x3
){
    __m128i s0 = _mm_add_epi64(x0, mask.m0);
    __m128i s1 = _mm_add_epi64(x1, mask.m1);
    __m128i s2 = _mm_add_epi64(x2, mask.m2);
    __m128i s3 = _mm_add_epi64(x3, mask.m3);

    s0 = _mm_andnot_si128(s0, mask.m0);
    s1 = _mm_andnot_si128(s1, mask.m1);
    s2 = _mm_andnot_si128(s2, mask.m2);
    s3 = _mm_andnot_si128(s3, mask.m3);

    x0 = _mm_or_si128(x0, s0);
    x1 = _mm_or_si128(x1, s1);
    x2 = _mm_or_si128(x2, s2);
    x3 = _mm_or_si128(x3, s3);
}
PA_FORCE_INLINE void expand_reverse(__m128i m, __m128i b, __m128i& x){
    __m128i s = bit_reverse(_mm_add_epi64(bit_reverse(x), b));
    s = _mm_xor_si128(s, m);
    s = _mm_and_si128(s, m);
    x = _mm_or_si128(x, s);
}
PA_FORCE_INLINE void expand_reverse(
    const Waterfill_64x8_x64_SSE42_ProcessedMask& mask,
    __m128i& x0, __m128i& x1, __m128i& x2, __m128i& x3
){
    expand_reverse(mask.m0, mask.b0, x0);
    expand_reverse(mask.m1, mask.b1, x1);
    expand_reverse(mask.m2, mask.b2, x2);
    expand_reverse(mask.m3, mask.b3, x3);
}
PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64x8_x64_SSE42_ProcessedMask& mask,
    __m128i& x0, __m128i& x1, __m128i& x2, __m128i& x3
){
    //  Carry across adjacent rows.
    transpose_i64_2x2_SSE2(x0, x1);
    transpose_i64_2x2_SSE2(x2, x3);
    x0 = _mm_or_si128(x0, _mm_and_si128(x1, mask.t0));
    x1 = _mm_or_si128(x1, _mm_and_si128(x0, mask.t1));
    x2 = _mm_or_si128(x2, _mm_and_si128(x3, mask.t2));
    x3 = _mm_or_si128(x3, _mm_and_si128(x2, mask.t3));
    transpose_i64_2x2_SSE2(x0, x1);
    transpose_i64_2x2_SSE2(x2, x3);

    //  Carry across groups of 2 rows.
    x1 = _mm_or_si128(x1, _mm_and_si128(_mm_unpackhi_epi64(x0, x0), mask.f1));
    x2 = _mm_or_si128(x2, _mm_and_si128(_mm_unpacklo_epi64(x3, x3), mask.r2));
    x2 = _mm_or_si128(x2, _mm_and_si128(_mm_unpackhi_epi64(x1, x1), mask.f2));
    x1 = _mm_or_si128(x1, _mm_and_si128(_mm_unpacklo_epi64(x2, x2), mask.r1));
    x3 = _mm_or_si128(x3, _mm_and_si128(_mm_unpackhi_epi64(x2, x2), mask.f3));
    x0 = _mm_or_si128(x0, _mm_and_si128(_mm_unpacklo_epi64(x1, x1), mask.r0));
}



struct Waterfill_64x8_x64_SSE42{



static PA_FORCE_INLINE __m128i vec_or(const BinaryTile_64x8_x64_SSE42& tile){
    __m128i v0 = _mm_or_si128(tile.vec[0], tile.vec[1]);
    __m128i v1 = _mm_or_si128(tile.vec[2], tile.vec[3]);
    v0 = _mm_or_si128(v0, v1);
    return v0;
}
static PA_FORCE_INLINE uint64_t row_or(const BinaryTile_64x8_x64_SSE42& tile){
    __m128i v = vec_or(tile);
    return _mm_cvtsi128_si64(v) | _mm_extract_epi64(v, 1);
}


//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static PA_FORCE_INLINE bool find_bit(size_t& x, size_t& y, const BinaryTile_64x8_x64_SSE42& tile){
    __m128i anything = vec_or(tile);
    if (_mm_test_all_zeros(anything, anything)){
        return false;
    }
    for (size_t c = 0; c < 8; c++){
        size_t pos;
        if (trailing_zeros(pos, tile.row(c))){
            x = pos;
            y = c;
            return true;
        }
    }
    return false;
}



//  Finds the boundaries of the one-bits inside the tile.
//  Max values are one past the end.
//  Behavior is undefined if tile is zero.
static PA_FORCE_INLINE void boundaries(
    const BinaryTile_64x8_x64_SSE42& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
){
    uint64_t all_or = row_or(tile);
    trailing_zeros(min_x, all_or);
    max_x = bitlength(all_or);

    min_y = 0;
    for (size_t c = 0; c < 8; c++){
        if (tile.row(c) != 0){
            min_y = c;
            break;
        }
    }
    max_y = 0;
    for (size_t c = 8; c > 0; c--){
        if (tile.row(c - 1) != 0){
            max_y = c;
            break;
        }
    }
}



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
static PA_FORCE_INLINE __m128i popcount_indexsum(__m128i& sum_index, __m128i x){
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

    sum_index = _mm_and_si128(sumxaxis, _mm_set1_epi64x(0x000000000000ffff));
    return _mm_and_si128(popcount, _mm_set1_epi64x(0x000000000000ffff));
}
static PA_FORCE_INLINE uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_64x8_x64_SSE42& tile
){
    __m128i sum_p, sum_x, sum_y;
    {
        __m128i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[0]);
        sum_p = pop;
        sum_x = sum;
        sum_y = _mm_mul_epu32(pop, _mm_set_epi64x(1, 0));
    }
    {
        __m128i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[1]);
        sum_p = _mm_add_epi64(sum_p, pop);
        sum_x = _mm_add_epi64(sum_x, sum);
        sum_y = _mm_add_epi64(sum_y, _mm_mul_epu32(pop, _mm_set_epi64x(3, 2)));
    }
    {
        __m128i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[2]);
        sum_p = _mm_add_epi64(sum_p, pop);
        sum_x = _mm_add_epi64(sum_x, sum);
        sum_y = _mm_add_epi64(sum_y, _mm_mul_epu32(pop, _mm_set_epi64x(5, 4)));
    }
    {
        __m128i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[3]);
        sum_p = _mm_add_epi64(sum_p, pop);
        sum_x = _mm_add_epi64(sum_x, sum);
        sum_y = _mm_add_epi64(sum_y, _mm_mul_epu32(pop, _mm_set_epi64x(7, 6)));
    }
    sum_xcoord = _mm_cvtsi128_si64(sum_x) + _mm_extract_epi64(sum_x, 1);
    sum_ycoord = _mm_cvtsi128_si64(sum_y) + _mm_extract_epi64(sum_y, 1);
    return _mm_cvtsi128_si64(sum_p) + _mm_extract_epi64(sum_p, 1);
}



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x". Clear bits of object from "m".
static PA_FORCE_INLINE void waterfill_expand(BinaryTile_64x8_x64_SSE42& m, BinaryTile_64x8_x64_SSE42& x){
    __m128i x0 = x.vec[0];
    __m128i x1 = x.vec[1];
    __m128i x2 = x.vec[2];
    __m128i x3 = x.vec[3];

    Waterfill_64x8_x64_SSE42_ProcessedMask mask(m, x0, x1, x2, x3);
    expand_forward(mask, x0, x1, x2, x3);

    __m128i m0, m1, m2, m3;
    do{
        expand_vertical(mask, x0, x1, x2, x3);
        expand_reverse(mask, x0, x1, x2, x3);
        expand_forward(mask, x0, x1, x2, x3);
    }while (keep_going(
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
    const BinaryTile_64x8_x64_SSE42& mask,
    BinaryTile_64x8_x64_SSE42& tile,
    const BinaryTile_64x8_x64_SSE42& border
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
    const BinaryTile_64x8_x64_SSE42& mask,
    BinaryTile_64x8_x64_SSE42& tile,
    const BinaryTile_64x8_x64_SSE42& border
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
    const BinaryTile_64x8_x64_SSE42& mask,
    BinaryTile_64x8_x64_SSE42& tile,
    const BinaryTile_64x8_x64_SSE42& border
){
    __m128i changed = _mm_setzero_si128();
    for (size_t c = 0; c < 4; c++){
        __m128i available = _mm_andnot_si128(tile.vec[c], mask.vec[c]);
        __m128i new_bits = _mm_and_si128(available, _mm_srli_epi64(border.vec[c], 63));
        changed = _mm_or_si128(changed, new_bits);
        tile.vec[c] = _mm_or_si128(tile.vec[c], new_bits);
    }
    return !_mm_test_all_zeros(changed, changed);
}
static PA_FORCE_INLINE bool waterfill_touch_right(
    const BinaryTile_64x8_x64_SSE42& mask,
    BinaryTile_64x8_x64_SSE42& tile,
    const BinaryTile_64x8_x64_SSE42& border
){
    __m128i changed = _mm_setzero_si128();
    for (size_t c = 0; c < 4; c++){
        __m128i available = _mm_andnot_si128(tile.vec[c], mask.vec[c]);
        __m128i new_bits = _mm_and_si128(available, _mm_slli_epi64(border.vec[c], 63));
        changed = _mm_or_si128(changed, new_bits);
        tile.vec[c] = _mm_or_si128(tile.vec[c], new_bits);
    }
    return !_mm_test_all_zeros(changed, changed);
}


};



}
}
}
#endif
