/*  Waterfill Core (x64 AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64x64_x64_AVX512GF_H
#define PokemonAutomation_Kernels_Waterfill_Core_64x64_x64_AVX512GF_H

#include "Kernels_Waterfill_Core_64x64_x64_AVX512.h"
#include "Kernels_Waterfill_Intrinsics_x64_AVX512.h"
#include "Kernels_Waterfill_Intrinsics_x64_AVX512-GF.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


struct Waterfill_64x64_x64_AVX512GF_ProcessedMask{
    __m512i m0, m1, m2, m3, m4, m5, m6, m7; //  Copy of the masks.
    __m512i b0, b1, b2, b3, b4, b5, b6, b7; //  Bit-reversed copy of the masks.
    __m512i f0, f1, f2, f3, f4, f5, f6, f7; //  Forward transposed.
    __m512i r0, r1, r2, r3, r4, r5, r6, r7; //  Reverse transposed.

    PA_FORCE_INLINE Waterfill_64x64_x64_AVX512GF_ProcessedMask(
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

        b0 = Intrinsics_x64_AVX512GF::bit_reverse(m0);
        b1 = Intrinsics_x64_AVX512GF::bit_reverse(m1);
        b2 = Intrinsics_x64_AVX512GF::bit_reverse(m2);
        b3 = Intrinsics_x64_AVX512GF::bit_reverse(m3);
        b4 = Intrinsics_x64_AVX512GF::bit_reverse(m4);
        b5 = Intrinsics_x64_AVX512GF::bit_reverse(m5);
        b6 = Intrinsics_x64_AVX512GF::bit_reverse(m6);
        b7 = Intrinsics_x64_AVX512GF::bit_reverse(m7);

        f0 = m0;
        f1 = m1;
        f2 = m2;
        f3 = m3;
        f4 = m4;
        f5 = m5;
        f6 = m6;
        f7 = m7;
        Intrinsics_x64_AVX512GF::transpose_1x64x64(f0, f1, f2, f3, f4, f5, f6, f7);
        r0 = Intrinsics_x64_AVX512GF::bit_reverse(f0);
        r1 = Intrinsics_x64_AVX512GF::bit_reverse(f1);
        r2 = Intrinsics_x64_AVX512GF::bit_reverse(f2);
        r3 = Intrinsics_x64_AVX512GF::bit_reverse(f3);
        r4 = Intrinsics_x64_AVX512GF::bit_reverse(f4);
        r5 = Intrinsics_x64_AVX512GF::bit_reverse(f5);
        r6 = Intrinsics_x64_AVX512GF::bit_reverse(f6);
        r7 = Intrinsics_x64_AVX512GF::bit_reverse(f7);
    }
};



PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64x64_x64_AVX512GF_ProcessedMask& mask,
    __m512i& x0, __m512i& x1, __m512i& x2, __m512i& x3,
    __m512i& x4, __m512i& x5, __m512i& x6, __m512i& x7
){
    Intrinsics_x64_AVX512GF::transpose_1x64x64(x0, x1, x2, x3, x4, x5, x6, x7);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.f0), mask.f0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.f1), mask.f1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.f2), mask.f2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.f3), mask.f3, 0b11110010);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_add_epi64(x4, mask.f4), mask.f4, 0b11110010);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_add_epi64(x5, mask.f5), mask.f5, 0b11110010);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_add_epi64(x6, mask.f6), mask.f6, 0b11110010);
    x7 = _mm512_ternarylogic_epi64(x7, _mm512_add_epi64(x7, mask.f7), mask.f7, 0b11110010);
    x0 = Intrinsics_x64_AVX512GF::bit_reverse(x0);
    x1 = Intrinsics_x64_AVX512GF::bit_reverse(x1);
    x2 = Intrinsics_x64_AVX512GF::bit_reverse(x2);
    x3 = Intrinsics_x64_AVX512GF::bit_reverse(x3);
    x4 = Intrinsics_x64_AVX512GF::bit_reverse(x4);
    x5 = Intrinsics_x64_AVX512GF::bit_reverse(x5);
    x6 = Intrinsics_x64_AVX512GF::bit_reverse(x6);
    x7 = Intrinsics_x64_AVX512GF::bit_reverse(x7);
    x0 = _mm512_ternarylogic_epi64(x0, _mm512_add_epi64(x0, mask.r0), mask.r0, 0b11110010);
    x1 = _mm512_ternarylogic_epi64(x1, _mm512_add_epi64(x1, mask.r1), mask.r1, 0b11110010);
    x2 = _mm512_ternarylogic_epi64(x2, _mm512_add_epi64(x2, mask.r2), mask.r2, 0b11110010);
    x3 = _mm512_ternarylogic_epi64(x3, _mm512_add_epi64(x3, mask.r3), mask.r3, 0b11110010);
    x4 = _mm512_ternarylogic_epi64(x4, _mm512_add_epi64(x4, mask.r4), mask.r4, 0b11110010);
    x5 = _mm512_ternarylogic_epi64(x5, _mm512_add_epi64(x5, mask.r5), mask.r5, 0b11110010);
    x6 = _mm512_ternarylogic_epi64(x6, _mm512_add_epi64(x6, mask.r6), mask.r6, 0b11110010);
    x7 = _mm512_ternarylogic_epi64(x7, _mm512_add_epi64(x7, mask.r7), mask.r7, 0b11110010);
    Intrinsics_x64_AVX512GF::transpose_1x64x64_bitreverse_in(x0, x1, x2, x3, x4, x5, x6, x7);
}







struct Waterfill_64x64_x64_AVX512GF : public Waterfill_64x64_x64_AVX512{



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
PA_FORCE_INLINE static void waterfill_expand(BinaryTile_64x64_x64_AVX512& m, BinaryTile_64x64_x64_AVX512& x){
    __m512i x0 = x.vec[0];
    __m512i x1 = x.vec[1];
    __m512i x2 = x.vec[2];
    __m512i x3 = x.vec[3];
    __m512i x4 = x.vec[4];
    __m512i x5 = x.vec[5];
    __m512i x6 = x.vec[6];
    __m512i x7 = x.vec[7];

    Waterfill_64x64_x64_AVX512GF_ProcessedMask mask(m, x0, x1, x2, x3, x4, x5, x6, x7);
    Intrinsics_x64_AVX512::expand_forward(mask, x0, x1, x2, x3, x4, x5, x6, x7);

    __m512i m0, m1, m2, m3, m4, m5, m6, m7;
    do{
        expand_vertical(mask, x0, x1, x2, x3, x4, x5, x6, x7);
        Intrinsics_x64_AVX512GF::expand_reverse(mask, x0, x1, x2, x3, x4, x5, x6, x7);
        Intrinsics_x64_AVX512::expand_forward(mask, x0, x1, x2, x3, x4, x5, x6, x7);
    }while (Intrinsics_x64_AVX512::keep_going(
        mask,
        m0, m1, m2, m3, m4, m5, m6, m7,
        x0, x1, x2, x3, x4, x5, x6, x7
    ));
    x.vec[0] = x0;
    x.vec[1] = x1;
    x.vec[2] = x2;
    x.vec[3] = x3;
    x.vec[4] = x4;
    x.vec[5] = x5;
    x.vec[6] = x6;
    x.vec[7] = x7;
    m.vec[0] = m0;
    m.vec[1] = m1;
    m.vec[2] = m2;
    m.vec[3] = m3;
    m.vec[4] = m4;
    m.vec[5] = m5;
    m.vec[6] = m6;
    m.vec[7] = m7;
}




};



}
}
}
#endif
