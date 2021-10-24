/*  Pixel Mean+Stddev (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImagePixelSumSqr_Default_H
#define PokemonAutomation_Kernels_ImagePixelSumSqr_Default_H

#include "Kernels_ImagePixelSumSqr.h"

namespace PokemonAutomation{
namespace Kernels{


void pixel_sum_sqr_Default(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);



}
}
#endif
