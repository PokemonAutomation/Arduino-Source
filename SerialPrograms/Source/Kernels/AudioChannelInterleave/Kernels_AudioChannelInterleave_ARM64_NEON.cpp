/*  Audio Channel Interleave (ARM64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
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
void channel_deinterleave_ARM64_NEON(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
template <size_t channels>
void channel_interleave_ARM64_NEON(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);



template <>
void channel_deinterleave_ARM64_NEON<2>(
    size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    //  Two 128-bit multi-structure loads deinterleave four samples at a
    //  time: val[0] gets the even lanes (channel 0) and val[1] the odd
    //  lanes (channel 1), so no explicit permutes are needed.
    while (samples >= 4){
        float32x4x2_t p = vld2q_f32(interleaved_stream);
        vst1q_f32(strided_stream + 0*stride, p.val[0]);
        vst1q_f32(strided_stream + 1*stride, p.val[1]);

        strided_stream += 4;
        interleaved_stream += 8;
        samples -= 4;
    }
    if (samples > 0){
        channel_deinterleave_Default(2, samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void channel_interleave_ARM64_NEON<2>(
    size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    //  The inverse of the deinterleave: gather the two channels into a
    //  multi-structure pair and store them interleaved.
    while (samples >= 4){
        float32x4x2_t p;
        p.val[0] = vld1q_f32(strided_stream + 0*stride);
        p.val[1] = vld1q_f32(strided_stream + 1*stride);
        vst2q_f32(interleaved_stream, p);

        strided_stream += 4;
        interleaved_stream += 8;
        samples -= 4;
    }
    if (samples > 0){
        channel_interleave_Default(2, samples, stride, strided_stream, interleaved_stream);
    }
}




void channel_deinterleave_ARM64_NEON(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_deinterleave_ARM64_NEON<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_deinterleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}
void channel_interleave_ARM64_NEON(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 2:
        channel_interleave_ARM64_NEON<2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_interleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}




class Test_deinterleave_ARM64_NEON : public Test{
public:
    Test_deinterleave_ARM64_NEON(size_t channels, size_t samples)
        : Test(
            "deinterleave_ARM64_NEON",
            channels, samples,
            channel_deinterleave_ARM64_NEON,
            channel_interleave_Default
        )
    {}
};
class Test_interleave_ARM64_NEON : public Test{
public:
    Test_interleave_ARM64_NEON(size_t channels, size_t samples)
        : Test(
            "interleave_ARM64_NEON",
            channels, samples,
            channel_deinterleave_Default,
            channel_interleave_ARM64_NEON
        )
    {}
};
void add_tests_ARM64_NEON(UnitTestDatabase& database){
    for (size_t c = 1; c <= 8; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_deinterleave_ARM64_NEON>(c, samples);
            database.add<Test_interleave_ARM64_NEON>(c, samples);
        }
    }
}




}
}
}
#endif
