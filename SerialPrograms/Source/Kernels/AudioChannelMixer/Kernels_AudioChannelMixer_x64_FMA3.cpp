/*  Audio Channel Mixer (x64 FMA3)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioChannelMixer_TestInfra.h"
#include "Kernels_AudioChannelMixer_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



PA_FORCE_INLINE void mix_channels_generic_x64_FMA3(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    while (samples > 8){
        for (size_t o = 0; o < out_channels; o++){
            __m256 x = _mm256_setzero_ps();
            for (size_t i = 0; i < in_channels; i++){
                x = _mm256_fmadd_ps(
                    _mm256_loadu_ps(in + in_stride * i),
                    _mm256_set1_ps(matrix[i + in_channels * o]),
                    x
                );
            }
            _mm256_storeu_ps(out + out_stride * o, x);
        }
        in += 8;
        out += 8;
        samples -= 8;
    }
    mix_channels_GenericDefault(
        matrix, samples,
        out_channels, out_stride, out,
        in_channels, in_stride, in
    );
}
template <size_t in_channels, size_t out_channels>
void mix_channels_x64_FMA3(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
);
template <>
void mix_channels_x64_FMA3<1, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m256 i0o0 = _mm256_set1_ps(matrix[0]);
    while (samples > 8){
        __m256 in0 = _mm256_loadu_ps(in);
        __m256 out0 = _mm256_mul_ps(in0, i0o0);
        _mm256_storeu_ps(out, out0);
        in += 8;
        out += 8;
        samples -= 8;
    }
    mix_channels_Default<1, 1>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_x64_FMA3<1, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m256 i0o0 = _mm256_set1_ps(matrix[0]);
    __m256 i0o1 = _mm256_set1_ps(matrix[1]);
    while (samples > 8){
        __m256 in0 = _mm256_loadu_ps(in);
        __m256 out0 = _mm256_mul_ps(in0, i0o0);
        __m256 out1 = _mm256_mul_ps(in0, i0o1);
        _mm256_storeu_ps(out + out_stride * 0, out0);
        _mm256_storeu_ps(out + out_stride * 1, out1);
        in += 8;
        out += 8;
        samples -= 8;
    }
    mix_channels_Default<1, 2>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_x64_FMA3<2, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m256 i0o0 = _mm256_set1_ps(matrix[0]);
    __m256 i1o0 = _mm256_set1_ps(matrix[1]);
    while (samples > 8){
        __m256 out0 = _mm256_mul_ps(i0o0, _mm256_loadu_ps(in + in_stride * 0));
        out0 = _mm256_fmadd_ps(i1o0, _mm256_loadu_ps(in + in_stride * 1), out0);
        _mm256_storeu_ps(out, out0);
        in += 8;
        out += 8;
        samples -= 8;
    }
    mix_channels_Default<2, 1>(matrix, samples, out_stride, out, in_stride, in);
}
template <>
void mix_channels_x64_FMA3<2, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m256 i0o0 = _mm256_set1_ps(matrix[0]);
    __m256 i1o0 = _mm256_set1_ps(matrix[1]);
    __m256 i0o1 = _mm256_set1_ps(matrix[2]);
    __m256 i1o1 = _mm256_set1_ps(matrix[3]);
    while (samples > 8){
        __m256 in0 = _mm256_loadu_ps(in + in_stride * 0);
        __m256 in1 = _mm256_loadu_ps(in + in_stride * 1);
        __m256 out0 = _mm256_mul_ps(in0, i0o0);
        __m256 out1 = _mm256_mul_ps(in0, i0o1);
        out0 = _mm256_fmadd_ps(in1, i1o0, out0);
        out1 = _mm256_fmadd_ps(in1, i1o1, out1);
        _mm256_storeu_ps(out + out_stride * 0, out0);
        _mm256_storeu_ps(out + out_stride * 1, out1);
        in += 8;
        out += 8;
        samples -= 8;
    }
    mix_channels_Default<2, 2>(matrix, samples, out_stride, out, in_stride, in);
}



void mix_channels_x64_FMA3(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    size_t combined = (in_channels << 16) | out_channels;
    switch (combined){
    case 0x00010001:
        mix_channels_x64_FMA3<1, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020001:
        mix_channels_x64_FMA3<2, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00010002:
        mix_channels_x64_FMA3<1, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020002:
        mix_channels_x64_FMA3<2, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    default:
        mix_channels_generic_x64_FMA3(
            matrix, samples,
            out_channels, out_stride, out,
            in_channels, in_stride, in
        );
    }
}




class Test_x64_FMA3 : public Test{
public:
    Test_x64_FMA3(
        size_t in_channels,
        size_t out_channels,
        size_t samples
    )
        : Test(
            "x64_FMA3",
            in_channels,
            out_channels,
            samples,
            (Convert)mix_channels_Default,
            (Convert)mix_channels_x64_FMA3
        )
    {}
};
void add_tests_x64_FMA3(UnitTestDatabase& database){
    for (size_t in_channels = 1; in_channels <= 4; in_channels++){
        for (size_t out_channels = 1; out_channels <= 4; out_channels++){
            for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
                database.add<Test_x64_FMA3>(in_channels, out_channels, samples);
            }
        }
    }
}


}
}
}
#endif
