/*  ABS FFT Base Transform (x86 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_BaseTransform_x86_AVX512_H
#define PokemonAutomation_Kernels_AbsFFT_BaseTransform_x86_AVX512_H

//#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_AbsFFT_Arch_x86_AVX512.h"
#include "Kernels_AbsFFT_Butterflies.h"
#include "Kernels_AbsFFT_ComplexVector.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


PA_FORCE_INLINE void vtranspose(
    __m512& r00, __m512& r01, __m512& r02, __m512& r03,
    __m512& r04, __m512& r05, __m512& r06, __m512& r07,
    __m512& r08, __m512& r09, __m512& r10, __m512& r11,
    __m512& r12, __m512& r13, __m512& r14, __m512& r15
){
    __m512 s00, s01, s02, s03, s04, s05, s06, s07, s08, s09, s10, s11, s12, s13, s14, s15;

    s00 = _mm512_unpacklo_ps(r00, r01);
    s01 = _mm512_unpackhi_ps(r00, r01);
    s02 = _mm512_unpacklo_ps(r02, r03);
    s03 = _mm512_unpackhi_ps(r02, r03);
    s04 = _mm512_unpacklo_ps(r04, r05);
    s05 = _mm512_unpackhi_ps(r04, r05);
    s06 = _mm512_unpacklo_ps(r06, r07);
    s07 = _mm512_unpackhi_ps(r06, r07);
    s08 = _mm512_unpacklo_ps(r08, r09);
    s09 = _mm512_unpackhi_ps(r08, r09);
    s10 = _mm512_unpacklo_ps(r10, r11);
    s11 = _mm512_unpackhi_ps(r10, r11);
    s12 = _mm512_unpacklo_ps(r12, r13);
    s13 = _mm512_unpackhi_ps(r12, r13);
    s14 = _mm512_unpacklo_ps(r14, r15);
    s15 = _mm512_unpackhi_ps(r14, r15);

    r00 = _mm512_shuffle_ps(s00, s02, 68);
    r01 = _mm512_shuffle_ps(s00, s02, 238);
    r02 = _mm512_shuffle_ps(s01, s03, 68);
    r03 = _mm512_shuffle_ps(s01, s03, 238);
    r04 = _mm512_shuffle_ps(s04, s06, 68);
    r05 = _mm512_shuffle_ps(s04, s06, 238);
    r06 = _mm512_shuffle_ps(s05, s07, 68);
    r07 = _mm512_shuffle_ps(s05, s07, 238);
    r08 = _mm512_shuffle_ps(s08, s10, 68);
    r09 = _mm512_shuffle_ps(s08, s10, 238);
    r10 = _mm512_shuffle_ps(s09, s11, 68);
    r11 = _mm512_shuffle_ps(s09, s11, 238);
    r12 = _mm512_shuffle_ps(s12, s14, 68);
    r13 = _mm512_shuffle_ps(s12, s14, 238);
    r14 = _mm512_shuffle_ps(s13, s15, 68);
    r15 = _mm512_shuffle_ps(s13, s15, 238);

    s00 = _mm512_shuffle_f32x4(r00, r04, 136);
    s01 = _mm512_shuffle_f32x4(r01, r05, 136);
    s02 = _mm512_shuffle_f32x4(r00, r04, 221);
    s03 = _mm512_shuffle_f32x4(r01, r05, 221);
    s04 = _mm512_shuffle_f32x4(r02, r06, 136);
    s05 = _mm512_shuffle_f32x4(r03, r07, 136);
    s06 = _mm512_shuffle_f32x4(r02, r06, 221);
    s07 = _mm512_shuffle_f32x4(r03, r07, 221);
    s08 = _mm512_shuffle_f32x4(r08, r12, 136);
    s09 = _mm512_shuffle_f32x4(r09, r13, 136);
    s10 = _mm512_shuffle_f32x4(r08, r12, 221);
    s11 = _mm512_shuffle_f32x4(r09, r13, 221);
    s12 = _mm512_shuffle_f32x4(r10, r14, 136);
    s13 = _mm512_shuffle_f32x4(r11, r15, 136);
    s14 = _mm512_shuffle_f32x4(r10, r14, 221);
    s15 = _mm512_shuffle_f32x4(r11, r15, 221);

    r00 = _mm512_shuffle_f32x4(s00, s08, 136);
    r01 = _mm512_shuffle_f32x4(s01, s09, 136);
    r02 = _mm512_shuffle_f32x4(s04, s12, 136);
    r03 = _mm512_shuffle_f32x4(s05, s13, 136);
    r04 = _mm512_shuffle_f32x4(s02, s10, 136);
    r05 = _mm512_shuffle_f32x4(s03, s11, 136);
    r06 = _mm512_shuffle_f32x4(s06, s14, 136);
    r07 = _mm512_shuffle_f32x4(s07, s15, 136);
    r08 = _mm512_shuffle_f32x4(s00, s08, 221);
    r09 = _mm512_shuffle_f32x4(s01, s09, 221);
    r10 = _mm512_shuffle_f32x4(s04, s12, 221);
    r11 = _mm512_shuffle_f32x4(s05, s13, 221);
    r12 = _mm512_shuffle_f32x4(s02, s10, 221);
    r13 = _mm512_shuffle_f32x4(s03, s11, 221);
    r14 = _mm512_shuffle_f32x4(s06, s14, 221);
    r15 = _mm512_shuffle_f32x4(s07, s15, 221);
}


template <>
void base_transform<Context_x86_AVX512>(const TwiddleTable<Context_x86_AVX512>& table, Context_x86_AVX512::vtype* T){
    __m512 r00, r01, r02, r03, r04, r05, r06, r07, r08, r09, r10, r11, r12, r13, r14, r15;
    __m512 i00, i01, i02, i03, i04, i05, i06, i07, i08, i09, i10, i11, i12, i13, i14, i15;

    r00 = T[ 0];
    i00 = T[ 1];
    r01 = T[ 2];
    i01 = T[ 3];
    r02 = T[ 4];
    i02 = T[ 5];
    r03 = T[ 6];
    i03 = T[ 7];
    r04 = T[ 8];
    i04 = T[ 9];
    r05 = T[10];
    i05 = T[11];
    r06 = T[12];
    i06 = T[13];
    r07 = T[14];
    i07 = T[15];
    r08 = T[16];
    i08 = T[17];
    r09 = T[18];
    i09 = T[19];
    r10 = T[20];
    i10 = T[21];
    r11 = T[22];
    i11 = T[23];
    r12 = T[24];
    i12 = T[25];
    r13 = T[26];
    i13 = T[27];
    r14 = T[28];
    i14 = T[29];
    r15 = T[30];
    i15 = T[31];


    {
        const vcomplex<Context_x86_AVX512>* w1 = table[7].w1.data();
        const vcomplex<Context_x86_AVX512>* w2 = table[8].w1.data();
        const vcomplex<Context_x86_AVX512>* w3 = table[8].w3.data();
        Butterflies<Context_x86_AVX512>::butterfly4(
            r00, i00,
            r04, i04, w1[0].r, w1[0].i,
            r08, i08, w2[0].r, w2[0].i,
            r12, i12, w3[0].r, w3[0].i
        );
        Butterflies<Context_x86_AVX512>::butterfly4(
            r01, i01,
            r05, i05, w1[1].r, w1[1].i,
            r09, i09, w2[1].r, w2[1].i,
            r13, i13, w3[1].r, w3[1].i
        );
        Butterflies<Context_x86_AVX512>::butterfly4(
            r02, i02,
            r06, i06, w1[2].r, w1[2].i,
            r10, i10, w2[2].r, w2[2].i,
            r14, i14, w3[2].r, w3[2].i
        );
        Butterflies<Context_x86_AVX512>::butterfly4(
            r03, i03,
            r07, i07, w1[3].r, w1[3].i,
            r11, i11, w2[3].r, w2[3].i,
            r15, i15, w3[3].r, w3[3].i
        );
    }
    {
        const vcomplex<Context_x86_AVX512>* w1 = table[5].w1.data();
        const vcomplex<Context_x86_AVX512>* w2 = table[6].w1.data();
        const vcomplex<Context_x86_AVX512>* w3 = table[6].w3.data();
        Butterflies<Context_x86_AVX512>::butterfly4(
            r00, i00,
            r01, i01, w1[0].r, w1[0].i,
            r02, i02, w2[0].r, w2[0].i,
            r03, i03, w3[0].r, w3[0].i
        );
        Butterflies<Context_x86_AVX512>::butterfly4(
            r04, i04,
            r05, i05, w1[0].r, w1[0].i,
            r06, i06, w2[0].r, w2[0].i,
            r07, i07, w3[0].r, w3[0].i
        );
        Butterflies<Context_x86_AVX512>::butterfly4(
            r08, i08,
            r09, i09, w1[0].r, w1[0].i,
            r10, i10, w2[0].r, w2[0].i,
            r11, i11, w3[0].r, w3[0].i
        );
        Butterflies<Context_x86_AVX512>::butterfly4(
            r12, i12,
            r13, i13, w1[0].r, w1[0].i,
            r14, i14, w2[0].r, w2[0].i,
            r15, i15, w3[0].r, w3[0].i
        );
    }

    vtranspose(r00, r01, r02, r03, r04, r05, r06, r07, r08, r09, r10, r11, r12, r13, r14, r15);
    vtranspose(i00, i01, i02, i03, i04, i05, i06, i07, i08, i09, i10, i11, i12, i13, i14, i15);

    Butterflies<Context_x86_AVX512>::butterfly4(
        r00, i00,
        r04, i04,
        r08, i08,
        r12, i12
    );
    Butterflies<Context_x86_AVX512>::butterfly4(
        r01, i01,
        r05, i05, Context_x86_AVX512::vset1(TW8_1), Context_x86_AVX512::vset1(TW8_1),
        r09, i09, Context_x86_AVX512::vset1(TW16_1), Context_x86_AVX512::vset1(TW16_3),
        r13, i13, Context_x86_AVX512::vset1(TW16_3), Context_x86_AVX512::vset1(TW16_1)
    );
    Butterflies<Context_x86_AVX512>::butterfly4(
        r02, i02,
        r06, i06, Context_x86_AVX512::vset1(0), Context_x86_AVX512::vset1(1),
        r10, i10, Context_x86_AVX512::vset1(TW8_1), Context_x86_AVX512::vset1(TW8_1),
        r14, i14, Context_x86_AVX512::vset1(-TW8_1), Context_x86_AVX512::vset1(TW8_1)
    );
    Butterflies<Context_x86_AVX512>::butterfly4(
        r03, i03,
        r07, i07, Context_x86_AVX512::vset1(-TW8_1), Context_x86_AVX512::vset1(TW8_1),
        r11, i11, Context_x86_AVX512::vset1(TW16_3), Context_x86_AVX512::vset1(TW16_1),
        r15, i15, Context_x86_AVX512::vset1(-TW16_1), Context_x86_AVX512::vset1(-TW16_3)
    );

    Butterflies<Context_x86_AVX512>::butterfly4(
        r00, i00,
        r01, i01,
        r02, i02,
        r03, i03
    );
    Butterflies<Context_x86_AVX512>::butterfly4(
        r04, i04,
        r05, i05,
        r06, i06,
        r07, i07
    );
    Butterflies<Context_x86_AVX512>::butterfly4(
        r08, i08,
        r09, i09,
        r10, i10,
        r11, i11
    );
    Butterflies<Context_x86_AVX512>::butterfly4(
        r12, i12,
        r13, i13,
        r14, i14,
        r15, i15
    );

    vtranspose(r00, r01, r02, r03, r04, r05, r06, r07, r08, r09, r10, r11, r12, r13, r14, r15);
    T[ 0] = r00;
    T[ 2] = r01;
    T[ 4] = r02;
    T[ 6] = r03;
    T[ 8] = r04;
    T[10] = r05;
    T[12] = r06;
    T[14] = r07;
    T[16] = r08;
    T[18] = r09;
    T[20] = r10;
    T[22] = r11;
    T[24] = r12;
    T[26] = r13;
    T[28] = r14;
    T[30] = r15;
    vtranspose(i00, i01, i02, i03, i04, i05, i06, i07, i08, i09, i10, i11, i12, i13, i14, i15);
    T[ 1] = i00;
    T[ 3] = i01;
    T[ 5] = i02;
    T[ 7] = i03;
    T[ 9] = i04;
    T[11] = i05;
    T[13] = i06;
    T[15] = i07;
    T[17] = i08;
    T[19] = i09;
    T[21] = i10;
    T[23] = i11;
    T[25] = i12;
    T[27] = i13;
    T[29] = i14;
    T[31] = i15;
}



}
}
}
#endif
