/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "BinaryImage_FilterRgb32.h"

namespace PokemonAutomation{



PackedBinaryMatrix2 compress_rgb32_to_binary_min(
    const ConstImageRef& image,
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
PackedBinaryMatrix2 compress_rgb32_to_binary_max(
    const ConstImageRef& image,
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
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const ConstImageRef& image,
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
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const ConstImageRef& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    PackedBinaryMatrix2 ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(), ret,
        ((uint32_t)min_alpha << 24) | ((uint32_t)min_red << 16) | ((uint32_t)min_green << 8) | (uint32_t)min_blue,
        ((uint32_t)max_alpha << 24) | ((uint32_t)max_red << 16) | ((uint32_t)max_green << 8) | (uint32_t)max_blue
    );
    return ret;
}
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const ConstImageRef& image,
    uint32_t mins, uint32_t maxs
){
    PackedBinaryMatrix2 ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(),
        ret, mins, maxs
    );
    return ret;
}

void compress2_rgb32_to_binary_range(
    const ConstImageRef& image,
    PackedBinaryMatrix2& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix2& matrix1, uint32_t mins1, uint32_t maxs1
){
    matrix0 = PackedBinaryMatrix2(image.width(), image.height());
    matrix1 = PackedBinaryMatrix2(image.width(), image.height());
    Kernels::compress2_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(),
        matrix0, mins0, maxs0,
        matrix1, mins1, maxs1
    );
}
void compress4_rgb32_to_binary_range(
    const ConstImageRef& image,
    PackedBinaryMatrix2& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix2& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix2& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix2& matrix3, uint32_t mins3, uint32_t maxs3
){
    matrix0 = PackedBinaryMatrix2(image.width(), image.height());
    matrix1 = PackedBinaryMatrix2(image.width(), image.height());
    matrix2 = PackedBinaryMatrix2(image.width(), image.height());
    matrix3 = PackedBinaryMatrix2(image.width(), image.height());
    Kernels::compress4_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(),
        matrix0, mins0, maxs0,
        matrix1, mins1, maxs1,
        matrix2, mins2, maxs2,
        matrix3, mins3, maxs3
    );
}



void filter_rgb32(
    const PackedBinaryMatrix2& matrix,
    const ImageRef& image,
    Color replace_with,
    bool replace_if_zero
){
    if (matrix.width() > image.width()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image width is too small.");
    }
    if (matrix.height() > image.height()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image height is too small.");
    }
    Kernels::filter_rgb32(
        matrix,
        image.data(), image.bytes_per_row(),
        (uint32_t)replace_with, replace_if_zero
    );
}















}
