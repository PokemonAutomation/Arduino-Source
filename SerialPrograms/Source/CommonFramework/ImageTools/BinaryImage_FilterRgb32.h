/*  Binary Image Filter RGB32
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_BinaryImage_FilterRgb32_H
#define PokemonAutomation_CommonFramework_BinaryImage_FilterRgb32_H

#include <stdint.h>
#include <vector>
#include "Common/Cpp/Color.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "CommonFramework/ImageTypes/ImageReference.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"

class QImage;

namespace PokemonAutomation{



PackedBinaryMatrix2 compress_rgb32_to_binary_min(
    const ConstImageRef& image,
    uint8_t min_red,
    uint8_t min_green,
    uint8_t min_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_max(
    const ConstImageRef& image,
    uint8_t max_red,
    uint8_t max_green,
    uint8_t max_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const ConstImageRef& image,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const ConstImageRef& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
PackedBinaryMatrix2 compress_rgb32_to_binary_range(
    const ConstImageRef& image,
    uint32_t mins, uint32_t maxs
);
std::vector<PackedBinaryMatrix2> compress_rgb32_to_binary_range(
    const ConstImageRef& image,
    const std::vector<std::pair<uint32_t, uint32_t>>& filters
);


void filter_rgb32(
    const PackedBinaryMatrix2& matrix,
    const ImageRef& image,
    Color replace_with,
    bool replace_if_zero    //  If false, replace if one.
);








}
#endif
