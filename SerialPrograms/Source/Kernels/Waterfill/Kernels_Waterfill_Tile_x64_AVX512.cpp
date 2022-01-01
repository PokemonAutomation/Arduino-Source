/*  Waterfill Tile (AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Kernels_Arch.h"
#ifdef PA_Arch_x64_AVX512

#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_Waterfill_Tile_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{

PA_FORCE_INLINE __m512i bit_reverse(__m512i x){
    __m512i r0, r1;

    x = _mm512_shuffle_epi8(
        x,
        _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );

#ifdef PA_Arch_x64_AVX512GF
    return _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);
#else
    r0 = _mm512_srli_epi32(x, 4);
    r1 = _mm512_slli_epi32(x, 4);
//    r0 = _mm512_and_si512(r0, _mm512_set1_epi8((uint8_t)0x0f));
//    r1 = _mm512_and_si512(r1, _mm512_set1_epi8((uint8_t)0xf0));
//    r1 = _mm512_or_si512(r0, r1);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x0f), 0b11011000);

    r0 = _mm512_srli_epi32(r1, 2);
    r1 = _mm512_slli_epi32(r1, 2);
//    r0 = _mm512_and_si512(r0, _mm512_set1_epi8((uint8_t)0x33));
//    r1 = _mm512_and_si512(r1, _mm512_set1_epi8((uint8_t)0xcc));
//    r1 = _mm512_or_si512(r0, r1);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x33), 0b11011000);

    r0 = _mm512_srli_epi32(r1, 1);
    r1 = _mm512_slli_epi32(r1, 1);
//    r0 = _mm512_and_si512(r0, _mm512_set1_epi8((uint8_t)0x55));
//    r1 = _mm512_and_si512(r1, _mm512_set1_epi8((uint8_t)0xaa));
//    r1 = _mm512_or_si512(r0, r1);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x55), 0b11011000);

    return r1;
#endif
}


struct ProcessedMask{
    __m512i m0, m1, m2, m3, m4, m5, m6, m7; //  Copy of the masks.
    __m512i b0, b1, b2, b3, b4, b5, b6, b7; //  Bit-reversed copy of the masks.
    __m512i f1, f2, f3, f4, f5, f6, f7;     //  Forward-carry mask.
    __m512i r0, r1, r2, r3, r4, r5, r6;     //  Reverse-carry mask.

    PA_FORCE_INLINE ProcessedMask(
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

        b0 = bit_reverse(m0);
        b1 = bit_reverse(m1);
        b2 = bit_reverse(m2);
        b3 = bit_reverse(m3);
        b4 = bit_reverse(m4);
        b5 = bit_reverse(m5);
        b6 = bit_reverse(m6);
        b7 = bit_reverse(m7);

        //  Forward carry
        __m512i f0 = m0;
        f1 = _mm512_and_si512(f0, m1);
        f2 = _mm512_and_si512(f1, m2);
        f3 = _mm512_and_si512(f2, m3);
        f4 = _mm512_and_si512(f3, m4);
        f5 = _mm512_and_si512(f4, m5);
        f6 = _mm512_and_si512(f5, m6);
        f7 = _mm512_and_si512(f6, m7);
        transpose_i64_8x8_AVX512(f0, f1, f2, f3, f4, f5, f6, f7);

        //  Reverse carry
        __m512i r7 = m7;
        r6 = _mm512_and_si512(r7, m6);
        r5 = _mm512_and_si512(r6, m5);
        r4 = _mm512_and_si512(r5, m4);
        r3 = _mm512_and_si512(r4, m3);
        r2 = _mm512_and_si512(r3, m2);
        r1 = _mm512_and_si512(r2, m1);
        r0 = _mm512_and_si512(r1, m0);
        transpose_i64_8x8_AVX512(r0, r1, r2, r3, r4, r5, r6, r7);
    }
};


PA_FORCE_INLINE void expand_reverse(__m512i m, __m512i b, __m512i& x){
    __m512i s = bit_reverse(_mm512_add_epi64(bit_reverse(x), b));
    x = _mm512_ternarylogic_epi64(x, s, m, 0b11110010);
}

PA_FORCE_INLINE void expand_forward(
    const ProcessedMask& mask,
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
PA_FORCE_INLINE void expand_reverse(
    const ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    expand_reverse(mask.m0, mask.b0, x0);
    expand_reverse(mask.m1, mask.b1, x1);
    expand_reverse(mask.m2, mask.b2, x2);
    expand_reverse(mask.m3, mask.b3, x3);
    expand_reverse(mask.m4, mask.b4, x4);
    expand_reverse(mask.m5, mask.b5, x5);
    expand_reverse(mask.m6, mask.b6, x6);
    expand_reverse(mask.m7, mask.b7, x7);
}




PA_FORCE_INLINE void expand_vertical(
    const ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
#if 0
    //  Carry across adjacent rows.
    x1 = _mm512_or_si512(x1, _mm512_and_si512(x0, mask.m1));
    x6 = _mm512_or_si512(x6, _mm512_and_si512(x7, mask.m6));
    x2 = _mm512_or_si512(x2, _mm512_and_si512(x1, mask.m2));
    x5 = _mm512_or_si512(x5, _mm512_and_si512(x6, mask.m5));
    x3 = _mm512_or_si512(x3, _mm512_and_si512(x2, mask.m3));
    x4 = _mm512_or_si512(x4, _mm512_and_si512(x5, mask.m4));
    x4 = _mm512_or_si512(x4, _mm512_and_si512(x3, mask.m4));
    x3 = _mm512_or_si512(x3, _mm512_and_si512(x4, mask.m3));
    x5 = _mm512_or_si512(x5, _mm512_and_si512(x4, mask.m5));
    x2 = _mm512_or_si512(x2, _mm512_and_si512(x3, mask.m2));
    x6 = _mm512_or_si512(x6, _mm512_and_si512(x5, mask.m6));
    x1 = _mm512_or_si512(x1, _mm512_and_si512(x2, mask.m1));
    x7 = _mm512_or_si512(x7, _mm512_and_si512(x6, mask.m7));
    x0 = _mm512_or_si512(x0, _mm512_and_si512(x1, mask.m0));

    //  Carry across groups of 4 rows.
    transpose_i64_8x8_AVX512(x0, x1, x2, x3, x4, x5, x6, x7);
    x1 = _mm512_or_si512(x1, _mm512_and_si512(_mm512_permutexvar_epi64(_mm512_set1_epi64(7), x0), mask.f1));
    x6 = _mm512_or_si512(x6, _mm512_and_si512(_mm512_broadcastq_epi64(_mm512_castsi512_si128(x7)), mask.r6));
    x2 = _mm512_or_si512(x2, _mm512_and_si512(_mm512_permutexvar_epi64(_mm512_set1_epi64(7), x1), mask.f2));
    x5 = _mm512_or_si512(x5, _mm512_and_si512(_mm512_broadcastq_epi64(_mm512_castsi512_si128(x6)), mask.r5));
    x3 = _mm512_or_si512(x3, _mm512_and_si512(_mm512_permutexvar_epi64(_mm512_set1_epi64(7), x2), mask.f3));
    x4 = _mm512_or_si512(x4, _mm512_and_si512(_mm512_broadcastq_epi64(_mm512_castsi512_si128(x5)), mask.r4));
    x4 = _mm512_or_si512(x4, _mm512_and_si512(_mm512_permutexvar_epi64(_mm512_set1_epi64(7), x3), mask.f4));
    x3 = _mm512_or_si512(x3, _mm512_and_si512(_mm512_broadcastq_epi64(_mm512_castsi512_si128(x4)), mask.r3));
    x5 = _mm512_or_si512(x5, _mm512_and_si512(_mm512_permutexvar_epi64(_mm512_set1_epi64(7), x4), mask.f5));
    x2 = _mm512_or_si512(x2, _mm512_and_si512(_mm512_broadcastq_epi64(_mm512_castsi512_si128(x3)), mask.r2));
    x6 = _mm512_or_si512(x6, _mm512_and_si512(_mm512_permutexvar_epi64(_mm512_set1_epi64(7), x5), mask.f6));
    x1 = _mm512_or_si512(x1, _mm512_and_si512(_mm512_broadcastq_epi64(_mm512_castsi512_si128(x2)), mask.r1));
    x7 = _mm512_or_si512(x7, _mm512_and_si512(_mm512_permutexvar_epi64(_mm512_set1_epi64(7), x6), mask.f7));
    x0 = _mm512_or_si512(x0, _mm512_and_si512(_mm512_broadcastq_epi64(_mm512_castsi512_si128(x1)), mask.r0));
    transpose_i64_8x8_AVX512(x0, x1, x2, x3, x4, x5, x6, x7);
#else
    //  Carry across adjacent rows.
    x1 = _mm512_ternarylogic_epi64(x1, x0, mask.m1, 0b11111000);
    x6 = _mm512_ternarylogic_epi64(x6, x7, mask.m6, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, x1, mask.m2, 0b11111000);
    x5 = _mm512_ternarylogic_epi64(x5, x6, mask.m5, 0b11111000);
    x3 = _mm512_ternarylogic_epi64(x3, x2, mask.m3, 0b11111000);
    x4 = _mm512_ternarylogic_epi64(x4, x5, mask.m4, 0b11111000);
    x4 = _mm512_ternarylogic_epi64(x4, x3, mask.m4, 0b11111000);
    x3 = _mm512_ternarylogic_epi64(x3, x4, mask.m3, 0b11111000);
    x5 = _mm512_ternarylogic_epi64(x5, x4, mask.m5, 0b11111000);
    x2 = _mm512_ternarylogic_epi64(x2, x3, mask.m2, 0b11111000);
    x6 = _mm512_ternarylogic_epi64(x6, x5, mask.m6, 0b11111000);
    x1 = _mm512_ternarylogic_epi64(x1, x2, mask.m1, 0b11111000);
    x7 = _mm512_ternarylogic_epi64(x7, x6, mask.m7, 0b11111000);
    x0 = _mm512_ternarylogic_epi64(x0, x1, mask.m0, 0b11111000);

    //  Carry across groups of 4 rows.
    transpose_i64_8x8_AVX512(x0, x1, x2, x3, x4, x5, x6, x7);
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
    transpose_i64_8x8_AVX512(x0, x1, x2, x3, x4, x5, x6, x7);
#endif
}

void waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x){
    __m512i x0 = x.vec[0];
    __m512i x1 = x.vec[1];
    __m512i x2 = x.vec[2];
    __m512i x3 = x.vec[3];
    __m512i x4 = x.vec[4];
    __m512i x5 = x.vec[5];
    __m512i x6 = x.vec[6];
    __m512i x7 = x.vec[7];

    ProcessedMask mask(m, x0, x1, x2, x3, x4, x5, x6, x7);

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
    }while (_mm512_cmpneq_epi64_mask(changed, _mm512_setzero_si512()));
}




}
}
#endif
