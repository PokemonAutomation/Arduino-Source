/*  Audio Channel Mixer (x64 SSE)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <emmintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioChannelMixer_TestInfra.h"
#include "Kernels_AudioChannelMixer_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



PA_FORCE_INLINE void mix_channels_generic_x64_SSE(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    while (samples > 4){
        for (size_t o = 0; o < out_channels; o++){
            __m128 x = _mm_setzero_ps();
            for (size_t i = 0; i < in_channels; i++){
                x = _mm_add_ps(
                    x,
                    _mm_mul_ps(
                        _mm_loadu_ps(in + in_stride * i),
                        _mm_set1_ps(matrix[i + in_channels * o])
                    )
                );
            }
            _mm_storeu_ps(out + out_stride * o, x);
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
void mix_channels_x64_SSE(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
);
template <>
void mix_channels_x64_SSE<1, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m128 i0o0 = _mm_set1_ps(matrix[0]);
    while (samples > 4){
        __m128 in0 = _mm_loadu_ps(in);
        __m128 out0 = _mm_mul_ps(in0, i0o0);
        _mm_storeu_ps(out, out0);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<1, 1>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_x64_SSE<1, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m128 i0o0 = _mm_set1_ps(matrix[0]);
    __m128 i0o1 = _mm_set1_ps(matrix[1]);
    while (samples > 4){
        __m128 in0 = _mm_loadu_ps(in);
        __m128 out0 = _mm_mul_ps(in0, i0o0);
        __m128 out1 = _mm_mul_ps(in0, i0o1);
        _mm_storeu_ps(out + out_stride * 0, out0);
        _mm_storeu_ps(out + out_stride * 1, out1);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<1, 2>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_x64_SSE<2, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m128 i0o0 = _mm_set1_ps(matrix[0]);
    __m128 i1o0 = _mm_set1_ps(matrix[1]);
    while (samples > 4){
        __m128 out0 = _mm_mul_ps(i0o0, _mm_loadu_ps(in + in_stride * 0));
        out0 = _mm_add_ps(out0, _mm_mul_ps(i1o0, _mm_loadu_ps(in + in_stride * 1)));
        _mm_storeu_ps(out, out0);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<2, 1>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_x64_SSE<2, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m128 i0o0 = _mm_set1_ps(matrix[0]);
    __m128 i1o0 = _mm_set1_ps(matrix[1]);
    __m128 i0o1 = _mm_set1_ps(matrix[2]);
    __m128 i1o1 = _mm_set1_ps(matrix[3]);
    while (samples > 4){
        __m128 in0 = _mm_loadu_ps(in + in_stride * 0);
        __m128 in1 = _mm_loadu_ps(in + in_stride * 1);
        __m128 out0 = _mm_mul_ps(in0, i0o0);
        __m128 out1 = _mm_mul_ps(in0, i0o1);
        out0 = _mm_add_ps(out0, _mm_mul_ps(in1, i1o0));
        out1 = _mm_add_ps(out1, _mm_mul_ps(in1, i1o1));
        _mm_storeu_ps(out + out_stride * 0, out0);
        _mm_storeu_ps(out + out_stride * 1, out1);
        in += 4;
        out += 4;
        samples -= 4;
    }
    mix_channels_Default<2, 2>(matrix, samples, out_stride, out, in_stride, in);
}



void mix_channels_x64_SSE(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    size_t combined = (in_channels << 16) | out_channels;
    switch (combined){
    case 0x00010001:
        mix_channels_x64_SSE<1, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020001:
        mix_channels_x64_SSE<2, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00010002:
        mix_channels_x64_SSE<1, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020002:
        mix_channels_x64_SSE<2, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    default:
        mix_channels_generic_x64_SSE(
            matrix, samples,
            out_channels, out_stride, out,
            in_channels, in_stride, in
        );
    }
}




class Test_x64_SSE : public Test{
public:
    Test_x64_SSE(
        size_t in_channels,
        size_t out_channels,
        size_t samples
    )
        : Test(
            "x64_SSE",
            in_channels,
            out_channels,
            samples,
            (Convert)mix_channels_Default,
            (Convert)mix_channels_x64_SSE
        )
    {}
};
void add_tests_x64_SSE(UnitTestDatabase& database){
    for (size_t in_channels = 1; in_channels <= 4; in_channels++){
        for (size_t out_channels = 1; out_channels <= 4; out_channels++){
            for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
                database.add<Test_x64_SSE>(in_channels, out_channels, samples);
            }
        }
    }
}


}
}
}
#endif
