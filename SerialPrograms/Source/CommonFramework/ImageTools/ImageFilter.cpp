/*  Image Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "ImageFilter.h"

namespace PokemonAutomation{



ImageRGB32 filter_rgb32_range(
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs, Color replace_with, bool replace_color_within_range
){
    ImageRGB32 ret(image.width(), image.height());
    Kernels::filter_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        ret.data(), ret.bytes_per_row(), mins, maxs, (uint32_t)replace_with, replace_color_within_range
    );
    return ret;
}
ImageRGB32 filter_rgb32_range(
    size_t& pixels_in_range,
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs, Color replace_with, bool replace_color_within_range
){
    ImageRGB32 ret(image.width(), image.height());
    pixels_in_range = Kernels::filter_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        ret.data(), ret.bytes_per_row(), mins, maxs, (uint32_t)replace_with, replace_color_within_range
    );
    return ret;
}
std::vector<std::pair<ImageRGB32, size_t>> filter_rgb32_range(
    const ImageViewRGB32& image,
    const std::vector<FilterRgb32Range>& filters
){
    std::vector<std::pair<ImageRGB32, size_t>> ret(filters.size());
    FixedLimitVector<Kernels::FilterRgb32RangeFilter> subfilters(filters.size());
    for (size_t c = 0; c < filters.size(); c++){
        ImageRGB32& out = ret[c].first;
        out = ImageRGB32(image.width(), image.height());
        subfilters.emplace_back(
            out.data(), out.bytes_per_row(),
            filters[c].mins, filters[c].maxs, (uint32_t)filters[c].replacement_color, filters[c].replace_color_within_range
        );
    }
    Kernels::filter_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        subfilters.data(), subfilters.size()
    );
    for (size_t c = 0; c < filters.size(); c++){
        ret[c].second = subfilters[c].pixels_in_range;
    }
    return ret;
}





ImageRGB32 filter_rgb32_euclidean(
    const ImageViewRGB32& image,
    uint32_t expected, double max_euclidean_distance,
    Color replace_with, bool replace_color_within_range
){
    ImageRGB32 ret(image.width(), image.height());
    Kernels::filter_rgb32_euclidean(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        ret.data(), ret.bytes_per_row(), expected, max_euclidean_distance, (uint32_t)replace_with, replace_color_within_range
    );
    return ret;
}
ImageRGB32 filter_rgb32_euclidean(
    size_t& pixels_in_range,
    const ImageViewRGB32& image,
    uint32_t expected, double max_euclidean_distance,
    Color replace_with, bool replace_color_within_range
){
    ImageRGB32 ret(image.width(), image.height());
    pixels_in_range = Kernels::filter_rgb32_euclidean(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        ret.data(), ret.bytes_per_row(), expected, max_euclidean_distance, (uint32_t)replace_with, replace_color_within_range
    );
    return ret;
}






ImageRGB32 to_blackwhite_rgb32_range(
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    ImageRGB32 ret(image.width(), image.height());
    Kernels::to_blackwhite_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        ret.data(), ret.bytes_per_row(), mins, maxs, in_range_black
    );
    return ret;
}
ImageRGB32 to_blackwhite_rgb32_range(
    size_t& pixels_in_range,
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    ImageRGB32 ret(image.width(), image.height());
    pixels_in_range = Kernels::to_blackwhite_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        ret.data(), ret.bytes_per_row(), mins, maxs, in_range_black
    );
    return ret;
}
std::vector<std::pair<ImageRGB32, size_t>> to_blackwhite_rgb32_range(
    const ImageViewRGB32& image,
    const std::vector<BlackWhiteRgb32Range>& filters
){
    std::vector<std::pair<ImageRGB32, size_t>> ret(filters.size());
    FixedLimitVector<Kernels::ToBlackWhiteRgb32RangeFilter> subfilters(filters.size());
    for (size_t c = 0; c < filters.size(); c++){
        ImageRGB32& out = ret[c].first;
        out = ImageRGB32(image.width(), image.height());
        subfilters.emplace_back(
            out.data(), out.bytes_per_row(),
            filters[c].mins, filters[c].maxs, (uint32_t)filters[c].in_range_black
        );
    }
    Kernels::to_blackwhite_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        subfilters.data(), subfilters.size()
    );
    for (size_t c = 0; c < filters.size(); c++){
        ret[c].second = subfilters[c].pixels_in_range;
    }
    return ret;
}

ImageRGB32 filter_green(
    const ImageViewRGB32& image,    
    Color replace_with,
    uint8_t rgb_gap
){
    ImageRGB32 ret(image.width(), image.height());
    Kernels::filter_green(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        ret.data(), ret.bytes_per_row(), (uint32_t)replace_with, rgb_gap
    );
    return ret;
}



}
