/*  Audio Channel Mixer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_AudioChannelMixer.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



void mix_channels_Default(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
);
void add_tests_Default(UnitTestDatabase& database);



void mix_channels_x64_SSE(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
);
void add_tests_x64_SSE(UnitTestDatabase& database);



void mix_channels_x64_FMA3(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
);
void add_tests_x64_FMA3(UnitTestDatabase& database);



void mix_channels_x64_AVX512(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
);
void add_tests_x64_AVX512(UnitTestDatabase& database);





void mix_channels(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        mix_channels_x64_AVX512(matrix, samples, out_channels, out_stride, out, in_channels, in_stride, in);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        mix_channels_x64_FMA3(matrix, samples, out_channels, out_stride, out, in_channels, in_stride, in);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        mix_channels_x64_SSE(matrix, samples, out_channels, out_stride, out, in_channels, in_stride, in);
        return;
    }
#endif

    mix_channels_Default(matrix, samples, out_channels, out_stride, out, in_channels, in_stride, in);
    return;
}




void add_tests(UnitTestDatabase& database){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        add_tests_x64_SSE(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        add_tests_x64_FMA3(database);
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
