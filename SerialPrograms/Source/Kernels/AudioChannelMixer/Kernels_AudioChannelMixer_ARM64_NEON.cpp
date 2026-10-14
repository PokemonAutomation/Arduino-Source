/*  Audio Channel Mixer (ARM64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioChannelMixer_TestInfra.h"
#include "Kernels_AudioChannelMixer_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



PA_FORCE_INLINE void mix_channels_generic_ARM64_NEON(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    while (samples > 4){
        for (size_t o = 0; o < out_channels; o++){
            float32x4_t x = vdupq_n_f32(0.0f);
            for (size_t i = 0; i < in_channels; i++){
                x = vaddq_f32(
                    x,
                    vmulq_f32(
                        vld1q_f32(in + in_stride * i),
                        vdupq_n_f32(matrix[i + in_channels * o])
                    )
                );
            }
            vst1q_f32(out + out_stride * o, x);
        }
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_GenericDefault(
        matrix, samples,
        out_channels, out_stride, out,
        in_channels, in_stride, in
    );
}
template <size_t in_channels, size_t out_channels>
void mix_channels_ARM64_NEON(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
);
template <>
void mix_channels_ARM64_NEON<1, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float32x4_t i0o0 = vdupq_n_f32(matrix[0]);
    while (samples > 4){
        float32x4_t in0 = vld1q_f32(in);
        float32x4_t out0 = vmulq_f32(in0, i0o0);
        vst1q_f32(out, out0);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<1, 1>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_ARM64_NEON<1, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float32x4_t i0o0 = vdupq_n_f32(matrix[0]);
    float32x4_t i0o1 = vdupq_n_f32(matrix[1]);
    while (samples > 4){
        float32x4_t in0 = vld1q_f32(in);
        float32x4_t out0 = vmulq_f32(in0, i0o0);
        float32x4_t out1 = vmulq_f32(in0, i0o1);
        vst1q_f32(out + out_stride * 0, out0);
        vst1q_f32(out + out_stride * 1, out1);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<1, 2>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_ARM64_NEON<2, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float32x4_t i0o0 = vdupq_n_f32(matrix[0]);
    float32x4_t i1o0 = vdupq_n_f32(matrix[1]);
    while (samples > 4){
        float32x4_t out0 = vmulq_f32(i0o0, vld1q_f32(in + in_stride * 0));
        out0 = vaddq_f32(out0, vmulq_f32(i1o0, vld1q_f32(in + in_stride * 1)));
        vst1q_f32(out, out0);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<2, 1>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_ARM64_NEON<2, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float32x4_t i0o0 = vdupq_n_f32(matrix[0]);
    float32x4_t i1o0 = vdupq_n_f32(matrix[1]);
    float32x4_t i0o1 = vdupq_n_f32(matrix[2]);
    float32x4_t i1o1 = vdupq_n_f32(matrix[3]);
    while (samples > 4){
        float32x4_t in0 = vld1q_f32(in + in_stride * 0);
        float32x4_t in1 = vld1q_f32(in + in_stride * 1);
        float32x4_t out0 = vmulq_f32(in0, i0o0);
        float32x4_t out1 = vmulq_f32(in0, i0o1);
        out0 = vaddq_f32(out0, vmulq_f32(in1, i1o0));
        out1 = vaddq_f32(out1, vmulq_f32(in1, i1o1));
        vst1q_f32(out + out_stride * 0, out0);
        vst1q_f32(out + out_stride * 1, out1);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<2, 2>(matrix, samples, out_stride, out, in_stride, in);
}



void mix_channels_ARM64_NEON(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    size_t combined = (in_channels << 16) | out_channels;
    switch (combined){
    case 0x00010001:
        mix_channels_ARM64_NEON<1, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020001:
        mix_channels_ARM64_NEON<2, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00010002:
        mix_channels_ARM64_NEON<1, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020002:
        mix_channels_ARM64_NEON<2, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    default:
        mix_channels_generic_ARM64_NEON(
            matrix, samples,
            out_channels, out_stride, out,
            in_channels, in_stride, in
        );
    }
}



class Test_ARM64_NEON : public Test{
public:
    Test_ARM64_NEON(
        size_t in_channels,
        size_t out_channels,
        size_t samples
    )
        : Test(
            "ARM64_NEON",
            in_channels,
            out_channels,
            samples,
            (Convert)mix_channels_Default,
            (Convert)mix_channels_ARM64_NEON
        )
    {}
};
void add_tests_ARM64_NEON(UnitTestDatabase& database){
    for (size_t in_channels = 1; in_channels <= 4; in_channels++){
        for (size_t out_channels = 1; out_channels <= 4; out_channels++){
            for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
                database.add<Test_ARM64_NEON>(in_channels, out_channels, samples);
            }
        }
    }
}


}
}
}
#endif
