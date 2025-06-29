/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Perform a filter over an image and replace pixels that match the filter.
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_RGB32_Range_H
#define PokemonAutomation_Kernels_ImageFilter_RGB32_Range_H

#include <stdint.h>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{


// Change certain color in `image_in` and save output to `image_out`.
//  If `replace_color_within_range` is true, replace the color within range [mins, maxs] with the color `replacement`.
//  If `replace_color_within_range` is false, replace the color outside of the range with the color `replacement`.
//  Returns the # of pixels inside the range [mins, maxs].
size_t filter_rgb32_range(
    const uint32_t* image_in, size_t image_in_bytes_per_row, size_t width, size_t height,
    uint32_t* image_out, size_t image_out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
);


//  Same as above, but multiple filters.
//  The purpose is to reduce passes over the entire image.
//  All matricies must have the same dimensions.
struct FilterRgb32RangeFilter{
    uint32_t* data;       //  Pointer will be overwritten.
    const size_t bytes_per_row;
    const uint32_t replacement;
    const bool invert;
    const uint32_t mins;
    const uint32_t maxs;

    size_t pixels_in_range;

    FilterRgb32RangeFilter(
        uint32_t* p_data, size_t p_bytes_per_row,
        uint32_t p_replacement, bool p_invert,
        uint32_t p_mins, uint32_t p_maxs
    )
        : data(p_data)
        , bytes_per_row(p_bytes_per_row)
        , replacement(p_replacement)
        , invert(p_invert)
        , mins(p_mins)
        , maxs(p_maxs)
    {}
};
void filter_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);







size_t to_blackwhite_rgb32_range(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);


//  Same as above, but multiple filters.
//  The purpose is to reduce passes over the entire image.
//  All matricies must have the same dimensions.
struct ToBlackWhiteRgb32RangeFilter{
    uint32_t* const data;
    const size_t bytes_per_row;
    const uint32_t mins;
    const uint32_t maxs;
    const bool in_range_black;

    size_t pixels_in_range;

    ToBlackWhiteRgb32RangeFilter(
        uint32_t* p_data, size_t p_bytes_per_row,
        uint32_t p_mins, uint32_t p_maxs, bool p_in_range_black
    )
        : data(p_data)
        , bytes_per_row(p_bytes_per_row)
        , mins(p_mins)
        , maxs(p_maxs)
        , in_range_black(p_in_range_black)
    {}
};
void to_blackwhite_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
);





}
}
#endif
