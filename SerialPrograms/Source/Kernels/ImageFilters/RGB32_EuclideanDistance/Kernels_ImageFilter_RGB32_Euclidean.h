/*  Image Filters RGB32 Euclidean Distance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Perform a filter over an image and replace pixels that match the filter.
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_RGB32_Euclidean_H
#define PokemonAutomation_Kernels_ImageFilter_RGB32_Euclidean_H

#include <stdint.h>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{



size_t filter_rgb32_euclidean(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);




}
}
#endif
