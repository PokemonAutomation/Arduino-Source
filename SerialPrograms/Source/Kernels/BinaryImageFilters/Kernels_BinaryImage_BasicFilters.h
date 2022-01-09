/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Perform a filter over an image and store the results in a binary image.
 *
 *  The parameter "binary_image" specifies the dimensions of the image.
 *  The parameter itself is overwritten by result.
 *
 *  Example Usage:
 *
 *      BinaryImage binary_image(1280, 720);
 *
 *      filter_min_rgb32(
 *          binary_image,
 *          image, bytes_per_row,
 *          0xff, 0x80, 0x80, 0x80
 *      );
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_H

//#include "Kernels/BinaryImage/Kernels_BinaryImage.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{


//  Compress (image, bytes_per_row) into a binary_image.
//  Use the specified RGB ranges to determine whether each pixel
//  becomes a 0 or a 1.
void compress_rgb32_to_binary_range(
    PackedBinaryMatrix& matrix,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);



//  Selectively replace each pixel in an rgb32 image with the specified pixel
//  according to the respective bit in the binary matrix.
void filter_rgb32(
    const PackedBinaryMatrix& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with,
    bool replace_if_zero    //  If false, replace if one.
);




}
}
#endif
