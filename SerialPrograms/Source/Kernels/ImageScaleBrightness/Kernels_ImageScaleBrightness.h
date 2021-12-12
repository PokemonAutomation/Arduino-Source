/*  Scale Brightness
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImageScaleBrightness_H
#define PokemonAutomation_Kernels_ImageScaleBrightness_H

#include <cstdint>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{


void scale_brightness(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
);


}
}
#endif
