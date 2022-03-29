/*  Waterfill Core (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64x64_x64_AVX512_H
#define PokemonAutomation_Kernels_Waterfill_Core_64x64_x64_AVX512_H

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x64_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


template <bool AVX512GF>
PA_FORCE_INLINE __m512i bit_reverse(__m512i x);

template <> PA_FORCE_INLINE __m512i bit_reverse<false>(__m512i x){
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


template <bool AVX512GF>
struct Waterfill_x64_AVX512_ProcessedMask;

template <>
struct Waterfill_x64_AVX512_ProcessedMask<false>{
    __m512i m0, m1, m2, m3, m4, m5, m6, m7; //  Copy of the masks.
    __m512i b0, b1, b2, b3, b4, b5, b6, b7; //  Bit-reversed copy of the masks.
    __m512i t0, t1, t2, t3, t4, t5, t6, t7; //  Transposed masks.
    __m512i f1, f2, f3, f4, f5, f6, f7;     //  Forward-carry mask.
    __m512i r0, r1, r2, r3, r4, r5, r6;     //  Reverse-carry mask.

    PA_FORCE_INLINE Waterfill_x64_AVX512_ProcessedMask(
        const BinaryTile_64x64_x64_AVX512& m,
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

        b0 = bit_reverse<false>(m0);
        b1 = bit_reverse<false>(m1);
        b2 = bit_reverse<false>(m2);
        b3 = bit_reverse<false>(m3);
        b4 = bit_reverse<false>(m4);
        b5 = bit_reverse<false>(m5);
        b6 = bit_reverse<false>(m6);
        b7 = bit_reverse<false>(m7);

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
PA_FORCE_INLINE void expand_reverse(__m512i m, __m512i b, __m512i& x){
    __m512i s = bit_reverse<AVX512GF>(_mm512_add_epi64(bit_reverse<AVX512GF>(x), b));
    x = _mm512_ternarylogic_epi64(x, s, m, 0b11110010);
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
);
template <>
PA_FORCE_INLINE void expand_vertical<false>(
    const Waterfill_x64_AVX512_ProcessedMask<false>& mask,
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
PA_FORCE_INLINE void waterfill_expand(const BinaryTile_64x64_x64_AVX512& m, BinaryTile_64x64_x64_AVX512& x){
    __m512i x0 = x.vec[0];
    __m512i x1 = x.vec[1];
    __m512i x2 = x.vec[2];
    __m512i x3 = x.vec[3];
    __m512i x4 = x.vec[4];
    __m512i x5 = x.vec[5];
    __m512i x6 = x.vec[6];
    __m512i x7 = x.vec[7];

    Waterfill_x64_AVX512_ProcessedMask<AVX512GF> mask(m, x0, x1, x2, x3, x4, x5, x6, x7);

//    uint64_t c = 0;
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
//        c++;
    }while (_mm512_test_epi64_mask(changed, changed));

#if 0
    cout << c << endl;
    if (c == 1){
        cout << x.dump() << endl;
    }
    {
        __m512i anything = m.vec[0];
        anything = _mm512_or_si512(anything, m.vec[1]);
        anything = _mm512_or_si512(anything, m.vec[2]);
        anything = _mm512_or_si512(anything, m.vec[3]);
        anything = _mm512_or_si512(anything, m.vec[4]);
        anything = _mm512_or_si512(anything, m.vec[5]);
        anything = _mm512_or_si512(anything, m.vec[6]);
        anything = _mm512_or_si512(anything, m.vec[7]);
        if (!_mm512_test_epi64_mask(anything, anything)){
            cout << "zero tile" << endl;
            return;
        }
    }
    static std::map<uint64_t, uint64_t> counts;
    counts[c]++;
    for (const auto& item : counts){
        cout << item.first << " : " << item.second << endl;
    }
#endif
}






struct Waterfill_x64_AVX512{



static PA_FORCE_INLINE __m512i vec_or(const BinaryTile_64x64_x64_AVX512& tile){
    __m512i v0 = _mm512_or_si512(tile.vec[0], tile.vec[1]);
    __m512i v1 = _mm512_or_si512(tile.vec[2], tile.vec[3]);
    __m512i v2 = _mm512_or_si512(tile.vec[4], tile.vec[5]);
    __m512i v3 = _mm512_or_si512(tile.vec[6], tile.vec[7]);
    v0 = _mm512_or_si512(v0, v1);
    v2 = _mm512_or_si512(v2, v3);
    v0 = _mm512_or_si512(v0, v2);
    return v0;
}
static PA_FORCE_INLINE uint64_t row_or(const BinaryTile_64x64_x64_AVX512& tile){
    return _mm512_reduce_or_epi64(vec_or(tile));
}


//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static PA_FORCE_INLINE bool find_bit(size_t& x, size_t& y, const BinaryTile_64x64_x64_AVX512& tile){
    __m512i anything = vec_or(tile);
    if (!_mm512_test_epi64_mask(anything, anything)){
        return false;
    }
    __m512i row = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    for (size_t c = 0; c < 8; c++){
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
    const BinaryTile_64x64_x64_AVX512& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
){
    uint64_t all_or = row_or(tile);
    trailing_zeros(min_x, all_or);
    max_x = bitlength(all_or);

    __m512i row = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i min = _mm512_set1_epi64(-1);
    __m512i max = _mm512_setzero_si512();
    for (size_t c = 0; c < 8; c++){
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
static PA_FORCE_INLINE __m512i popcount_indexsum(__m512i& sum_index, __m512i x){
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
static PA_FORCE_INLINE uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_64x64_x64_AVX512& tile
){
    __m512i sum_p, sum_x, sum_y;
    __m512i offsets = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    {
        __m512i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[0]);
        sum_p = pop;
        sum_x = sum;
        sum_y = _mm512_mul_epu32(pop, offsets);
    }
    for (size_t c = 1; c < 8; c++){
        __m512i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[c]);
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
static PA_FORCE_INLINE void waterfill_expand(const BinaryTile_64x64_x64_AVX512& m, BinaryTile_64x64_x64_AVX512& x){
    Waterfill::waterfill_expand<false>(m, x);
}



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
static PA_FORCE_INLINE bool waterfill_touch_top(const BinaryTile_64x64_x64_AVX512& mask, BinaryTile_64x64_x64_AVX512& tile, const BinaryTile_64x64_x64_AVX512& border){
    uint64_t available = mask.top() & ~tile.top();
    uint64_t new_bits = available & border.bottom();
    if (new_bits == 0){
        return false;
    }
    tile.top() |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_bottom(const BinaryTile_64x64_x64_AVX512& mask, BinaryTile_64x64_x64_AVX512& tile, const BinaryTile_64x64_x64_AVX512& border){
    uint64_t available = mask.bottom() & ~tile.bottom();
    uint64_t new_bits = available & border.top();
    if (new_bits == 0){
        return false;
    }
    tile.bottom() |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_left(const BinaryTile_64x64_x64_AVX512& mask, BinaryTile_64x64_x64_AVX512& tile, const BinaryTile_64x64_x64_AVX512& border){
    __m512i changed = _mm512_setzero_si512();
    for (size_t c = 0; c < 8; c++){
        __m512i available = _mm512_andnot_si512(tile.vec[c], mask.vec[c]);
        __m512i new_bits = _mm512_and_si512(available, _mm512_srli_epi64(border.vec[c], 63));
        changed = _mm512_or_si512(changed, new_bits);
        tile.vec[c] = _mm512_or_si512(tile.vec[c], new_bits);
    }
    return _mm512_test_epi64_mask(changed, changed);
}
static PA_FORCE_INLINE bool waterfill_touch_right(const BinaryTile_64x64_x64_AVX512& mask, BinaryTile_64x64_x64_AVX512& tile, const BinaryTile_64x64_x64_AVX512& border){
    __m512i changed = _mm512_setzero_si512();
    for (size_t c = 0; c < 8; c++){
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
