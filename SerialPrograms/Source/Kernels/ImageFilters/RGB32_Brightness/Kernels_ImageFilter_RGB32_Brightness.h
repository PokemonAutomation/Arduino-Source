/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Perform a filter over an image and replace pixels that match the filter.
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_RGB32_Brightness_H
#define PokemonAutomation_Kernels_ImageFilter_RGB32_Brightness_H

#include <stdint.h>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{



size_t filter_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t min_brightness, uint32_t max_brightness
);


size_t to_blackwhite_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t min_brightness, uint32_t max_brightness
);




}
}
#endif
