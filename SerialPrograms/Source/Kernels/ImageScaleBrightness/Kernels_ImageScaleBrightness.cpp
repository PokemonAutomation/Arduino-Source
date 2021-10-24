/*  Scale Brightness
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_ImageScaleBrightness.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels_ImageScaleBrightness_Default.h"
#ifdef PA_Arch_x64_SSE41
#include "Kernels_ImageScaleBrightness_x64_SSE41.h"
#endif
#ifdef PA_Arch_x64_AVX2
#include "Kernels_ImageScaleBrightness_x64_AVX2.h"
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels_ImageScaleBrightness_x64_AVX512.h"
#endif

namespace PokemonAutomation{
namespace Kernels{


void scale_brightness(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
){
#if 0
#elif defined PA_Arch_x64_AVX512
    scale_brightness_x64_AVX512(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
#elif defined PA_Arch_x64_AVX2
    scale_brightness_x64_AVX2(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
#elif defined PA_Arch_x64_SSE41
    scale_brightness_x64_SSE41(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
#else
    scale_brightness_Default(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
#endif
}




}
}
