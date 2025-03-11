/*  ABS FFT Base Transform (x86 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_BaseTransform_x86_AVX2_H
#define PokemonAutomation_Kernels_AbsFFT_BaseTransform_x86_AVX2_H

#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels_AbsFFT_Arch_x86_AVX2.h"
#include "Kernels_AbsFFT_Butterflies.h"
#include "Kernels_AbsFFT_ComplexVector.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{

PA_FORCE_INLINE void vtranspose(
    __m256& r0, __m256& r1, __m256& r2, __m256& r3,
    __m256& r4, __m256& r5, __m256& r6, __m256& r7
){
    __m256 a0, a1, a2, a3, a4, a5, a6, a7;
    __m256 b0, b1, b2, b3, b4, b5, b6, b7;

    a0 = _mm256_unpacklo_ps(r0, r1);
    a1 = _mm256_unpackhi_ps(r0, r1);
    a2 = _mm256_unpacklo_ps(r2, r3);
    a3 = _mm256_unpackhi_ps(r2, r3);
    a4 = _mm256_unpacklo_ps(r4, r5);
    a5 = _mm256_unpackhi_ps(r4, r5);
    a6 = _mm256_unpacklo_ps(r6, r7);
    a7 = _mm256_unpackhi_ps(r6, r7);

    b0 = _mm256_shuffle_ps(a0, a2, 68);
    b1 = _mm256_shuffle_ps(a0, a2, 238);
    b2 = _mm256_shuffle_ps(a1, a3, 68);
    b3 = _mm256_shuffle_ps(a1, a3, 238);
    b4 = _mm256_shuffle_ps(a4, a6, 68);
    b5 = _mm256_shuffle_ps(a4, a6, 238);
    b6 = _mm256_shuffle_ps(a5, a7, 68);
    b7 = _mm256_shuffle_ps(a5, a7, 238);

    r0 = _mm256_permute2f128_ps(b0, b4, 32);
    r4 = _mm256_permute2f128_ps(b0, b4, 49);
    r1 = _mm256_permute2f128_ps(b1, b5, 32);
    r5 = _mm256_permute2f128_ps(b1, b5, 49);
    r2 = _mm256_permute2f128_ps(b2, b6, 32);
    r6 = _mm256_permute2f128_ps(b2, b6, 49);
    r3 = _mm256_permute2f128_ps(b3, b7, 32);
    r7 = _mm256_permute2f128_ps(b3, b7, 49);
}

template <>
void base_transform<Context_x86_AVX2>(const TwiddleTable<Context_x86_AVX2>& table, Context_x86_AVX2::vtype* T){
    __m256 r0, r1, r2, r3, r4, r5, r6, r7;
    __m256 i0, i1, i2, i3, i4, i5, i6, i7;

    r0 = T[ 0];
    i0 = T[ 1];
    r1 = T[ 2];
    i1 = T[ 3];
    r2 = T[ 4];
    i2 = T[ 5];
    r3 = T[ 6];
    i3 = T[ 7];
    r4 = T[ 8];
    i4 = T[ 9];
    r5 = T[10];
    i5 = T[11];
    r6 = T[12];
    i6 = T[13];
    r7 = T[14];
    i7 = T[15];

    {
        const vcomplex<Context_x86_AVX2>* w1 = table[6].w1.data();
        Butterflies<Context_x86_AVX2>::butterfly2(r0, i0, r4, i4, w1[0].r, w1[0].i);
        Butterflies<Context_x86_AVX2>::butterfly2(r1, i1, r5, i5, w1[1].r, w1[1].i);
        Butterflies<Context_x86_AVX2>::butterfly2(r2, i2, r6, i6, w1[2].r, w1[2].i);
        Butterflies<Context_x86_AVX2>::butterfly2(r3, i3, r7, i7, w1[3].r, w1[3].i);
    }
    {
        const vcomplex<Context_x86_AVX2>* w1 = table[4].w1.data();
        const vcomplex<Context_x86_AVX2>* w2 = table[5].w1.data();
        const vcomplex<Context_x86_AVX2>* w3 = table[5].w3.data();
        Butterflies<Context_x86_AVX2>::butterfly4(
            r0, i0,
            r1, i1, w1[0].r, w1[0].i,
            r2, i2, w2[0].r, w2[0].i,
            r3, i3, w3[0].r, w3[0].i
        );
        Butterflies<Context_x86_AVX2>::butterfly4(
            r4, i4,
            r5, i5, w1[0].r, w1[0].i,
            r6, i6, w2[0].r, w2[0].i,
            r7, i7, w3[0].r, w3[0].i
        );
    }

    vtranspose(r0, r1, r2, r3, r4, r5, r6, r7);
    vtranspose(i0, i1, i2, i3, i4, i5, i6, i7);

    Butterflies<Context_x86_AVX2>::butterfly2(r0, i0, r4, i4);
    Butterflies<Context_x86_AVX2>::butterfly2(r1, i1, r5, i5, Context_x86_AVX2::vset1(TW8_1), Context_x86_AVX2::vset1(TW8_1));
    Butterflies<Context_x86_AVX2>::butterfly2(r2, i2, r6, i6, Context_x86_AVX2::vset1(0), Context_x86_AVX2::vset1(1));
    Butterflies<Context_x86_AVX2>::butterfly2(r3, i3, r7, i7, Context_x86_AVX2::vset1(-TW8_1), Context_x86_AVX2::vset1(TW8_1));
    Butterflies<Context_x86_AVX2>::butterfly4(
        r0, i0,
        r1, i1,
        r2, i2,
        r3, i3
    );
    Butterflies<Context_x86_AVX2>::butterfly4(
        r4, i4,
        r5, i5,
        r6, i6,
        r7, i7
    );

    vtranspose(r0, r1, r2, r3, r4, r5, r6, r7);
    T[ 0] = r0;
    T[ 2] = r1;
    T[ 4] = r2;
    T[ 6] = r3;
    T[ 8] = r4;
    T[10] = r5;
    T[12] = r6;
    T[14] = r7;
    vtranspose(i0, i1, i2, i3, i4, i5, i6, i7);
    T[ 1] = i0;
    T[ 3] = i1;
    T[ 5] = i2;
    T[ 7] = i3;
    T[ 9] = i4;
    T[11] = i5;
    T[13] = i6;
    T[15] = i7;
}



}
}
}
#endif
