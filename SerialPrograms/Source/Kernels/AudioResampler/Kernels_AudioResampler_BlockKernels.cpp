/*  Audio Resampler Block Kernels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_AudioResampler_BlockKernels.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



BlockKernel get_block_kernel(
    size_t in_sample_rate,
    size_t out_sample_rate,
    uint32_t taps
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        //  The permute-based kernels handle any tap count. The index delta
        //  within a 16-output group bounds how many input lanes each load
        //  must span: 15 for upsampling (in <= out) and 31 for downsampling
        //  up to 2x. Stronger downsampling falls back to the transpose POC.
        if (in_sample_rate <= out_sample_rate){
            return {run_block128_up_x64_AVX512, 128, 1};
        }
        if (in_sample_rate <= 2 * out_sample_rate){
            return {run_block128_down2_x64_AVX512, 128, 1};
        }
        return {run_block32_tap8_x64_AVX512, 32, 8};
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return {run_block16_tap4_x64_FMA3, 16, 4};
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return {run_block8_tap2_x64_SSE2, 8, 2};
    }
#endif
    return {run_block8_Default, 8, 1};
}



}
}
}
