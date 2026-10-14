/*  Audio Resampler Block Kernels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioResampler_BlockKernels_H
#define PokemonAutomation_Kernels_AudioResampler_BlockKernels_H

#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



//
//  Block kernels compute a run of consecutive output samples from the
//  weight table:
//
//      out_samples[c] = sum over taps t of
//          table.tap_start(t)[out_index + c] x
//          in_samples[table.in_sample_index()[out_index + c] + t - in_index]
//
//  `in_index` is the absolute input stream index of in_samples[0].
//  `out_index` is the absolute output stream index wrapped to the table
//  period.
//
//  Each kernel processes a fixed block size. Fixed blocks can overshoot
//  the end of the table in both dimensions, so the weight table must be
//  constructed with min_samples / min_taps large enough for the block
//  plus the kernel's read overshoot (documented per kernel). The extra
//  input samples read past the block's last window are never used for
//  the outputs and only need to be valid memory.
//
//  The definitions are non-inline, one per ISA translation unit. The x64
//  kernels are only defined when the matching PA_AutoDispatch_x64_* macro
//  is set, and require the matching CPU features at run time.
//



//
//  8 outputs. Any rate ratio, any tap count.
//  Read overshoot: 0 input samples.
//
void run_block8_Default(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  8 outputs. Any rate ratio.
//  Tap count must be a multiple of 2, or the table must be zero-padded
//  with min_taps to taps + 1 rows.
//  Read overshoot: 1 input sample.
//
void run_block8_tap2_x64_SSE2(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  8 outputs. Any rate ratio.
//  Tap count must be a multiple of 4, or the table must be zero-padded
//  with min_taps to taps + 3 rows.
//  Read overshoot: 3 input samples.
//
void run_block8_tap4_ARM64_NEON(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  8 outputs. Fast byte-permute path (the NEON equivalent of the AVX512
//  fast kernels): gathers the input samples with TBL byte permutes
//  instead of transposing the tap rows.
//  Upsampling only (in_rate <= out_rate): the index deltas between the
//  first and last column of each 4-column group must fit in a 4-float
//  window.
//  Read overshoot: 3 input samples.
//
void run_block8_up_ARM64_NEON(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  8 outputs. Fast byte-permute path (the NEON equivalent of the AVX512
//  fast kernels).
//  Downsampling by no more than 2x (in_rate <= 2 x out_rate): the index
//  deltas between the first and last column of each 4-column group must
//  fit in an 8-float window.
//  Read overshoot: 7 input samples.
//
void run_block8_down2_ARM64_NEON(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  16 outputs. Any rate ratio.
//  Tap count must be a multiple of 4, or the table must be zero-padded
//  with min_taps to taps + 3 rows.
//  Read overshoot: 3 input samples.
//
void run_block16_tap4_x64_FMA3(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  32 outputs. Any rate ratio.
//  Tap count must be a multiple of 8, or the table must be zero-padded
//  with min_taps to taps + 7 rows.
//  Read overshoot: 7 input samples.
//
void run_block32_tap8_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  128 outputs. Upsampling only (in_rate <= out_rate): the index deltas
//  between the first and last column of each 16-column group must fit in
//  a 16-lane permute.
//  Read overshoot: 15 input samples.
//
void run_block128_up_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  128 outputs. Downsampling by no more than 2x (in_rate <= 2 x out_rate):
//  the index deltas between the first and last column of each 16-column
//  group must fit in a 32-lane permute.
//  Read overshoot: 31 input samples.
//
void run_block128_down2_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



}
}
}
#endif
