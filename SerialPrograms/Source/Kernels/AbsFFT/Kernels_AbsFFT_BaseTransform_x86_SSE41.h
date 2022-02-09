/*  ABS FFT Base Transform (x86 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_BaseTransform_x86_SSE41_H
#define PokemonAutomation_Kernels_AbsFFT_BaseTransform_x86_SSE41_H

#include "Kernels_AbsFFT_Butterflies.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{

PA_FORCE_INLINE void vtranspose(__m128& r0, __m128& r1, __m128& r2, __m128& r3){
    __m128 a0, a1, a2, a3;
    a0 = _mm_unpacklo_ps(r0, r1);
    a1 = _mm_unpackhi_ps(r0, r1);
    a2 = _mm_unpacklo_ps(r2, r3);
    a3 = _mm_unpackhi_ps(r2, r3);
    r0 = _mm_shuffle_ps(a0, a2, 68);
    r1 = _mm_shuffle_ps(a0, a2, 238);
    r2 = _mm_shuffle_ps(a1, a3, 68);
    r3 = _mm_shuffle_ps(a1, a3, 238);
}

void base_transform(const TwiddleTable& table, vtype T[8]){
    vtype r0, r1, r2, r3;
    vtype i0, i1, i2, i3;

    r0 = T[0];
    i0 = T[1];
    r1 = T[2];
    i1 = T[3];
    r2 = T[4];
    r3 = T[6];
    i2 = T[5];
    i3 = T[7];

    const vcomplex* w1 = table[3].w1.data();
    const vcomplex* w2 = table[4].w1.data();
    const vcomplex* w3 = table[4].w3.data();
    Butterflies<Intrinsics_x86_SSE41>::butterfly4(
        r0, i0,
        r1, i1, w1[0].r, w1[0].i,
        r2, i2, w2[0].r, w2[0].i,
        r3, i3, w3[0].r, w3[0].i
    );

    vtranspose(r0, r1, r2, r3);
    vtranspose(i0, i1, i2, i3);

    Butterflies<Intrinsics_x86_SSE41>::butterfly4(
        r0, i0,
        r1, i1,
        r2, i2,
        r3, i3
    );

    vtranspose(r0, r1, r2, r3);
    T[0] = r0;
    T[2] = r1;
    T[4] = r2;
    T[6] = r3;
    vtranspose(i0, i1, i2, i3);
    T[1] = i0;
    T[3] = i1;
    T[5] = i2;
    T[7] = i3;
}



}
}
}
#endif
