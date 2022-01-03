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
#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{


#if 0
template <typename BinaryImageType, typename Filter>
void rgb32_to_binary_image(
    BinaryImageType& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    const Filter& filter
){
    size_t width = binary_image.width();
    for (size_t r = 0; r < binary_image.height(); r++){
        const uint32_t* img = image;
        uint64_t* bin = (uint64_t*)binary_image.row(r);
        size_t left = width;
        while (left >= 64){
            *bin = filter.convert64(img);
            bin++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            *bin = filter.convert64(img, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }
}
void filter_min_rgb32(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
);
void filter_rgb32_range(
    BinaryImage& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);
#endif

template <typename BinaryMatrixType, typename Filter>
void rgb32_to_binary_image2(
    BinaryMatrixType& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    const Filter& filter
){
    size_t bit_width = binary_image.width();
//    size_t bit_height = binary_image.height();
//    size_t word_width = binary_image.word64_width();
    size_t word_height = binary_image.word64_height();
    for (size_t r = 0; r < word_height; r++){
        const uint32_t* img = image;
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            binary_image.word64(c, r) = filter.convert64(img);
            c++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            binary_image.word64(c, r) = filter.convert64(img, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }
}


void filter_rgb32_range(
    PackedBinaryMatrix& binary_image,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
);


}
}
#endif
