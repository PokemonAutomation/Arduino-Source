/*  Binary Image Filter RGB32
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_BinaryImage_FilterRgb32_H
#define PokemonAutomation_CommonFramework_BinaryImage_FilterRgb32_H

#include <stdint.h>
#include "Common/Cpp/Color.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "CommonFramework/ImageTypes/ImageReference.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"

class QImage;

namespace PokemonAutomation{



PackedBinaryMatrix2 compress_rgb32_to_binary_min(
    const QImage& image,
    uint8_t min_red,
    uint8_t min_green,
    uint8_t min_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_max(
    const QImage& image,
    uint8_t max_red,
    uint8_t max_green,
    uint8_t max_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const QImage& image,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const QImage& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const QImage& image,
    uint32_t mins, uint32_t maxs
);

void compress2_rgb32_to_binary_range(
    const QImage& image,
    PackedBinaryMatrix2& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix2& matrix1, uint32_t mins1, uint32_t maxs1
);
void compress4_rgb32_to_binary_range(
    const QImage& image,
    PackedBinaryMatrix2& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix2& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix2& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix2& matrix3, uint32_t mins3, uint32_t maxs3
);


void filter_rgb32(
    const PackedBinaryMatrix2& matrix,
    const ImageRef& image,
    Color replace_with,
    bool replace_if_zero    //  If false, replace if one.
);








}
#endif
