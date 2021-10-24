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


#if 0

#elif defined PA_Arch_x64_AVX512

void filter_min_rgb32(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
){
    MinRgbFilter_x64_AVX512 filter(min_alpha, min_red, min_green, min_blue);
    rgb32_to_binary_image(binary_image, image, bytes_per_row, filter);
}

#elif defined PA_Arch_x64_AVX2

void filter_min_rgb32(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
){
    MinRgbFilter_x64_AVX2 filter(min_alpha, min_red, min_green, min_blue);
    rgb32_to_binary_image(binary_image, image, bytes_per_row, filter);
}

#elif defined PA_Arch_x64_SSE42

void filter_min_rgb32(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
){
    MinRgbFilter_x64_SSE41 filter(min_alpha, min_red, min_green, min_blue);
    rgb32_to_binary_image(binary_image, image, bytes_per_row, filter);
}

#else

void filter_min_rgb32(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
){
    MinRgbFilter_Default filter(min_alpha, min_red, min_green, min_blue);
    rgb32_to_binary_image(binary_image, image, bytes_per_row, filter);
}

#endif



}
}
