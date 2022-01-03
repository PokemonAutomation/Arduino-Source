/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "BinaryImage_FilterRgb32.h"

namespace PokemonAutomation{


Kernels::PackedBinaryMatrix filter_rgb32_min(
    const QImage& image,
    uint8_t min_red,
    uint8_t min_green,
    uint8_t min_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::filter_rgb32_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        0, 255,
        min_red, 255,
        min_green, 255,
        min_blue, 255
    );
    return ret;
}
Kernels::PackedBinaryMatrix filter_rgb32_max(
    const QImage& image,
    uint8_t max_red,
    uint8_t max_green,
    uint8_t max_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::filter_rgb32_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        0, 255,
        0, max_red,
        0, max_green,
        0, max_blue
    );
    return ret;
}
Kernels::PackedBinaryMatrix filter_rgb32_range(
    const QImage& image,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::filter_rgb32_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        0, 255,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
    return ret;
}
Kernels::PackedBinaryMatrix filter_rgb32_range(
    const QImage& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::filter_rgb32_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
    return ret;
}





}
