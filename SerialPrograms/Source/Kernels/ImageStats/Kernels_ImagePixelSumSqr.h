/*  Pixel Sum + Sum of Squares
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_ImagePixelSumSqr_H
#define PokemonAutomation_Kernels_ImagePixelSumSqr_H

#include <stdint.h>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{


struct PixelSums{
    size_t count = 0;
    uint64_t sumR = 0;
    uint64_t sumG = 0;
    uint64_t sumB = 0;
    uint64_t sqrR = 0;
    uint64_t sqrG = 0;
    uint64_t sqrB = 0;
};


//  Alpha on "image" is ignored. All pixels are considered active.
//  Pixels on "alpha" are considered active if the alpha is >= 128.
void pixel_sum_sqr(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);


}
}
#endif
