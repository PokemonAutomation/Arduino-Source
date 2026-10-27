/*  Audio Channel Interleave (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell
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
void channel_deinterleave_x64_AVX2(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
template <size_t channels>
void channel_interleave_x64_AVX2(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);



template <>
void channel_deinterleave_x64_AVX2<2>(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    while (samples >= 8){
        __m256 r0, r1;
        __m256 s0, s1;
        r0 = _mm256_loadu_ps(interleaved_stream + 0);
        r1 = _mm256_loadu_ps(interleaved_stream + 8);
        r0 = _mm256_permutevar8x32_ps(r0, _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7));
        r1 = _mm256_permutevar8x32_ps(r1, _mm256_setr_epi32(1, 3, 5, 7, 0, 2, 4, 6));
        s0 = _mm256_blend_ps(r0, r1, 0xf0);
        s1 = _mm256_permute2f128_ps(r0, r1, 33);
        _mm256_storeu_ps(strided_stream + 0*stride, s0);
        _mm256_storeu_ps(strided_stream + 1*stride, s1);

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        channel_deinterleave_Default(2, samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void channel_interleave_x64_AVX2<2>(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    while (samples >= 8){
        __m256 r0, r1;
        __m256 s0, s1;
        r0 = _mm256_loadu_ps(strided_stream + 0*stride);
        r1 = _mm256_loadu_ps(strided_stream + 1*stride);
        r0 = _mm256_permutevar8x32_ps(r0, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
        r1 = _mm256_permutevar8x32_ps(r1, _mm256_setr_epi32(7, 0, 4, 1, 5, 2, 6, 3));
        s0 = _mm256_blend_ps(r0, r1, 0xaa);
        s1 = _mm256_blend_ps(r0, r1, 0x55);
        s1 = _mm256_permutevar8x32_ps(s1, _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 0));
        _mm256_storeu_ps(interleaved_stream + 0, s0);
        _mm256_storeu_ps(interleaved_stream + 8, s1);

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        channel_interleave_Default(2, samples, stride, strided_stream, interleaved_stream);
    }
}




void channel_deinterleave_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_deinterleave_x64_AVX2<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_deinterleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}
void channel_interleave_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_interleave_x64_AVX2<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_interleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}





class Test_deinterleave_x64_AVX2 : public Test{
public:
    Test_deinterleave_x64_AVX2(size_t channels, size_t samples)
        : Test(
            "deinterleave_x64_AVX2",
            channels, samples,
            channel_deinterleave_x64_AVX2,
            channel_interleave_Default
        )
    {}
};
class Test_interleave_x64_AVX2 : public Test{
public:
    Test_interleave_x64_AVX2(size_t channels, size_t samples)
        : Test(
            "interleave_x64_AVX2",
            channels, samples,
            channel_deinterleave_Default,
            channel_interleave_x64_AVX2
        )
    {}
};
void add_tests_x64_AVX2(UnitTestDatabase& database){
    for (size_t c = 1; c <= 8; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_deinterleave_x64_AVX2>(c, samples);
            database.add<Test_interleave_x64_AVX2>(c, samples);
        }
    }
}




}
}
}
#endif
