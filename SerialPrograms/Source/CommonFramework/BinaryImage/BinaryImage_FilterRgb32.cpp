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


Kernels::PackedBinaryMatrix compress_rgb32_to_binary_min(
    const QImage& image,
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
Kernels::PackedBinaryMatrix compress_rgb32_to_binary_max(
    const QImage& image,
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
Kernels::PackedBinaryMatrix compress_rgb32_to_binary_range(
    const QImage& image,
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
        ((uint32_t)min_alpha << 24) | ((uint32_t)min_red << 16) | ((uint32_t)min_green << 8) | (uint32_t)min_blue,
        ((uint32_t)max_alpha << 24) | ((uint32_t)max_red << 16) | ((uint32_t)max_green << 8) | (uint32_t)max_blue
    );
    return ret;
}
Kernels::PackedBinaryMatrix compress_rgb32_to_binary_range(
    const QImage& image,
    uint32_t mins, uint32_t maxs
){
    Kernels::PackedBinaryMatrix ret(image.width(), image.height());
    Kernels::compress_rgb32_to_binary_range(
        ret, (const uint32_t*)image.bits(), image.bytesPerLine(),
        mins, maxs
    );
    return ret;
}

void compress2_rgb32_to_binary_range(
    const QImage& image,
    Kernels::PackedBinaryMatrix& matrix0, uint32_t mins0, uint32_t maxs0,
    Kernels::PackedBinaryMatrix& matrix1, uint32_t mins1, uint32_t maxs1
){
    matrix0 = Kernels::PackedBinaryMatrix(image.width(), image.height());
    matrix1 = Kernels::PackedBinaryMatrix(image.width(), image.height());
    Kernels::compress2_rgb32_to_binary_range(
        (const uint32_t*)image.bits(), image.bytesPerLine(),
        matrix0, mins0, maxs0,
        matrix1, mins1, maxs1
    );
}
void compress4_rgb32_to_binary_range(
    const QImage& image,
    Kernels::PackedBinaryMatrix& matrix0, uint32_t mins0, uint32_t maxs0,
    Kernels::PackedBinaryMatrix& matrix1, uint32_t mins1, uint32_t maxs1,
    Kernels::PackedBinaryMatrix& matrix2, uint32_t mins2, uint32_t maxs2,
    Kernels::PackedBinaryMatrix& matrix3, uint32_t mins3, uint32_t maxs3
){
    matrix0 = Kernels::PackedBinaryMatrix(image.width(), image.height());
    matrix1 = Kernels::PackedBinaryMatrix(image.width(), image.height());
    matrix2 = Kernels::PackedBinaryMatrix(image.width(), image.height());
    matrix3 = Kernels::PackedBinaryMatrix(image.width(), image.height());
    Kernels::compress4_rgb32_to_binary_range(
        (const uint32_t*)image.bits(), image.bytesPerLine(),
        matrix0, mins0, maxs0,
        matrix1, mins1, maxs1,
        matrix2, mins2, maxs2,
        matrix3, mins3, maxs3
    );
}



void filter_rgb32(
    const Kernels::PackedBinaryMatrix& matrix,
    QImage& image,
    Color replace_with,
    bool replace_if_zero
){
    if ((int)matrix.width() > image.width()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image width is too small.");
    }
    if ((int)matrix.height() > image.height()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image height is too small.");
    }
    Kernels::filter_rgb32(
        matrix,
        (uint32_t*)image.bits(), image.bytesPerLine(),
        (uint32_t)replace_with, replace_if_zero
    );
}




}
