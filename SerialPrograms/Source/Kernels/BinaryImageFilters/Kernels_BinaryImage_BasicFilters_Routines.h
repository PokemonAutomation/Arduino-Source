/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_Routines_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_Routines_H

#include <stddef.h>
#include <stdint.h>

namespace PokemonAutomation{
namespace Kernels{



template <typename BinaryMatrixType, typename Compressor>
void compress_rgb32_to_binary(
    const uint32_t* image, size_t bytes_per_row,
    BinaryMatrixType& matrix0, const Compressor& compressor0
){
    size_t bit_width = matrix0.width();
//    size_t bit_height = binary_image.height();
//    size_t word_width = binary_image.word64_width();
    size_t word_height = matrix0.word64_height();
    for (size_t r = 0; r < word_height; r++){
        const uint32_t* img = image;
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            matrix0.word64(c, r) = compressor0.convert64(img);
            c++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            matrix0.word64(c, r) = compressor0.convert64(img, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }
}
template <typename BinaryMatrixType, typename Compressor>
void compress2_rgb32_to_binary(
    const uint32_t* image, size_t bytes_per_row,
    BinaryMatrixType& matrix0, const Compressor& compressor0,
    BinaryMatrixType& matrix1, const Compressor& compressor1
){
    size_t bit_width = matrix0.width();
//    size_t bit_height = binary_image.height();
//    size_t word_width = binary_image.word64_width();
    size_t word_height = matrix0.word64_height();
    for (size_t r = 0; r < word_height; r++){
        const uint32_t* img = image;
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            matrix0.word64(c, r) = compressor0.convert64(img);
            matrix1.word64(c, r) = compressor1.convert64(img);
            c++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            matrix0.word64(c, r) = compressor0.convert64(img, left);
            matrix1.word64(c, r) = compressor1.convert64(img, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }
}
template <typename BinaryMatrixType, typename Compressor>
void compress4_rgb32_to_binary(
    const uint32_t* image, size_t bytes_per_row,
    BinaryMatrixType& matrix0, const Compressor& compressor0,
    BinaryMatrixType& matrix1, const Compressor& compressor1,
    BinaryMatrixType& matrix2, const Compressor& compressor2,
    BinaryMatrixType& matrix3, const Compressor& compressor3
){
    size_t bit_width = matrix0.width();
//    size_t bit_height = binary_image.height();
//    size_t word_width = binary_image.word64_width();
    size_t word_height = matrix0.word64_height();
    for (size_t r = 0; r < word_height; r++){
        const uint32_t* img = image;
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            matrix0.word64(c, r) = compressor0.convert64(img);
            matrix1.word64(c, r) = compressor1.convert64(img);
            matrix2.word64(c, r) = compressor2.convert64(img);
            matrix3.word64(c, r) = compressor3.convert64(img);
            c++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            matrix0.word64(c, r) = compressor0.convert64(img, left);
            matrix1.word64(c, r) = compressor1.convert64(img, left);
            matrix2.word64(c, r) = compressor2.convert64(img, left);
            matrix3.word64(c, r) = compressor3.convert64(img, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }
}


template <typename BinaryMatrixType, typename Filter>
void filter_rgb32(
    const BinaryMatrixType& matrix,
    uint32_t* image, size_t bytes_per_row,
    const Filter& filter
){
    size_t bit_width = matrix.width();
//    size_t bit_height = binary_image.height();
//    size_t word_width = binary_image.word64_width();
    size_t word_height = matrix.word64_height();
    for (size_t r = 0; r < word_height; r++){
        uint32_t* img = image;
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            filter.filter64(matrix.word64(c, r), img);
            c++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            filter.filter64(matrix.word64(c, r), img, left);
        }
        image = (uint32_t*)((const char*)image + bytes_per_row);
    }
}




}
}
#endif
