/*  Audio Resampler Block Kernel (ARM64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioResampler_BlockKernel_ARM64_NEON_H
#define PokemonAutomation_Kernels_AudioResampler_BlockKernel_ARM64_NEON_H

#include <arm_neon.h>
#include "Common/Compiler.h"
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



//
//  Opcount Ratios:
//    - 1 x MUL + 1 x ADD
//    - 3/4 x shuffle
//    - 1/2 x aligned 128-bit load
//    - 1 x misaligned 64-bit load
//
PA_FORCE_INLINE void transposef32x4x2_ARM64_NEON(
    const float* I0, const float* I1, const float* I2, const float* I3,
    float32x4_t& r0, float32x4_t& r1
){
    //  I0..I3 each point at in_samples + index[k] + tap. The tap + 1 value
    //  of each output is adjacent in the stream, so load the two taps of
    //  each output with 64-bit loads:
    //      s0 = [in0[t], in0[t+1], in1[t], in1[t+1]]
    //      s1 = [in2[t], in2[t+1], in3[t], in3[t+1]]
    float32x2_t p0 = vld1_f32(I0);
    float32x2_t p1 = vld1_f32(I1);
    float32x2_t p2 = vld1_f32(I2);
    float32x2_t p3 = vld1_f32(I3);
    float32x4_t s0 = vcombine_f32(p0, p1);
    float32x4_t s1 = vcombine_f32(p2, p3);

    //  vtrnq interleaves the even/odd lanes of the pair:
    //      z.val[0] = [in0[t], in2[t], in1[t], in3[t]]
    //      z.val[1] = [in0[t+1], in2[t+1], in1[t+1], in3[t+1]]
    //  and two 64-bit vtrns undo the lane pairing:
    //      r0 = [in0[t], in1[t], in2[t], in3[t]]
    //      r1 = [in0[t+1], in1[t+1], in2[t+1], in3[t+1]]
    float32x4x2_t z = vtrnq_f32(s0, s1);
    float32x2x2_t t0 = vtrn_f32(vget_low_f32(z.val[0]), vget_high_f32(z.val[0]));
    float32x2x2_t t1 = vtrn_f32(vget_low_f32(z.val[1]), vget_high_f32(z.val[1]));
    r0 = vcombine_f32(t0.val[0], t0.val[1]);
    r1 = vcombine_f32(t1.val[0], t1.val[1]);
}
inline void run_block8_tap2_ARM64_NEON(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    in_samples -= in_index;

    const uint32_t* index = table.in_sample_index() + out_index;

    const float* tap_weights = table.tap_start(0) + out_index;
    size_t tap_stride = table.tap_stride();

    float32x4_t r0 = vdupq_n_f32(0.0f);
    float32x4_t r1 = vdupq_n_f32(0.0f);

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        float32x4_t a0, a1;
        float32x4_t b0, b1;

        transposef32x4x2_ARM64_NEON(
            in_samples + index[ 0],
            in_samples + index[ 1],
            in_samples + index[ 2],
            in_samples + index[ 3],
            a0, a1
        );
        transposef32x4x2_ARM64_NEON(
            in_samples + index[ 4],
            in_samples + index[ 5],
            in_samples + index[ 6],
            in_samples + index[ 7],
            b0, b1
        );

        r0 = vaddq_f32(r0, vmulq_f32(a0, vld1q_f32(tap_weights + tap_stride*0 + 0)));
        r1 = vaddq_f32(r1, vmulq_f32(b0, vld1q_f32(tap_weights + tap_stride*0 + 4)));

        r0 = vaddq_f32(r0, vmulq_f32(a1, vld1q_f32(tap_weights + tap_stride*1 + 0)));
        r1 = vaddq_f32(r1, vmulq_f32(b1, vld1q_f32(tap_weights + tap_stride*1 + 4)));

        tap_weights += tap_stride*2;
        in_samples += 2;
        tap += 2;
    }while (tap < stop);

    vst1q_f32(out_samples + 4*0, r0);
    vst1q_f32(out_samples + 4*1, r1);
}



}
}
}
#endif
