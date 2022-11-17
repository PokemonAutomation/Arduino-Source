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
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"

namespace PokemonAutomation{



void filter_by_mask(
    const PackedBinaryMatrix& matrix,
    ImageRGB32& image,
    Color replace_with,
    bool replace_if_zero    //  If false, replace if one.
);





PackedBinaryMatrix compress_rgb32_to_binary_min(
    const ImageViewRGB32& image,
    uint8_t min_red,
    uint8_t min_green,
    uint8_t min_blue
);
PackedBinaryMatrix compress_rgb32_to_binary_max(
    const ImageViewRGB32& image,
    uint8_t max_red,
    uint8_t max_green,
    uint8_t max_blue
);
PackedBinaryMatrix compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
PackedBinaryMatrix compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
PackedBinaryMatrix compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    uint32_t mins, uint32_t maxs
);



//  Run multiple filters at once. This is more memory efficient than making
//  multiple calls to one filter at a time.
//  Pixels Within filter color ranges are marked as 1 in the corresponding binary matrices,
//  while those output of range are 0.
std::vector<PackedBinaryMatrix> compress_rgb32_to_binary_range(
    const ImageViewRGB32& image,
    const std::vector<std::pair<uint32_t, uint32_t>>& filters
);



//  Run multiple filters and OR them all together. (experimental)
PackedBinaryMatrix compress_rgb32_to_binary_multirange(
    const ImageViewRGB32& image,
    const std::vector<std::pair<uint32_t, uint32_t>>& filters
);





PackedBinaryMatrix compress_rgb32_to_binary_euclidean(
    const ImageViewRGB32& image,
    uint32_t expected, double max_euclidean_distance
);




}
#endif
