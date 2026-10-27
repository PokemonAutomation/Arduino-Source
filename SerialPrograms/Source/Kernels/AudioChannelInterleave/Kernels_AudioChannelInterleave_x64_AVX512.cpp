/*  Audio Channel Interleave (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <stdint.h>
#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioChannelInterleave_TestInfra.h"
#include "Kernels_AudioChannelInterleave_Default.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelInterleave{



template <size_t channels>
void channel_deinterleave_x64_AVX512(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
template <size_t channels>
void channel_interleave_x64_AVX512(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);



template <>
void channel_deinterleave_x64_AVX512<2>(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    while (samples > 16){
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_loadu_ps(interleaved_stream +  0);
        r1 = _mm512_loadu_ps(interleaved_stream + 16);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31), r1);
        _mm512_storeu_ps(strided_stream + 0*stride, s0);
        _mm512_storeu_ps(strided_stream + 1*stride, s1);

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        uint64_t double_mask = ((uint64_t)1 << samples*2) - 1;
        __mmask16 mask0 = (__mmask16)double_mask;
        __mmask16 mask1 = (__mmask16)(double_mask >> 16);
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_maskz_loadu_ps(mask0, interleaved_stream +  0);
        r1 = _mm512_maskz_loadu_ps(mask1, interleaved_stream + 16);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31), r1);
        _mm512_mask_storeu_ps(strided_stream + 0*stride, mask, s0);
        _mm512_mask_storeu_ps(strided_stream + 1*stride, mask, s1);
    }
}
template <>
void channel_interleave_x64_AVX512<2>(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    while (samples > 16){
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_loadu_ps(strided_stream + 0*stride);
        r1 = _mm512_loadu_ps(strided_stream + 1*stride);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31), r1);
        _mm512_storeu_ps(interleaved_stream +  0, s0);
        _mm512_storeu_ps(interleaved_stream + 16, s1);

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        uint64_t double_mask = ((uint64_t)1 << samples*2) - 1;
        __mmask16 mask0 = (__mmask16)double_mask;
        __mmask16 mask1 = (__mmask16)(double_mask >> 16);
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_maskz_loadu_ps(mask, strided_stream + 0*stride);
        r1 = _mm512_maskz_loadu_ps(mask, strided_stream + 1*stride);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31), r1);
        _mm512_mask_storeu_ps(interleaved_stream +  0, mask0, s0);
        _mm512_mask_storeu_ps(interleaved_stream + 16, mask1, s1);
    }
}




void channel_deinterleave_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_deinterleave_x64_AVX512<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_deinterleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}
void channel_interleave_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_interleave_x64_AVX512<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_interleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}





class Test_deinterleave_x64_AVX512 : public Test{
public:
    Test_deinterleave_x64_AVX512(size_t channels, size_t samples)
        : Test(
            "deinterleave_x64_AVX512",
            channels, samples,
            channel_deinterleave_x64_AVX512,
            channel_interleave_Default
        )
    {}
};
class Test_interleave_x64_AVX512 : public Test{
public:
    Test_interleave_x64_AVX512(size_t channels, size_t samples)
        : Test(
            "interleave_x64_AVX512",
            channels, samples,
            channel_deinterleave_Default,
            channel_interleave_x64_AVX512
        )
    {}
};
void add_tests_x64_AVX512(UnitTestDatabase& database){
    for (size_t c = 1; c <= 8; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_deinterleave_x64_AVX512>(c, samples);
            database.add<Test_interleave_x64_AVX512>(c, samples);
        }
    }
}




}
}
}
#endif
