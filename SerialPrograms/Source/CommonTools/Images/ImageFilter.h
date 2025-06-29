/*  Image Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_ImageFilter_H
#define PokemonAutomation_CommonTools_ImageFilter_H

#include <cstddef>
#include <vector>
#include "Common/Cpp/Color.h"
#include "Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;


//  If `replace_color_within_range` is true, replace the color range [mins, maxs] with the color `replacement_color`.
//  If `replace_color_within_range` is false, replace the color outside of the range [mins, maxs] with the color
//    `replacement_color`.
ImageRGB32 filter_rgb32_range(
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs,
    Color replacement_color, bool replace_color_within_range
);
//  If `replace_color_within_range` is true, replace the color range [mins, maxs] with the color `replacement_color`.
//  If `replace_color_within_range` is false, replace the color outside of the range [mins, maxs] with the color
//    `replacement_color`.
//  Returns the # of pixels inside the range [mins, maxs] as `pixels_in_range`
ImageRGB32 filter_rgb32_range(
    size_t& pixels_in_range,
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs,
    Color replacement_color, bool replace_color_within_range
);
//  Run multiple filters at once. This is more memory efficient than making
//  multiple calls to one filter at a time.
//  For each filter:
//  If `replace_color_within_range` is true, replace the color range [mins, maxs] with the color `replacement_color`.
//  If `replace_color_within_range` is false, replace the color outside of the range [mins, maxs] with the color
//    `replacement_color`.
//  For each filter, return the filtered image and the # of pixels inside the [mins, maxs] range of the filter.
struct FilterRgb32Range{
    Color replacement_color;
    bool replace_color_within_range;
    uint32_t mins;
    uint32_t maxs;
};
std::vector<std::pair<ImageRGB32, size_t>> filter_rgb32_range(
    const ImageViewRGB32& image,
    const std::vector<FilterRgb32Range>& filters
);



//  Convert the image to black and white.
//  Inside [mins, maxs] is white, otherwise it's black.
//  Set "in_range_black" to true to invert the colors.
//  Both white and black colors have alpha=255.
ImageRGB32 to_blackwhite_rgb32_range(
    const ImageViewRGB32& image,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);
//  Convert the image to black and white.
//  Inside [mins, maxs] is white, otherwise it's black.
//  Set "in_range_black" to true to invert the colors.
//  Both white and black colors have alpha=255.
//  Returns the # of pixels inside the distance as `pixels_in_range`.
ImageRGB32 to_blackwhite_rgb32_range(
    size_t& pixels_in_range,
    const ImageViewRGB32& image,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);
//  Run multiple filters at once. This is more memory efficient than making
//  multiple calls to one filter at a time.
//  For each filter:
//  If `in_range_black` is true, replace the color range [mins, maxs] with color black while the rest white.
//  If `in_range_black` is false, replace the color range [mins, maxs] with color white while the rest black.
//  Both white and black colors have alpha=255.
//  For each filter, return the filtered image and the # of pixels inside the [mins, maxs] range of the filter.
struct BlackWhiteRgb32Range{
    bool in_range_black;
    uint32_t mins;
    uint32_t maxs;
};
std::vector<std::pair<ImageRGB32, size_t>> to_blackwhite_rgb32_range(
    const ImageViewRGB32& image,
    const std::vector<BlackWhiteRgb32Range>& filters
);









//  If `replace_color_within_range` is true, replace the colors within (<=) `max_euclidean_distance` of the
//    `expected_color` with `replacement_color`.
//  If `replace_color_within_range` is false, replace the color outside of the distance with the color `replacement_color`.
ImageRGB32 filter_rgb32_euclidean(
    const ImageViewRGB32& image,
    uint32_t expected_color, double max_euclidean_distance,
    Color replacement_color, bool replace_color_within_range
);
//  If `replace_color_within_range` is true, replace the colors within (<=) `max_euclidean_distance` of the
//    `expected_color` with `replacement_color`.
//  If `replace_color_within_range` is false, replace the color outside of the distance with the color `replacement_color`.
//  Returns the # of pixels inside the distance as `pixels_in_range`.
//  Note: the alpha channel of `image` and `expected_color` are ignored during computation.
ImageRGB32 filter_rgb32_euclidean(
    size_t& pixels_in_range,
    const ImageViewRGB32& image,
    uint32_t expected_color, double max_euclidean_distance,
    Color replacement_color, bool replace_color_within_range
);








//  Similar to "filter_rgb32_range()". But instead of checking if each of
//  the RGB components are within a range, we check if the dot product of the
//  components and a set of weights is within a range. By selecting the weights,
//  you can pick which components to check the brightness of.
ImageRGB32 filter_rgb32_brightness(
    const ImageViewRGB32& image,
    Color replacement_color, bool replace_color_within_range,
    Kernels::Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
//  Similar to "to_blackwhite_rgb32_range()". But instead of checking if each of
//  the RGB components are within a range, we check if the dot product of the
//  components and a set of weights is within a range. By selecting the weights,
//  you can pick which components to check the brightness of.
ImageRGB32 to_blackwhite_rgb32_brightness(
    const ImageViewRGB32& image,
    bool in_range_black,
    Kernels::Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);




// keep all pixels where green is the dominant RGB value. otherwise, replace the pixel with `replace_with`
// `rgb_gap` is the amount that green has to exceed the red or blue value, in order to keep the pixel.
ImageRGB32 filter_green(
    const ImageViewRGB32& image,
    Color replace_with,
    uint8_t rgb_gap = 0
);



}
#endif
