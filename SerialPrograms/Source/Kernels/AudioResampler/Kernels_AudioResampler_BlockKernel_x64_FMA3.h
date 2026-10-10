/*  Audio Resampler Block Kernel (x64 FMA3)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BlockKernel_x64_FMA3_H
#define PokemonAutomation_Kernels_BlockKernel_x64_FMA3_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



//
//  Opcount Ratios:
//    - 1 x FMA
//    - 2 x shuffle
//    - 1 x aligned 256-bit load
//    - 2 x misaligned 128-bit load
//
PA_FORCE_INLINE __m256 splitload256(const float* L, const float* H){
    return _mm256_loadu2_m128(H, L);
}
PA_FORCE_INLINE void transposef32x8x4(
    const float* I0, const float* I1, const float* I2, const float* I3,
    const float* I4, const float* I5, const float* I6, const float* I7,
    __m256& r0, __m256& r1, __m256& r2, __m256& r3
){
    __m256 s0, s1, s2, s3;

    r0 = splitload256(I0, I4);
    r1 = splitload256(I2, I6);
    r2 = splitload256(I1, I5);
    r3 = splitload256(I3, I7);

    s0 = _mm256_unpacklo_ps(r0, r2);
    s1 = _mm256_unpacklo_ps(r1, r3);
    s2 = _mm256_unpackhi_ps(r0, r2);
    s3 = _mm256_unpackhi_ps(r1, r3);

    r0 = _mm256_shuffle_ps(s0, s1, 68);
    r1 = _mm256_shuffle_ps(s0, s1, 238);
    r2 = _mm256_shuffle_ps(s2, s3, 68);
    r3 = _mm256_shuffle_ps(s2, s3, 238);
}
inline void run_block16_tap4_x64_FMA3(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    in_samples -= in_index;

    const uint32_t* index = table.in_sample_index() + out_index;

    const float* tap_weights = table.tap_start(0) + out_index;
    size_t tap_stride = table.tap_stride();

    __m256 r0 = _mm256_setzero_ps();
    __m256 r1 = _mm256_setzero_ps();

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        __m256 a0, a1, a2, a3;
        __m256 b0, b1, b2, b3;

        transposef32x8x4(
            in_samples + index[ 0],
            in_samples + index[ 1],
            in_samples + index[ 2],
            in_samples + index[ 3],
            in_samples + index[ 4],
            in_samples + index[ 5],
            in_samples + index[ 6],
            in_samples + index[ 7],
            a0, a1, a2, a3
        );
        transposef32x8x4(
            in_samples + index[ 8],
            in_samples + index[ 9],
            in_samples + index[10],
            in_samples + index[11],
            in_samples + index[12],
            in_samples + index[13],
            in_samples + index[14],
            in_samples + index[15],
            b0, b1, b2, b3
        );

        r0 = _mm256_fmadd_ps(a0, _mm256_loadu_ps(tap_weights + tap_stride*0 + 0), r0);
        r1 = _mm256_fmadd_ps(b0, _mm256_loadu_ps(tap_weights + tap_stride*0 + 8), r1);

        r0 = _mm256_fmadd_ps(a1, _mm256_loadu_ps(tap_weights + tap_stride*1 + 0), r0);
        r1 = _mm256_fmadd_ps(b1, _mm256_loadu_ps(tap_weights + tap_stride*1 + 8), r1);

        r0 = _mm256_fmadd_ps(a2, _mm256_loadu_ps(tap_weights + tap_stride*2 + 0), r0);
        r1 = _mm256_fmadd_ps(b2, _mm256_loadu_ps(tap_weights + tap_stride*2 + 8), r1);

        r0 = _mm256_fmadd_ps(a3, _mm256_loadu_ps(tap_weights + tap_stride*3 + 0), r0);
        r1 = _mm256_fmadd_ps(b3, _mm256_loadu_ps(tap_weights + tap_stride*3 + 8), r1);

        tap_weights += tap_stride*4;
        in_samples += 4;
        tap += 4;
    }while (tap < stop);

    _mm256_storeu_ps(out_samples + 8*0, r0);
    _mm256_storeu_ps(out_samples + 8*1, r1);
}




}
}
}
#endif
