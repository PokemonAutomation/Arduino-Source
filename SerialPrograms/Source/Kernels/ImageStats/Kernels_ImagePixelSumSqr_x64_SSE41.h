/*  Pixel Mean+Stddev (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImagePixelSumSqr_x64_SSE41_H
#define PokemonAutomation_Kernels_ImagePixelSumSqr_x64_SSE41_H

#include "Kernels_ImagePixelSumSqr.h"

namespace PokemonAutomation{
namespace Kernels{


void pixel_sum_sqr_x64_SSE41(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);



}
}
#endif
