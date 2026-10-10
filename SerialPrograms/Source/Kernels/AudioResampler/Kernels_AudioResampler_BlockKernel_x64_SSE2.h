/*  Audio Resampler Block Kernel (x64 SSE4)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BlockKernel_x64_SSE4_H
#define PokemonAutomation_Kernels_BlockKernel_x64_SSE4_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



//
//  Opcount Ratios:
//    - 1 x FMA
//    - 1 x shuffle
//    - 1 x aligned 128-bit load
//    - 2 x misaligned 64-bit load
//
PA_FORCE_INLINE __m128 splitload128(const float* L, const float* H){
    return _mm_castpd_ps(_mm_loadh_pd(_mm_load_sd((const double*)(L)), (const double*)(H)));
}
PA_FORCE_INLINE void transposef32x4x2(
    const float* I0, const float* I1, const float* I2, const float* I3,
    __m128& r0, __m128& r1
){
    __m128 s0, s1;

    s0 = splitload128(I0, I1);
    s1 = splitload128(I2, I3);

    r0 = _mm_shuffle_ps(s0, s1, 136);
    r1 = _mm_shuffle_ps(s0, s1, 221);
}
inline void run_block8_tap2_x64_SSE2(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    in_samples -= in_index;

    const uint32_t* index = table.in_sample_index() + out_index;

    const float* tap_weights = table.tap_start(0) + out_index;
    size_t tap_stride = table.tap_stride();

    __m128 r0 = _mm_setzero_ps();
    __m128 r1 = _mm_setzero_ps();

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        __m128 a0, a1;
        __m128 b0, b1;

        transposef32x4x2(
            in_samples + index[ 0],
            in_samples + index[ 1],
            in_samples + index[ 2],
            in_samples + index[ 3],
            a0, a1
        );
        transposef32x4x2(
            in_samples + index[ 4],
            in_samples + index[ 5],
            in_samples + index[ 6],
            in_samples + index[ 7],
            b0, b1
        );

        a0 = _mm_mul_ps(a0, _mm_loadu_ps(tap_weights + tap_stride*0 + 0));
        b0 = _mm_mul_ps(b0, _mm_loadu_ps(tap_weights + tap_stride*0 + 4));

        a1 = _mm_mul_ps(a1, _mm_loadu_ps(tap_weights + tap_stride*1 + 0));
        b1 = _mm_mul_ps(b1, _mm_loadu_ps(tap_weights + tap_stride*1 + 4));

        r0 = _mm_add_ps(r0, a0);
        r1 = _mm_add_ps(r1, b0);

        r0 = _mm_add_ps(r0, a1);
        r1 = _mm_add_ps(r1, b1);

        tap_weights += tap_stride*2;
        in_samples += 2;
        tap += 2;
    }while (tap < stop);

    _mm_storeu_ps(out_samples + 4*0, r0);
    _mm_storeu_ps(out_samples + 4*1, r1);
}




}
}
}
#endif
