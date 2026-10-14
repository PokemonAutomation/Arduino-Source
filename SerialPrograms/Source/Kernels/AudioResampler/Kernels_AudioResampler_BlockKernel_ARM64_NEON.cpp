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
//    - 1 x FMA
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



//
//  Byte-permute fast path: the NEON equivalent of the AVX512 fast
//  kernels. Instead of transposing the tap rows into the output
//  columns, build one TBL byte-index register per 4-output group and
//  gather the input samples directly from 2 x 128-bit table registers.
//
//  The TBL byte index is the 4-byte input index converted to byte
//  granularity: each 32-bit delta is multiplied by 0x04040404 and
//  0x03020100 is added, so word c holds [4d, 4d+1, 4d+2, 4d+3] and
//  picks out the 4 bytes of input sample d_c for the 4 output lanes.
//
//  Per-group input windows:
//    - run_block8_up:      1 x misaligned 128-bit load (4-float window)
//    - run_block8_down2:   2 x misaligned 128-bit loads (8-float window)
//
PA_FORCE_INLINE uint8x16_t build_group_byte_index(
    const uint32_t* index, uint32_t base
){
    uint32x4_t d = vsubq_u32(vld1q_u32(index), vdupq_n_u32(base));
    //  d < 16, so the replicated bytes never carry into each other.
    uint32x4_t b = vmlaq_u32(vdupq_n_u32(0x03020100u), d, vdupq_n_u32(0x04040404u));
    return vreinterpretq_u8_u32(b);
}


//
//  TBL: gather 4 floats by byte index from 2 x 128-bit table registers.
//  The intrinsic name differs between compilers:
//  GCC: vqtbl2q_u8(tab, idx); Clang/MSVC: vtbl2q_u8(idx, tab0, tab1).
//
PA_FORCE_INLINE float32x4_t gather_group(
    float32x4_t tab0, float32x4_t tab1, uint8x16_t idx
){
#if defined(__GNUC__) && !defined(__clang__)
    uint8x16x2_t tab{ vreinterpretq_u8_f32(tab0), vreinterpretq_u8_f32(tab1) };
    return vreinterpretq_f32_u8(vqtbl2q_u8(tab, idx));
#else
    return vreinterpretq_f32_u8(vtbl2q_u8(idx, vreinterpretq_u8_f32(tab0), vreinterpretq_u8_f32(tab1)));
#endif
}


template <int input_loads>
PA_FORCE_INLINE void run_block8_permute_ARM64_NEON(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    in_samples -= in_index;

    const uint32_t* index = table.in_sample_index() + out_index;

    //
    //  The index deltas of each 4-column group, relative to the group's
    //  first column, are the TBL byte indices.
    //
    const uint8x16_t idx0 = build_group_byte_index(index + 4*0, index[4*0]);
    const uint8x16_t idx1 = build_group_byte_index(index + 4*1, index[4*1]);

    //
    //  Each group's input window starts at its first column.
    //
    const float* in0 = in_samples + index[4*0];
    const float* in1 = in_samples + index[4*1];

    float32x4_t r0 = vdupq_n_f32(0.0f);
    float32x4_t r1 = vdupq_n_f32(0.0f);

    const float* tap_weights = table.tap_start(0) + out_index;
    size_t tap_stride = table.tap_stride();

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        float32x4_t g0, g1;
        if constexpr (input_loads == 1){
            g0 = gather_group(vld1q_f32(in0 + tap), vld1q_f32(in0 + tap), idx0);
            g1 = gather_group(vld1q_f32(in1 + tap), vld1q_f32(in1 + tap), idx1);
        }else{
            g0 = gather_group(vld1q_f32(in0 + tap + 0), vld1q_f32(in0 + tap + 4), idx0);
            g1 = gather_group(vld1q_f32(in1 + tap + 0), vld1q_f32(in1 + tap + 4), idx1);
        }
        r0 = vfmaq_f32(r0, g0, vld1q_f32(tap_weights + 0));
        r1 = vfmaq_f32(r1, g1, vld1q_f32(tap_weights + 4));
        tap_weights += tap_stride;
        tap += 1;
    }while (tap < stop);

    vst1q_f32(out_samples + 4*0, r0);
    vst1q_f32(out_samples + 4*1, r1);
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

        r0 = vfmaq_f32(r0, a0, vld1q_f32(tap_weights + tap_stride*0 + 0));
        r1 = vfmaq_f32(r1, b0, vld1q_f32(tap_weights + tap_stride*0 + 4));

        r0 = vfmaq_f32(r0, a1, vld1q_f32(tap_weights + tap_stride*1 + 0));
        r1 = vfmaq_f32(r1, b1, vld1q_f32(tap_weights + tap_stride*1 + 4));

        r0 = vfmaq_f32(r0, a2, vld1q_f32(tap_weights + tap_stride*2 + 0));
        r1 = vfmaq_f32(r1, b2, vld1q_f32(tap_weights + tap_stride*2 + 4));

        r0 = vfmaq_f32(r0, a3, vld1q_f32(tap_weights + tap_stride*3 + 0));
        r1 = vfmaq_f32(r1, b3, vld1q_f32(tap_weights + tap_stride*3 + 4));

        tap_weights += tap_stride*4;
        in_samples += 4;
        tap += 4;
    }while (tap < stop);

    vst1q_f32(out_samples + 4*0, r0);
    vst1q_f32(out_samples + 4*1, r1);
}


void run_block8_up_ARM64_NEON(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    run_block8_permute_ARM64_NEON<1>(
        table, in_index, in_samples, out_index, out_samples
    );
}


void run_block8_down2_ARM64_NEON(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    run_block8_permute_ARM64_NEON<2>(
        table, in_index, in_samples, out_index, out_samples
    );
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

    database.add<BlockKernelTest>(
        "NEON_Up", run_block8_up_ARM64_NEON,
        44100, 48000, 16, 8, 0
    );
    //  The block straddles the table period (160) into the continuing
    //  index columns.
    database.add<BlockKernelTest>(
        "NEON_Up", run_block8_up_ARM64_NEON,
        44100, 48000, 16, 8, 155
    );
    database.add<BlockKernelTest>(
        "NEON_Up", run_block8_up_ARM64_NEON,
        44100, 88200, 16, 8, 0
    );

    database.add<BlockKernelTest>(
        "NEON_Down2", run_block8_down2_ARM64_NEON,
        48000, 44100, 16, 8, 0
    );
    //  The block straddles the table period (147) into the continuing
    //  index columns.
    database.add<BlockKernelTest>(
        "NEON_Down2", run_block8_down2_ARM64_NEON,
        48000, 44100, 16, 8, 140
    );
    //  Downsampling just past the 2x boundary (in_rate = 2.0045 x
    //  out_rate): the 4-column index deltas still fit in the 8-float
    //  windows.
    database.add<BlockKernelTest>(
        "NEON_Down2", run_block8_down2_ARM64_NEON,
        88200, 44000, 16, 8, 0
    );
}



}
}
}
#endif
