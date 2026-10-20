/*  Audio Channel Mixer (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioChannelMixer_TestInfra.h"
#include "Kernels_AudioChannelMixer_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



PA_FORCE_INLINE void mix_channels_generic_x64_AVX512(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    while (samples > 16){
        for (size_t o = 0; o < out_channels; o++){
            __m512 x = _mm512_setzero_ps();
            for (size_t i = 0; i < in_channels; i++){
                x = _mm512_fmadd_ps(
                    _mm512_loadu_ps(in + in_stride * i),
                    _mm512_set1_ps(matrix[i + in_channels * o]),
                    x
                );
            }
            _mm512_storeu_ps(out + out_stride * o, x);
        }
        in += 16;
        out += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        for (size_t o = 0; o < out_channels; o++){
            __m512 x = _mm512_setzero_ps();
            for (size_t i = 0; i < in_channels; i++){
                x = _mm512_fmadd_ps(
                    _mm512_maskz_loadu_ps(mask, in + in_stride * i),
                    _mm512_set1_ps(matrix[i + in_channels * o]),
                    x
                );
            }
            _mm512_mask_storeu_ps(out + out_stride * o, mask, x);
        }
    }
}
template <size_t in_channels, size_t out_channels>
void mix_channels_x64_AVX512(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
);
template <>
void mix_channels_x64_AVX512<1, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m512 i0o0 = _mm512_set1_ps(matrix[0]);
    while (samples > 16){
        __m512 in0 = _mm512_loadu_ps(in);
        __m512 out0 = _mm512_mul_ps(in0, i0o0);
        _mm512_storeu_ps(out, out0);
        in += 16;
        out += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 in0 = _mm512_maskz_loadu_ps(mask, in);
        __m512 out0 = _mm512_mul_ps(in0, i0o0);
        _mm512_mask_storeu_ps(out, mask, out0);
    }
}
template <>
void mix_channels_x64_AVX512<1, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m512 i0o0 = _mm512_set1_ps(matrix[0]);
    __m512 i0o1 = _mm512_set1_ps(matrix[1]);
    while (samples > 16){
        __m512 in0 = _mm512_loadu_ps(in);
        __m512 out0 = _mm512_mul_ps(in0, i0o0);
        __m512 out1 = _mm512_mul_ps(in0, i0o1);
        _mm512_storeu_ps(out + out_stride * 0, out0);
        _mm512_storeu_ps(out + out_stride * 1, out1);
        in += 16;
        out += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 in0 = _mm512_maskz_loadu_ps(mask, in);
        __m512 out0 = _mm512_mul_ps(in0, i0o0);
        __m512 out1 = _mm512_mul_ps(in0, i0o1);
        _mm512_mask_storeu_ps(out + out_stride * 0, mask, out0);
        _mm512_mask_storeu_ps(out + out_stride * 1, mask, out1);
    }
}
template <>
void mix_channels_x64_AVX512<2, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m512 i0o0 = _mm512_set1_ps(matrix[0]);
    __m512 i1o0 = _mm512_set1_ps(matrix[1]);
    while (samples > 16){
        __m512 out0 = _mm512_mul_ps(i0o0, _mm512_loadu_ps(in + in_stride * 0));
        out0 = _mm512_fmadd_ps(i1o0, _mm512_loadu_ps(in + in_stride * 1), out0);
        _mm512_storeu_ps(out, out0);
        in += 16;
        out += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 out0 = _mm512_mul_ps(i0o0, _mm512_maskz_loadu_ps(mask, in + in_stride * 0));
        out0 = _mm512_fmadd_ps(i1o0, _mm512_maskz_loadu_ps(mask, in + in_stride * 1), out0);
        _mm512_mask_storeu_ps(out, mask, out0);
    }
}
template <>
void mix_channels_x64_AVX512<2, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    __m512 i0o0 = _mm512_set1_ps(matrix[0]);
    __m512 i1o0 = _mm512_set1_ps(matrix[1]);
    __m512 i0o1 = _mm512_set1_ps(matrix[2]);
    __m512 i1o1 = _mm512_set1_ps(matrix[3]);
    while (samples > 16){
        __m512 in0 = _mm512_loadu_ps(in + in_stride * 0);
        __m512 in1 = _mm512_loadu_ps(in + in_stride * 1);
        __m512 out0 = _mm512_mul_ps(in0, i0o0);
        __m512 out1 = _mm512_mul_ps(in0, i0o1);
        out0 = _mm512_fmadd_ps(in1, i1o0, out0);
        out1 = _mm512_fmadd_ps(in1, i1o1, out1);
        _mm512_storeu_ps(out + out_stride * 0, out0);
        _mm512_storeu_ps(out + out_stride * 1, out1);
        in += 16;
        out += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 in0 = _mm512_maskz_loadu_ps(mask, in + in_stride * 0);
        __m512 in1 = _mm512_maskz_loadu_ps(mask, in + in_stride * 1);
        __m512 out0 = _mm512_mul_ps(in0, i0o0);
        __m512 out1 = _mm512_mul_ps(in0, i0o1);
        out0 = _mm512_fmadd_ps(in1, i1o0, out0);
        out1 = _mm512_fmadd_ps(in1, i1o1, out1);
        _mm512_mask_storeu_ps(out + out_stride * 0, mask, out0);
        _mm512_mask_storeu_ps(out + out_stride * 1, mask, out1);
    }
}



void mix_channels_x64_AVX512(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    size_t combined = (in_channels << 16) | out_channels;
    switch (combined){
    case 0x00010001:
        mix_channels_x64_AVX512<1, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020001:
        mix_channels_x64_AVX512<2, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00010002:
        mix_channels_x64_AVX512<1, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020002:
        mix_channels_x64_AVX512<2, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    default:
        mix_channels_generic_x64_AVX512(
            matrix, samples,
            out_channels, out_stride, out,
            in_channels, in_stride, in
        );
    }
}




class Test_x64_AVX512 : public Test{
public:
    Test_x64_AVX512(
        size_t in_channels,
        size_t out_channels,
        size_t samples
    )
        : Test(
            "x64_AVX512",
            in_channels,
            out_channels,
            samples,
            (Convert)mix_channels_Default,
            (Convert)mix_channels_x64_AVX512
        )
    {}
};
void add_tests_x64_AVX512(UnitTestDatabase& database){
    for (size_t in_channels = 1; in_channels <= 4; in_channels++){
        for (size_t out_channels = 1; out_channels <= 4; out_channels++){
            for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
                database.add<Test_x64_AVX512>(in_channels, out_channels, samples);
            }
        }
    }
}


}
}
}
#endif
