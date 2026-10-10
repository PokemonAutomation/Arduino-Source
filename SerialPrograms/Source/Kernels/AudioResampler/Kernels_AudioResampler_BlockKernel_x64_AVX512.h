/*  Audio Resampler Block Kernel (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BlockKernel_x64_AVX512_H
#define PokemonAutomation_Kernels_BlockKernel_x64_AVX512_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{


//
//  This is the fast kernel. But it only works for:
//    - Upsampling
//    - Downsampling by no more than 2x
//
//  Opcount Ratios:
//    - 1 x FMA
//    - 1 x shuffle
//    - 1 x aligned 512-bit load
//    - 1 x misaligned 512-bit load
//
template <int input_loads>
inline void run_block128_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    in_samples -= in_index;

    const uint32_t* index = table.in_sample_index() + out_index;
    const uint32_t* index0 = index + 16*0;
    const uint32_t* index1 = index + 16*1;
    const uint32_t* index2 = index + 16*2;
    const uint32_t* index3 = index + 16*3;
    const uint32_t* index4 = index + 16*4;
    const uint32_t* index5 = index + 16*5;
    const uint32_t* index6 = index + 16*6;
    const uint32_t* index7 = index + 16*7;

    __m512i shuffle0 = _mm512_loadu_si512(index0);
    __m512i shuffle1 = _mm512_loadu_si512(index1);
    __m512i shuffle2 = _mm512_loadu_si512(index2);
    __m512i shuffle3 = _mm512_loadu_si512(index3);
    __m512i shuffle4 = _mm512_loadu_si512(index4);
    __m512i shuffle5 = _mm512_loadu_si512(index5);
    __m512i shuffle6 = _mm512_loadu_si512(index6);
    __m512i shuffle7 = _mm512_loadu_si512(index7);

    shuffle0 = _mm512_sub_epi32(shuffle0, _mm512_set1_epi32(index0[0]));
    shuffle1 = _mm512_sub_epi32(shuffle1, _mm512_set1_epi32(index1[0]));
    shuffle2 = _mm512_sub_epi32(shuffle2, _mm512_set1_epi32(index2[0]));
    shuffle3 = _mm512_sub_epi32(shuffle3, _mm512_set1_epi32(index3[0]));
    shuffle4 = _mm512_sub_epi32(shuffle4, _mm512_set1_epi32(index4[0]));
    shuffle5 = _mm512_sub_epi32(shuffle5, _mm512_set1_epi32(index5[0]));
    shuffle6 = _mm512_sub_epi32(shuffle6, _mm512_set1_epi32(index6[0]));
    shuffle7 = _mm512_sub_epi32(shuffle7, _mm512_set1_epi32(index7[0]));

    const float* in0 = in_samples + index0[0];
    const float* in1 = in_samples + index1[0];
    const float* in2 = in_samples + index2[0];
    const float* in3 = in_samples + index3[0];
    const float* in4 = in_samples + index4[0];
    const float* in5 = in_samples + index5[0];
    const float* in6 = in_samples + index6[0];
    const float* in7 = in_samples + index7[0];

    __m512 r0 = _mm512_setzero_ps();
    __m512 r1 = _mm512_setzero_ps();
    __m512 r2 = _mm512_setzero_ps();
    __m512 r3 = _mm512_setzero_ps();
    __m512 r4 = _mm512_setzero_ps();
    __m512 r5 = _mm512_setzero_ps();
    __m512 r6 = _mm512_setzero_ps();
    __m512 r7 = _mm512_setzero_ps();

    size_t tap_stride = table.tap_stride();
    const float* tap_weights = table.tap_start(0) + out_index;

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        __m512 i0, i1, i2, i3, i4, i5, i6, i7;

        if constexpr (input_loads == 1){
            i0 = _mm512_permutexvar_ps(shuffle0, _mm512_loadu_ps(in0 + tap));
            i1 = _mm512_permutexvar_ps(shuffle1, _mm512_loadu_ps(in1 + tap));
            i2 = _mm512_permutexvar_ps(shuffle2, _mm512_loadu_ps(in2 + tap));
            i3 = _mm512_permutexvar_ps(shuffle3, _mm512_loadu_ps(in3 + tap));
            i4 = _mm512_permutexvar_ps(shuffle4, _mm512_loadu_ps(in4 + tap));
            i5 = _mm512_permutexvar_ps(shuffle5, _mm512_loadu_ps(in5 + tap));
            i6 = _mm512_permutexvar_ps(shuffle6, _mm512_loadu_ps(in6 + tap));
            i7 = _mm512_permutexvar_ps(shuffle7, _mm512_loadu_ps(in7 + tap));
        }else if constexpr (input_loads == 2){
            i0 = _mm512_permutex2var_ps(_mm512_loadu_ps(in0 + tap + 0), shuffle0, _mm512_loadu_ps(in0 + tap + 16));
            i1 = _mm512_permutex2var_ps(_mm512_loadu_ps(in1 + tap + 0), shuffle1, _mm512_loadu_ps(in1 + tap + 16));
            i2 = _mm512_permutex2var_ps(_mm512_loadu_ps(in2 + tap + 0), shuffle2, _mm512_loadu_ps(in2 + tap + 16));
            i3 = _mm512_permutex2var_ps(_mm512_loadu_ps(in3 + tap + 0), shuffle3, _mm512_loadu_ps(in3 + tap + 16));
            i4 = _mm512_permutex2var_ps(_mm512_loadu_ps(in4 + tap + 0), shuffle4, _mm512_loadu_ps(in4 + tap + 16));
            i5 = _mm512_permutex2var_ps(_mm512_loadu_ps(in5 + tap + 0), shuffle5, _mm512_loadu_ps(in5 + tap + 16));
            i6 = _mm512_permutex2var_ps(_mm512_loadu_ps(in6 + tap + 0), shuffle6, _mm512_loadu_ps(in6 + tap + 16));
            i7 = _mm512_permutex2var_ps(_mm512_loadu_ps(in7 + tap + 0), shuffle7, _mm512_loadu_ps(in7 + tap + 16));
        }else{
            static_assert(false);
        }

        r0 = _mm512_fmadd_ps(i0, _mm512_loadu_ps(tap_weights + 16*0), r0);
        r1 = _mm512_fmadd_ps(i1, _mm512_loadu_ps(tap_weights + 16*1), r1);
        r2 = _mm512_fmadd_ps(i2, _mm512_loadu_ps(tap_weights + 16*2), r2);
        r3 = _mm512_fmadd_ps(i3, _mm512_loadu_ps(tap_weights + 16*3), r3);
        r4 = _mm512_fmadd_ps(i4, _mm512_loadu_ps(tap_weights + 16*4), r4);
        r5 = _mm512_fmadd_ps(i5, _mm512_loadu_ps(tap_weights + 16*5), r5);
        r6 = _mm512_fmadd_ps(i6, _mm512_loadu_ps(tap_weights + 16*6), r6);
        r7 = _mm512_fmadd_ps(i7, _mm512_loadu_ps(tap_weights + 16*7), r7);

        tap_weights += tap_stride;
        tap++;
    }while (tap < stop);

    _mm512_storeu_ps(out_samples + 16*0, r0);
    _mm512_storeu_ps(out_samples + 16*1, r1);
    _mm512_storeu_ps(out_samples + 16*2, r2);
    _mm512_storeu_ps(out_samples + 16*3, r3);
    _mm512_storeu_ps(out_samples + 16*4, r4);
    _mm512_storeu_ps(out_samples + 16*5, r5);
    _mm512_storeu_ps(out_samples + 16*6, r6);
    _mm512_storeu_ps(out_samples + 16*7, r7);
}





//
//  This is a proof-of-concept for a transpose approach. It works for both
//  upsampling and downsampling by any amount.
//
//  This is not efficient though since the opcount ratios come down to:
//    - 1 x FMA
//    - 3 x shuffle
//    - 1 x aligned 512-bit load
//    - 2 x misaligned 256-bit load
//    - 2 x GPR load for spills
//
PA_FORCE_INLINE __m512 splitload512(const float* L, const float* H){
    return _mm512_insertf32x8(
        _mm512_castps256_ps512(_mm256_loadu_ps(L)),
        _mm256_loadu_ps(H),
        1
    );
}
PA_FORCE_INLINE void transposef32x16x8(
    const float* I00, const float* I01, const float* I02, const float* I03,
    const float* I04, const float* I05, const float* I06, const float* I07,
    const float* I08, const float* I09, const float* I10, const float* I11,
    const float* I12, const float* I13, const float* I14, const float* I15,
    __m512& r0, __m512& r1, __m512& r2, __m512& r3,
    __m512& r4, __m512& r5, __m512& r6, __m512& r7
){
    __m512 s0, s1, s2, s3, s4, s5, s6, s7;

    s0 = splitload512(I00, I04);
    s1 = splitload512(I01, I05);
    s2 = splitload512(I02, I06);
    s3 = splitload512(I03, I07);
    s4 = splitload512(I08, I12);
    s5 = splitload512(I09, I13);
    s6 = splitload512(I10, I14);
    s7 = splitload512(I11, I15);

    r0 = _mm512_shuffle_f32x4(s0, s4, 136);
    r4 = _mm512_shuffle_f32x4(s0, s4, 221);
    r1 = _mm512_shuffle_f32x4(s1, s5, 136);
    r5 = _mm512_shuffle_f32x4(s1, s5, 221);
    r2 = _mm512_shuffle_f32x4(s2, s6, 136);
    r6 = _mm512_shuffle_f32x4(s2, s6, 221);
    r3 = _mm512_shuffle_f32x4(s3, s7, 136);
    r7 = _mm512_shuffle_f32x4(s3, s7, 221);

    s0 = _mm512_shuffle_ps(r0, r1, 136);
    s1 = _mm512_shuffle_ps(r0, r1, 221);
    s2 = _mm512_shuffle_ps(r2, r3, 136);
    s3 = _mm512_shuffle_ps(r2, r3, 221);
    s4 = _mm512_shuffle_ps(r4, r5, 136);
    s5 = _mm512_shuffle_ps(r4, r5, 221);
    s6 = _mm512_shuffle_ps(r6, r7, 136);
    s7 = _mm512_shuffle_ps(r6, r7, 221);

    r0 = _mm512_shuffle_ps(s0, s2, 136);
    r2 = _mm512_shuffle_ps(s0, s2, 221);
    r1 = _mm512_shuffle_ps(s1, s3, 136);
    r3 = _mm512_shuffle_ps(s1, s3, 221);
    r4 = _mm512_shuffle_ps(s4, s6, 136);
    r6 = _mm512_shuffle_ps(s4, s6, 221);
    r5 = _mm512_shuffle_ps(s5, s7, 136);
    r7 = _mm512_shuffle_ps(s5, s7, 221);
}
inline void run_block32_tap8_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    in_samples -= in_index;

    const uint32_t* index = table.in_sample_index() + out_index;

    const float* tap_weights = table.tap_start(0) + out_index;
    size_t tap_stride = table.tap_stride();

    __m512 r0 = _mm512_setzero_ps();
    __m512 r1 = _mm512_setzero_ps();

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        __m512 a0, a1, a2, a3, a4, a5, a6, a7;
        __m512 b0, b1, b2, b3, b4, b5, b6, b7;

        transposef32x16x8(
            in_samples + index[ 0],
            in_samples + index[ 1],
            in_samples + index[ 2],
            in_samples + index[ 3],
            in_samples + index[ 4],
            in_samples + index[ 5],
            in_samples + index[ 6],
            in_samples + index[ 7],
            in_samples + index[ 8],
            in_samples + index[ 9],
            in_samples + index[10],
            in_samples + index[11],
            in_samples + index[12],
            in_samples + index[13],
            in_samples + index[14],
            in_samples + index[15],
            a0, a1, a2, a3, a4, a5, a6, a7
        );
        transposef32x16x8(
            in_samples + index[16],
            in_samples + index[17],
            in_samples + index[18],
            in_samples + index[19],
            in_samples + index[20],
            in_samples + index[21],
            in_samples + index[22],
            in_samples + index[23],
            in_samples + index[24],
            in_samples + index[25],
            in_samples + index[26],
            in_samples + index[27],
            in_samples + index[28],
            in_samples + index[29],
            in_samples + index[30],
            in_samples + index[31],
            b0, b1, b2, b3, b4, b5, b6, b7
        );

        r0 = _mm512_fmadd_ps(a0, _mm512_loadu_ps(tap_weights + tap_stride*0 +  0), r0);
        r1 = _mm512_fmadd_ps(b0, _mm512_loadu_ps(tap_weights + tap_stride*0 + 16), r1);

        r0 = _mm512_fmadd_ps(a1, _mm512_loadu_ps(tap_weights + tap_stride*1 +  0), r0);
        r1 = _mm512_fmadd_ps(b1, _mm512_loadu_ps(tap_weights + tap_stride*1 + 16), r1);

        r0 = _mm512_fmadd_ps(a2, _mm512_loadu_ps(tap_weights + tap_stride*2 +  0), r0);
        r1 = _mm512_fmadd_ps(b2, _mm512_loadu_ps(tap_weights + tap_stride*2 + 16), r1);

        r0 = _mm512_fmadd_ps(a3, _mm512_loadu_ps(tap_weights + tap_stride*3 +  0), r0);
        r1 = _mm512_fmadd_ps(b3, _mm512_loadu_ps(tap_weights + tap_stride*3 + 16), r1);

        r0 = _mm512_fmadd_ps(a4, _mm512_loadu_ps(tap_weights + tap_stride*4 +  0), r0);
        r1 = _mm512_fmadd_ps(b4, _mm512_loadu_ps(tap_weights + tap_stride*4 + 16), r1);

        r0 = _mm512_fmadd_ps(a5, _mm512_loadu_ps(tap_weights + tap_stride*5 +  0), r0);
        r1 = _mm512_fmadd_ps(b5, _mm512_loadu_ps(tap_weights + tap_stride*5 + 16), r1);

        r0 = _mm512_fmadd_ps(a6, _mm512_loadu_ps(tap_weights + tap_stride*6 +  0), r0);
        r1 = _mm512_fmadd_ps(b6, _mm512_loadu_ps(tap_weights + tap_stride*6 + 16), r1);

        r0 = _mm512_fmadd_ps(a7, _mm512_loadu_ps(tap_weights + tap_stride*7 +  0), r0);
        r1 = _mm512_fmadd_ps(b7, _mm512_loadu_ps(tap_weights + tap_stride*7 + 16), r1);

        tap_weights += tap_stride*8;
        in_samples += 8;
        tap += 8;
    }while (tap < stop);

    _mm512_storeu_ps(out_samples + 16*0, r0);
    _mm512_storeu_ps(out_samples + 16*1, r1);
}



}
}
}
#endif
