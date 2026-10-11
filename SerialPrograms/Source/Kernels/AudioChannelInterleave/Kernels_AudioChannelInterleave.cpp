/*  Audio Channel Interleave
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_AudioChannelInterleave.h"

#include "Kernels_AudioChannelInterleave_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelInterleave{



void channel_deinterleave_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
void channel_interleave_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);
void add_tests_x64_AVX2(UnitTestDatabase& database);


void channel_deinterleave_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
void channel_interleave_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);
void add_tests_x64_AVX512(UnitTestDatabase& database);




void channel_deinterleave(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        channel_deinterleave_x64_AVX512(channels, samples, stride, strided_stream, interleaved_stream);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        channel_deinterleave_x64_AVX2(channels, samples, stride, strided_stream, interleaved_stream);
        return;
    }
#endif

    channel_deinterleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
    return;
}
void channel_interleave(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        channel_interleave_x64_AVX512(channels, samples, stride, strided_stream, interleaved_stream);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        channel_interleave_x64_AVX2(channels, samples, stride, strided_stream, interleaved_stream);
        return;
    }
#endif

    channel_interleave_Default(channels, samples, stride, strided_stream, interleaved_stream);
    return;
}



void add_tests(UnitTestDatabase& database){
    add_tests_Default(database);
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        add_tests_x64_AVX2(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        add_tests_x64_AVX512(database);
    }
#endif
}



}
}
}
