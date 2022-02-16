/*  Image Filters Basic
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Perform a filter over an image and replace pixels that match the filter.
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_Basic_H
#define PokemonAutomation_Kernels_ImageFilter_Basic_H

#include <stdint.h>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{


void filter_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t replacement0, uint32_t mins0, uint32_t maxs0, bool invert0
);
void filter2_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t replacement0, uint32_t mins0, uint32_t maxs0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t replacement1, uint32_t mins1, uint32_t maxs1, bool invert1
);
void filter4_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t replacement0, uint32_t mins0, uint32_t maxs0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t replacement1, uint32_t mins1, uint32_t maxs1, bool invert1,
    uint32_t* out2, size_t bytes_per_row2, uint32_t replacement2, uint32_t mins2, uint32_t maxs2, bool invert2,
    uint32_t* out3, size_t bytes_per_row3, uint32_t replacement3, uint32_t mins3, uint32_t maxs3, bool invert3
);



}
}
#endif
