/*  Image Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/FixedLimitVector.tpp"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "CommonFramework/ImageTypes/ImageReference.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "ImageFilter.h"

namespace PokemonAutomation{



size_t filter_rgb32_range(ImageRGB32& image, uint32_t mins, uint32_t maxs, Color replace_with, bool invert){
    return Kernels::filter_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        image.data(), image.bytes_per_row(), mins, maxs, (uint32_t)replace_with, invert
    );
}
std::vector<std::pair<QImage, size_t>> filter_rgb32_range(
    const ImageViewRGB32& image,
    const std::vector<FilterRgb32Range>& filters
){
    std::vector<std::pair<QImage, size_t>> ret(filters.size());
    FixedLimitVector<Kernels::FilterRgb32RangeFilter> subfilters(filters.size());
    for (size_t c = 0; c < filters.size(); c++){
        QImage& out = ret[c].first;
        out = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);
        subfilters.emplace_back(
            (uint32_t*)out.bits(), out.bytesPerLine(),
            filters[c].mins, filters[c].maxs, (uint32_t)filters[c].replace_with, filters[c].invert
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






size_t to_blackwhite_rgb32_range(QImage& image, uint32_t mins, uint32_t maxs, bool in_range_black){
    if (!(image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32)){
        image = image.convertToFormat(QImage::Format_RGB32);
    }
    return Kernels::to_blackwhite_rgb32_range(
        (uint32_t*)image.bits(), image.bytesPerLine(), image.width(), image.height(),
        (uint32_t*)image.bits(), image.bytesPerLine(), mins, maxs, in_range_black
    );
}
std::vector<std::pair<QImage, size_t>> to_blackwhite_rgb32_range(
    const ImageViewRGB32& image,
    const std::vector<BlackWhiteRgb32Range>& filters
){
    std::vector<std::pair<QImage, size_t>> ret(filters.size());
    FixedLimitVector<Kernels::ToBlackWhiteRgb32RangeFilter> subfilters(filters.size());
    for (size_t c = 0; c < filters.size(); c++){
        QImage& out = ret[c].first;
        out = QImage((int)image.width(), (int)image.height(), QImage::Format_ARGB32);
        subfilters.emplace_back(
            (uint32_t*)out.bits(), out.bytesPerLine(),
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




}
