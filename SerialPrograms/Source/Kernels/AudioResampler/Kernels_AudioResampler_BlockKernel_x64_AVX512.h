/*  Audio Resampler Block Kernel (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BlockKernel_x64_AVX512_H
#define PokemonAutomation_Kernels_BlockKernel_x64_AVX512_H

#include <immintrin.h>
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



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

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        const float* tap_weights = table.tap_start(tap) + out_index;

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



}
}
}
#endif
