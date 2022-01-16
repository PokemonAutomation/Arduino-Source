/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_BinaryImage_BasicFilters.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels_BinaryImage_BasicFilters_Default.h"
#ifdef PA_Arch_x64_SSE42
#include "Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
#endif
#ifdef PA_Arch_x64_AVX2
#include "Kernels_BinaryImage_BasicFilters_x64_AVX2.h"
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels_BinaryImage_BasicFilters_x64_AVX512.h"
#endif

namespace PokemonAutomation{
namespace Kernels{


template <typename BinaryMatrixType, typename Compressor>
void compress_rgb32_to_binary(
    BinaryMatrixType& matrix,
    const uint32_t* image, size_t bytes_per_row,
    const Compressor& compressor
){
    size_t bit_width = matrix.width();
//    size_t bit_height = binary_image.height();
//    size_t word_width = binary_image.word64_width();
    size_t word_height = matrix.word64_height();
    for (size_t r = 0; r < word_height; r++){
        const uint32_t* img = image;
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            matrix.word64(c, r) = compressor.convert64(img);
            c++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            matrix.word64(c, r) = compressor.convert64(img, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }
}
void compress_rgb32_to_binary_range(
    PackedBinaryMatrix& matrix,
    const uint32_t* image, size_t bytes_per_row,
    uint8_t min_alpha, uint8_t max_alpha,
    uint8_t min_red, uint8_t max_red,
    uint8_t min_green, uint8_t max_green,
    uint8_t min_blue, uint8_t max_blue
){
#if 0
#elif defined PA_Arch_x64_AVX512
    Compressor_RgbRange_x64_AVX512 compressor(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#elif defined PA_Arch_x64_AVX2
    Compressor_RgbRange_x64_AVX2 compressor(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#elif defined PA_Arch_x64_SSE42
    Compressor_RgbRange_x64_SSE41 compressor(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#else
    Compressor_RgbRange_Default compressor(
        min_alpha, max_alpha,
        min_red, max_red,
        min_green, max_green,
        min_blue, max_blue
    );
#endif
    compress_rgb32_to_binary(matrix, image, bytes_per_row, compressor);
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
void filter_rgb32(
    const PackedBinaryMatrix& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with,
    bool replace_if_zero    //  If false, replace if one.
){
#if 0
#elif defined PA_Arch_x64_AVX512
    Filter_x64_AVX512 filter(replace_with, replace_if_zero);
#elif defined PA_Arch_x64_AVX2
    Filter_x64_AVX2 filter(replace_with, replace_if_zero);
#elif defined PA_Arch_x64_SSE42
    Filter_x64_SSE41 filter(replace_with, replace_if_zero);
#else
    Filter_Default filter(replace_with, replace_if_zero);
#endif
    filter_rgb32(matrix, image, bytes_per_row, filter);
}





}
}
