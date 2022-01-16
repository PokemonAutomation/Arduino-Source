/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exception.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "BinaryImage_FilterRgb32.h"

namespace PokemonAutomation{


Kernels::PackedBinaryMatrix compress_rgb32_to_binary_min(
    const QImage& image,
    uint8_t min_red,
    uint8_t min_green,
    uint8_t min_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        255, 255,
        min_red, 255,
        min_green, 255,
        min_blue, 255
    );
    return ret;
}
Kernels::PackedBinaryMatrix compress_rgb32_to_binary_max(
    const QImage& image,
    uint8_t max_red,
    uint8_t max_green,
    uint8_t max_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        255, 255,
        0, max_red,
        0, max_green,
        0, max_blue
    );
    return ret;
}
Kernels::PackedBinaryMatrix compress_rgb32_to_binary_range(
    const QImage& image,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        255, 255,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
    return ret;
}
Kernels::PackedBinaryMatrix compress_rgb32_to_binary_range(
    const QImage& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
    return ret;
}

void filter_rgb32(
    const Kernels::PackedBinaryMatrix& matrix,
    QImage& image,
    Color replace_with,
    bool replace_if_zero
){
    if ((int)matrix.width() > image.width()){
        PA_THROW_StringException("Image width is too small.");
    }
    if ((int)matrix.height() > image.height()){
        PA_THROW_StringException("Image height is too small.");
    }
    Kernels::filter_rgb32(
        matrix,
        (uint32_t*)image.bits(), image.bytesPerLine(),
        (uint32_t)replace_with, replace_if_zero
    );
}




}
