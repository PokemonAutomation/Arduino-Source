/*  Audio Resampler Block Kernel (ARM64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
#include "Common/Compiler.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioResampler_BlockKernels.h"
#include "Kernels_AudioResampler_Tests.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



namespace{



//
//  Opcount Ratios (per tap, per 4 outputs):
//    - 1 x MUL + 1 x ADD
//    - 1/2 x shuffle
//    - 1 x aligned 128-bit load
//    - 1 x misaligned 128-bit load
//
//
//  The x86 kernels use the half-width transpose (4x2 for SSE, 8x4 for
//  AVX, 16x8 for AVX512) because unaligned half-vector loads and
//  half-vector insertions are cheap there. NEON has no unaligned 64-bit
//  loads, but it does have unaligned full-vector loads, so use the 4x4
//  square transpose instead: a given output's taps are adjacent in the
//  input stream, so one 128-bit load grabs 4 taps per output, and the
//  transpose itself is 2 x vtrnq + 4 x vcombine in-register.
//
PA_FORCE_INLINE void transposef32x4x4(
    const float* I0, const float* I1, const float* I2, const float* I3,
    float32x4_t& r0, float32x4_t& r1,
    float32x4_t& r2, float32x4_t& r3
){
    //  I0..I3 each point at in_samples + index[k] + tap, and the next
    //  three stream values are taps + 1 .. + 3 of the same output, so a
    //  full 128-bit unaligned load grabs all 4 taps per output:
    //      Lk = [Ik[t], Ik[t+1], Ik[t+2], Ik[t+3]]
    float32x4_t L0 = vld1q_f32(I0);
    float32x4_t L1 = vld1q_f32(I1);
    float32x4_t L2 = vld1q_f32(I2);
    float32x4_t L3 = vld1q_f32(I3);

    //  Two 128-bit lane-pair transposes and four 64-bit combines give
    //  the full 4x4 transpose:
    //      rj = [I0[t+j], I1[t+j], I2[t+j], I3[t+j]]
    float32x4x2_t AB = vtrnq_f32(L0, L1);
    float32x4x2_t CD = vtrnq_f32(L2, L3);
    r0 = vcombine_f32(vget_low_f32(AB.val[0]),  vget_low_f32(CD.val[0]));
    r1 = vcombine_f32(vget_low_f32(AB.val[1]),  vget_low_f32(CD.val[1]));
    r2 = vcombine_f32(vget_high_f32(AB.val[0]), vget_high_f32(CD.val[0]));
    r3 = vcombine_f32(vget_high_f32(AB.val[1]), vget_high_f32(CD.val[1]));
}



}


void run_block8_tap4_ARM64_NEON(
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
        float32x4_t a0, a1, a2, a3;
        float32x4_t b0, b1, b2, b3;

        transposef32x4x4(
            in_samples + index[ 0],
            in_samples + index[ 1],
            in_samples + index[ 2],
            in_samples + index[ 3],
            a0, a1, a2, a3
        );
        transposef32x4x4(
            in_samples + index[ 4],
            in_samples + index[ 5],
            in_samples + index[ 6],
            in_samples + index[ 7],
            b0, b1, b2, b3
        );

        r0 = vaddq_f32(r0, vmulq_f32(a0, vld1q_f32(tap_weights + tap_stride*0 + 0)));
        r1 = vaddq_f32(r1, vmulq_f32(b0, vld1q_f32(tap_weights + tap_stride*0 + 4)));

        r0 = vaddq_f32(r0, vmulq_f32(a1, vld1q_f32(tap_weights + tap_stride*1 + 0)));
        r1 = vaddq_f32(r1, vmulq_f32(b1, vld1q_f32(tap_weights + tap_stride*1 + 4)));

        r0 = vaddq_f32(r0, vmulq_f32(a2, vld1q_f32(tap_weights + tap_stride*2 + 0)));
        r1 = vaddq_f32(r1, vmulq_f32(b2, vld1q_f32(tap_weights + tap_stride*2 + 4)));

        r0 = vaddq_f32(r0, vmulq_f32(a3, vld1q_f32(tap_weights + tap_stride*3 + 0)));
        r1 = vaddq_f32(r1, vmulq_f32(b3, vld1q_f32(tap_weights + tap_stride*3 + 4)));

        tap_weights += tap_stride*4;
        in_samples += 4;
        tap += 4;
    }while (tap < stop);

    vst1q_f32(out_samples + 4*0, r0);
    vst1q_f32(out_samples + 4*1, r1);
}



void add_tests_ARM64_NEON(UnitTestDatabase& database){
    database.add<BlockKernelTest>(
        "NEON", run_block8_tap4_ARM64_NEON,
        48000, 44100, 16, 8, 0
    );
    //  The block straddles the table period (147) into the continuing
    //  index columns.
    database.add<BlockKernelTest>(
        "NEON", run_block8_tap4_ARM64_NEON,
        48000, 44100, 16, 8, 140
    );
    database.add<BlockKernelTest>(
        "NEON", run_block8_tap4_ARM64_NEON,
        44100, 88200, 16, 8, 0
    );
    database.add<BlockKernelTest>(
        "NEON", run_block8_tap4_ARM64_NEON,
        48000, 44100, 8, 8, 140
    );
}



}
}
}
#endif
