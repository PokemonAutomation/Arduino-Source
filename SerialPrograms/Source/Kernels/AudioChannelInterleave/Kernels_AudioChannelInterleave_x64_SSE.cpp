/*  Audio Channel Interleave (x64 SSE)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <emmintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioChannelInterleave_TestInfra.h"
#include "Kernels_AudioChannelInterleave_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelInterleave{



template <size_t channels>
void channel_deinterleave_x64_SSE(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
template <size_t channels>
void channel_interleave_x64_SSE(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);



template <>
void channel_deinterleave_x64_SSE<2>(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    while (samples >= 4){
        __m128 in0, in1;
        __m128 channel0, channel1;
        in0 = _mm_loadu_ps(interleaved_stream + 0);
        in1 = _mm_loadu_ps(interleaved_stream + 4);
        //  Pull the even and odd lanes out of both registers:
        //  channel0 = [in0[0], in0[2], in1[0], in1[2]]
        //  channel1 = [in0[1], in0[3], in1[1], in1[3]]
        channel0 = _mm_shuffle_ps(in0, in1, 136);
        channel1 = _mm_shuffle_ps(in0, in1, 221);
        _mm_storeu_ps(strided_stream + 0*stride, channel0);
        _mm_storeu_ps(strided_stream + 1*stride, channel1);

        strided_stream += 4;
        interleaved_stream += 8;
        samples -= 4;
    }
    if (samples > 0){
        channel_deinterleave_Default(2, samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void channel_interleave_x64_SSE<2>(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    while (samples >= 4){
        __m128 in0, in1;
        __m128 channel0, channel1;
        channel0 = _mm_loadu_ps(strided_stream + 0*stride);
        channel1 = _mm_loadu_ps(strided_stream + 1*stride);
        //  unpacklo/unpackhi are the exact inverse of the shuffles above:
        //  in0 = [c0[0], c1[0], c0[1], c1[1]]
        //  in1 = [c0[2], c1[2], c0[3], c1[3]]
        in0 = _mm_unpacklo_ps(channel0, channel1);
        in1 = _mm_unpackhi_ps(channel0, channel1);
        _mm_storeu_ps(interleaved_stream + 0, in0);
        _mm_storeu_ps(interleaved_stream + 4, in1);

        strided_stream += 4;
        interleaved_stream += 8;
        samples -= 4;
    }
    if (samples > 0){
        channel_interleave_Default(2, samples, stride, strided_stream, interleaved_stream);
    }
}




void channel_deinterleave_x64_SSE(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_deinterleave_x64_SSE<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_deinterleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}
void channel_interleave_x64_SSE(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_interleave_x64_SSE<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_interleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}




class Test_deinterleave_x64_SSE : public Test{
public:
    Test_deinterleave_x64_SSE(size_t channels, size_t samples)
        : Test(
            "deinterleave_x64_SSE",
            channels, samples,
            channel_deinterleave_x64_SSE,
            channel_interleave_Default
        )
    {}
};
class Test_interleave_x64_SSE : public Test{
public:
    Test_interleave_x64_SSE(size_t channels, size_t samples)
        : Test(
            "interleave_x64_SSE",
            channels, samples,
            channel_deinterleave_Default,
            channel_interleave_x64_SSE
        )
    {}
};
void add_tests_x64_SSE(UnitTestDatabase& database){
    for (size_t c = 1; c <= 8; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_deinterleave_x64_SSE>(c, samples);
            database.add<Test_interleave_x64_SSE>(c, samples);
        }
    }
}




}
}
}
#endif
