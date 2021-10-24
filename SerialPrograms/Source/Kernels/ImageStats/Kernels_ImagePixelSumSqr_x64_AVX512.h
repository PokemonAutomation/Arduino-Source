/*  Pixel Mean+Stddev (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImagePixelSumSqr_x64_AVX512_H
#define PokemonAutomation_Kernels_ImagePixelSumSqr_x64_AVX512_H

#include "Kernels_ImagePixelSumSqr.h"

namespace PokemonAutomation{
namespace Kernels{


void pixel_sum_sqr_x64_AVX512(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);



}
}
#endif
