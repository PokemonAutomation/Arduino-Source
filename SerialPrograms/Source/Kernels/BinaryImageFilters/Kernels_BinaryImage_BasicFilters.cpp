/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_BinaryImage_BasicFilters.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels_BinaryImage_BasicFilters_Default.h"
#ifdef PA_Arch_x64_SSE42
#include "Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
#endif
#ifdef PA_Arch_x64_AVX2
#include "Kernels_BinaryImage_BasicFilters_x64_AVX2.h"
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels_BinaryImage_BasicFilters_x64_AVX512.h"
#endif

namespace PokemonAutomation{
namespace Kernels{



void filter_min_rgb32(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
){
#if 0
#elif defined PA_Arch_x64_AVX512
    MinRgbFilter_x64_AVX512 filter(min_alpha, min_red, min_green, min_blue);
#elif defined PA_Arch_x64_AVX2
    MinRgbFilter_x64_AVX2 filter(min_alpha, min_red, min_green, min_blue);
#elif defined PA_Arch_x64_SSE42
    MinRgbFilter_x64_SSE41 filter(min_alpha, min_red, min_green, min_blue);
#else
    MinRgbFilter_Default filter(min_alpha, min_red, min_green, min_blue);
#endif
    rgb32_to_binary_image(binary_image, image, bytes_per_row, filter);
}

void filter_rgb32_range(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
#if 0
#elif defined PA_Arch_x64_AVX512
    RgbRangeFilter_x64_AVX512 filter(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#elif defined PA_Arch_x64_AVX2
    RgbRangeFilter_x64_AVX2 filter(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#elif defined PA_Arch_x64_SSE42
    RgbRangeFilter_x64_SSE41 filter(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#else
    RgbRangeFilter_Default filter(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#endif
    rgb32_to_binary_image(binary_image, image, bytes_per_row, filter);
}











}
}
