/*  Audio Resampler (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include "Kernels_AudioResampler_BlockKernel_x64_AVX512.h"
#include "Kernels_AudioResampler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



void run_block32_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    run_block32_tap8_x64_AVX512(table, in_index, in_samples, out_index, out_samples);
}
void run_block128_up_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    run_block128_x64_AVX512<1>(table, in_index, in_samples, out_index, out_samples);
}
void run_block128_down2_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    run_block128_x64_AVX512<2>(table, in_index, in_samples, out_index, out_samples);
}



}
}
}
#endif
