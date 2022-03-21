/*  Scale Brightness
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/CpuId.h"
#include "Kernels_ImageScaleBrightness.h"

namespace PokemonAutomation{
namespace Kernels{


void scale_brightness_Default(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
);
void scale_brightness_x64_SSE41(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
);
void scale_brightness_x64_AVX2(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
);
void scale_brightness_x64_AVX512(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
);



void scale_brightness(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
){
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        scale_brightness_x64_AVX512(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
        return;
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        scale_brightness_x64_AVX2(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
        return;
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        scale_brightness_x64_SSE41(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
        return;
    }
#endif
    scale_brightness_Default(width, height, image, bytes_per_row, scaleR, scaleG, scaleB);
}




}
}
