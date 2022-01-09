/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_BinaryImage_FilterRgb32_H
#define PokemonAutomation_CommonFramework_BinaryImage_FilterRgb32_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"

class QImage;

namespace PokemonAutomation{


Kernels::PackedBinaryMatrix filter_rgb32_min(
    const QImage& image,
    uint8_t min_red,
    uint8_t min_green,
    uint8_t min_blue
);
Kernels::PackedBinaryMatrix filter_rgb32_max(
    const QImage& image,
    uint8_t max_red,
    uint8_t max_green,
    uint8_t max_blue
);
Kernels::PackedBinaryMatrix filter_rgb32_range(
    const QImage& image,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
Kernels::PackedBinaryMatrix filter_rgb32_range(
    const QImage& image,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);


}
#endif
