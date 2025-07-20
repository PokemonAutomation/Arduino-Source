/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "BinaryImage_FilterRgb32.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



void filter_by_mask(
    const PackedBinaryMatrix& matrix,
    ImageRGB32& image,
    Color replacement_color,
    bool replace_zero_bits
){
    if (matrix.width() > image.width()){
        dump_image(
            global_logger_tagged(), ProgramInfo(),
            "filter_by_mask-width-matrix" + std::to_string(matrix.width()) + "x" + std::to_string(matrix.height()),
            image
        );
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image width is too small.");
    }
    if (matrix.height() > image.height()){
        dump_image(
            global_logger_tagged(), ProgramInfo(),
            "filter_by_mask-width-matrix" + std::to_string(matrix.width()) + "x" + std::to_string(matrix.height()),
            image
        );
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image height is too small.");
    }
    Kernels::filter_by_mask(
        matrix,
        image.data(), image.bytes_per_row(),
        (uint32_t)replacement_color, replace_zero_bits
    );
}
void filter_by_mask(
    const PackedBinaryMatrix& matrix,
    ImageRGB32& image, size_t offset_x, size_t offset_y,
    Color replacement_color,
    bool replace_zero_bits
){
    if (matrix.width() > image.width()){
        dump_image(
            global_logger_tagged(), ProgramInfo(),
            "filter_by_mask-width-matrix" + std::to_string(matrix.width()) + "x" + std::to_string(matrix.height()),
            image
        );
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image width is too small.");
    }
    if (matrix.height() > image.height()){
        dump_image(
            global_logger_tagged(), ProgramInfo(),
            "filter_by_mask-width-matrix" + std::to_string(matrix.width()) + "x" + std::to_string(matrix.height()),
            image
        );
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image height is too small.");
    }
    size_t bytes_per_row = image.bytes_per_row();
    Kernels::filter_by_mask(
        matrix,
        (uint32_t*)((char*)image.data() + bytes_per_row * offset_y + offset_x * sizeof(uint32_t)),
        bytes_per_row,
        (uint32_t)replacement_color, replace_zero_bits
    );
}





PackedBinaryMatrix compress_rgb32_to_binary_min(
    const ImageViewRGB32& image,
    uint8_t min_red,
    uint8_t min_green,
    uint8_t min_blue
){
    return compress_rgb32_to_binary_range(
        image,
        255, 255,
        min_red, 255,
        min_green, 255,
        min_blue, 255
    );
}
PackedBinaryMatrix compress_rgb32_to_binary_max(
    const ImageViewRGB32& image,
    uint8_t max_red,
    uint8_t max_green,
    uint8_t max_blue
){
    return compress_rgb32_to_binary_range(
        image,
        255, 255,
        0, max_red,
        0, max_green,
        0, max_blue
    );
}
PackedBinaryMatrix compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    return compress_rgb32_to_binary_range(
        image,
        255, 255,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
}
PackedBinaryMatrix compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(), ret,
        ((uint32_t)min_alpha << 24) | ((uint32_t)min_red << 16) | ((uint32_t)min_green << 8) | (uint32_t)min_blue,
        ((uint32_t)max_alpha << 24) | ((uint32_t)max_red << 16) | ((uint32_t)max_green << 8) | (uint32_t)max_blue
    );
    return ret;
}
PackedBinaryMatrix compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs
){
    PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(),
        ret, mins, maxs
    );
    return ret;
}
std::vector<PackedBinaryMatrix> compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    const std::vector<std::pair<uint32_t, uint32_t>>& filters
){
    std::vector<PackedBinaryMatrix> ret;
    FixedLimitVector<Kernels::CompressRgb32ToBinaryRangeFilter> vec(filters.size());
    for (size_t c = 0; c < filters.size(); c++){
        ret.emplace_back(image.width(), image.height());
        vec.emplace_back(ret[c], filters[c].first, filters[c].second);
    }
    compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(),
        vec.data(), vec.size()
    );
    return ret;
}



PackedBinaryMatrix compress_rgb32_to_binary_multirange(
    const ImageViewRGB32& image,
    const std::vector<std::pair<uint32_t, uint32_t>>& filters
){
    if (filters.empty()){
        PackedBinaryMatrix ret(image.width(), image.height());
        ret.set_zero();
        return ret;
    }
    PackedBinaryMatrix ret = compress_rgb32_to_binary_range(image, filters[0].first, filters[0].second);
    for (size_t c = 1; c < filters.size(); c++){
        ret |= compress_rgb32_to_binary_range(image, filters[c].first, filters[c].second);
    }
    return ret;
}







PackedBinaryMatrix compress_rgb32_to_binary_euclidean(
    const ImageViewRGB32& image,
    uint32_t expected, double max_euclidean_distance
){
    PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_euclidean(
        image.data(), image.bytes_per_row(),
        ret, expected, max_euclidean_distance
    );
    return ret;
}










}
