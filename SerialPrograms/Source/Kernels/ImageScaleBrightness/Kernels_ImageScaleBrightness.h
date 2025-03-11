/*  Scale Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_ImageScaleBrightness_H
#define PokemonAutomation_Kernels_ImageScaleBrightness_H

#include <cstdint>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{

// Scale the R, G, B channels of every pixel of the image independently, by `scaleR`, `scaleG`, `scaleB`. The alpha channel is left untouched.
// image: each pixel is represented as uint32_t, where each of the 8 bit is used as alpha (highest bits), r, g, b(lowest bits).
// image is row-major; advance to next row by a step size of `bytes_per_row`.
void scale_brightness(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
);


}
}
#endif
