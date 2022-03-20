/*  Waterfill Core (AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512_H
#define PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
struct Waterfill_x64_AVX512{



//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static bool find_bit(size_t& x, size_t& y, const BinaryTile_AVX512& tile);



//  Finds the boundaries of the one-bits inside the tile.
//  Max values are one past the end.
//  Behavior is undefined if tile is zero.
static void boundaries(
    const BinaryTile_AVX512& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
);



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
static __m512i popcount_indexsum(__m512i& sum_index, __m512i x);
static uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_AVX512& tile
);



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
static void Waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x);



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
static bool Waterfill_touch_top   (const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);
static bool Waterfill_touch_bottom(const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);
static bool Waterfill_touch_left  (const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);
static bool Waterfill_touch_right (const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);




};


template <bool AVX512GF>
PA_FORCE_INLINE __m512i bit_reverse(__m512i x);


template <bool AVX512GF>
struct Waterfill_x64_AVX512_ProcessedMask{
    __m512i m0, m1, m2, m3, m4, m5, m6, m7; //  Copy of the masks.
    __m512i b0, b1, b2, b3, b4, b5, b6, b7; //  Bit-reversed copy of the masks.
    __m512i t0, t1, t2, t3, t4, t5, t6, t7; //  Transposed masks.
    __m512i f1, f2, f3, f4, f5, f6, f7;     //  Forward-carry mask.
    __m512i r0, r1, r2, r3, r4, r5, r6;     //  Reverse-carry mask.

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

        b0 = bit_reverse<AVX512GF>(m0);
        b1 = bit_reverse<AVX512GF>(m1);
        b2 = bit_reverse<AVX512GF>(m2);
        b3 = bit_reverse<AVX512GF>(m3);
        b4 = bit_reverse<AVX512GF>(m4);
        b5 = bit_reverse<AVX512GF>(m5);
        b6 = bit_reverse<AVX512GF>(m6);
        b7 = bit_reverse<AVX512GF>(m7);

        t0 = m0;
        t1 = m1;
        t2 = m2;
        t3 = m3;
        t4 = m4;
        t5 = m5;
        t6 = m6;
        t7 = m7;
        transpose_i64_8x8_AVX512(t0, t1, t2, t3, t4, t5, t6, t7);

        //  Forward carry
        __m512i f0 = t0;
        f1 = _mm512_and_si512(f0, t1);
        f2 = _mm512_and_si512(f1, t2);
        f3 = _mm512_and_si512(f2, t3);
        f4 = _mm512_and_si512(f3, t4);
        f5 = _mm512_and_si512(f4, t5);
        f6 = _mm512_and_si512(f5, t6);
        f7 = _mm512_and_si512(f6, t7);
        transpose_i64_8x8_AVX512(f0, f1, f2, f3, f4, f5, f6, f7);

        //  Reverse carry
        __m512i r7 = t7;
        r6 = _mm512_and_si512(r7, t6);
        r5 = _mm512_and_si512(r6, t5);
        r4 = _mm512_and_si512(r5, t4);
        r3 = _mm512_and_si512(r4, t3);
        r2 = _mm512_and_si512(r3, t2);
        r1 = _mm512_and_si512(r2, t1);
        r0 = _mm512_and_si512(r1, t0);
        transpose_i64_8x8_AVX512(r0, r1, r2, r3, r4, r5, r6, r7);
    }
};


template <bool AVX512GF>
PA_FORCE_INLINE void expand_reverse(__m512i m, __m512i b, __m512i& x){
    __m512i s = bit_reverse<AVX512GF>(_mm512_add_epi64(bit_reverse<AVX512GF>(x), b));
    x = _mm512_ternarylogic_epi64(x, s, m, 0b11110010);
}
template <bool AVX512GF>
PA_FORCE_INLINE void expand_forward(
    const Waterfill_x64_AVX512_ProcessedMask<AVX512GF>& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    __m512i s0 = _mm512_add_epi64(x0, mask.m0);
    __m512i s1 = _mm512_add_epi64(x1, mask.m1);
    __m512i s2 = _mm512_add_epi64(x2, mask.m2);
    __m512i s3 = _mm512_add_epi64(x3, mask.m3);
    __m512i s4 = _mm512_add_epi64(x4, mask.m4);
    __m512i s5 = _mm512_add_epi64(x5, mask.m5);
    __m512i s6 = _mm512_add_epi64(x6, mask.m6);
    __m512i s7 = _mm512_add_epi64(x7, mask.m7);

    x0 = _mm512_ternarylogic_epi64(x0, s0, mask.m0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, s1, mask.m1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, s2, mask.m2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, s3, mask.m3, 0b11110010);
    x4 = _mm512_ternarylogic_epi64(x4, s4, mask.m4, 0b11110010);
    x5 = _mm512_ternarylogic_epi64(x5, s5, mask.m5, 0b11110010);
    x6 = _mm512_ternarylogic_epi64(x6, s6, mask.m6, 0b11110010);
    x7 = _mm512_ternarylogic_epi64(x7, s7, mask.m7, 0b11110010);
}
template <bool AVX512GF>
PA_FORCE_INLINE void expand_reverse(
    const Waterfill_x64_AVX512_ProcessedMask<AVX512GF>& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    expand_reverse<AVX512GF>(mask.m0, mask.b0, x0);
    expand_reverse<AVX512GF>(mask.m1, mask.b1, x1);
    expand_reverse<AVX512GF>(mask.m2, mask.b2, x2);
    expand_reverse<AVX512GF>(mask.m3, mask.b3, x3);
    expand_reverse<AVX512GF>(mask.m4, mask.b4, x4);
    expand_reverse<AVX512GF>(mask.m5, mask.b5, x5);
    expand_reverse<AVX512GF>(mask.m6, mask.b6, x6);
    expand_reverse<AVX512GF>(mask.m7, mask.b7, x7);
}




template <bool AVX512GF>
PA_FORCE_INLINE void expand_vertical(
    const Waterfill_x64_AVX512_ProcessedMask<AVX512GF>& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    //  Carry across adjacent rows.
    transpose_i64_8x8_AVX512(x0, x1, x2, x3, x4, x5, x6, x7);
    x1 = _mm512_ternarylogic_epi64(x1, x0, mask.t1, 0b11111000);
    x6 = _mm512_ternarylogic_epi64(x6, x7, mask.t6, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, x1, mask.t2, 0b11111000);
    x5 = _mm512_ternarylogic_epi64(x5, x6, mask.t5, 0b11111000);
    x3 = _mm512_ternarylogic_epi64(x3, x2, mask.t3, 0b11111000);
    x4 = _mm512_ternarylogic_epi64(x4, x5, mask.t4, 0b11111000);
    x4 = _mm512_ternarylogic_epi64(x4, x3, mask.t4, 0b11111000);
    x3 = _mm512_ternarylogic_epi64(x3, x4, mask.t3, 0b11111000);
    x5 = _mm512_ternarylogic_epi64(x5, x4, mask.t5, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, x3, mask.t2, 0b11111000);
    x6 = _mm512_ternarylogic_epi64(x6, x5, mask.t6, 0b11111000);
    x1 = _mm512_ternarylogic_epi64(x1, x2, mask.t1, 0b11111000);
    x7 = _mm512_ternarylogic_epi64(x7, x6, mask.t7, 0b11111000);
    x0 = _mm512_ternarylogic_epi64(x0, x1, mask.t0, 0b11111000);
    transpose_i64_8x8_AVX512(x0, x1, x2, x3, x4, x5, x6, x7);

    //  Carry across groups of 8 rows.
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x0), mask.f1, 0b11111000);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x7)), mask.r6, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x1), mask.f2, 0b11111000);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x6)), mask.r5, 0b11111000);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x2), mask.f3, 0b11111000);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x5)), mask.r4, 0b11111000);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x3), mask.f4, 0b11111000);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x4)), mask.r3, 0b11111000);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x4), mask.f5, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x3)), mask.r2, 0b11111000);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x5), mask.f6, 0b11111000);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x2)), mask.r1, 0b11111000);
    x7 = _mm512_ternarylogic_epi64(x7, _mm512_permutexvar_epi64(_mm512_set1_epi64(7), x6), mask.f7, 0b11111000);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_broadcastq_epi64(_mm512_castsi512_si128(x1)), mask.r0, 0b11111000);
}


template <bool AVX512GF>
void Waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x){
    __m512i x0 = x.vec[0];
    __m512i x1 = x.vec[1];
    __m512i x2 = x.vec[2];
    __m512i x3 = x.vec[3];
    __m512i x4 = x.vec[4];
    __m512i x5 = x.vec[5];
    __m512i x6 = x.vec[6];
    __m512i x7 = x.vec[7];

    Waterfill_x64_AVX512_ProcessedMask<AVX512GF> mask(m, x0, x1, x2, x3, x4, x5, x6, x7);

    __m512i changed;
    do{
        expand_forward(mask, x0, x1, x2, x3, x4, x5, x6, x7);
        expand_vertical(mask, x0, x1, x2, x3, x4, x5, x6, x7);
        expand_reverse(mask, x0, x1, x2, x3, x4, x5, x6, x7);
        changed = _mm512_xor_si512(x0, x.vec[0]);
        x.vec[0] = x0;
        changed = _mm512_ternarylogic_epi64(changed, x1, x.vec[1], 0b11110110);
        x.vec[1] = x1;
        changed = _mm512_ternarylogic_epi64(changed, x2, x.vec[2], 0b11110110);
        x.vec[2] = x2;
        changed = _mm512_ternarylogic_epi64(changed, x3, x.vec[3], 0b11110110);
        x.vec[3] = x3;
        changed = _mm512_ternarylogic_epi64(changed, x4, x.vec[4], 0b11110110);
        x.vec[4] = x4;
        changed = _mm512_ternarylogic_epi64(changed, x5, x.vec[5], 0b11110110);
        x.vec[5] = x5;
        changed = _mm512_ternarylogic_epi64(changed, x6, x.vec[6], 0b11110110);
        x.vec[6] = x6;
        changed = _mm512_ternarylogic_epi64(changed, x7, x.vec[7], 0b11110110);
        x.vec[7] = x7;
//        cout << x.dump() << endl;
    }while (_mm512_test_epi64_mask(changed, changed));
}




}
}
}
#endif
