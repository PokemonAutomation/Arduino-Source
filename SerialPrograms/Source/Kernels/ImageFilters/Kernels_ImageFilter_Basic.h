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
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
);



//  Same as above, but multiple filters.
//  The purpose is to reduce passes over the entire image.
//  All matricies must have the same dimensions.
struct FilterRgb32RangeFilter{
    uint32_t* data;     //  Pointer will be overwritten.
    const size_t bytes_per_row;
    const uint32_t mins;
    const uint32_t maxs;
    const uint32_t replacement;
    const bool invert;

    size_t pixels_in_range;

    FilterRgb32RangeFilter(
        uint32_t* p_data, size_t p_bytes_per_row,
        uint32_t p_mins, uint32_t p_maxs, uint32_t p_replacement, bool p_invert
    )
        : data(p_data)
        , bytes_per_row(p_bytes_per_row)
        , mins(p_mins)
        , maxs(p_maxs)
        , replacement(p_replacement)
        , invert(p_invert)
    {}
};
void filter_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);





}
}
#endif
