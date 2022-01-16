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

void compress_rgb32_to_binary_range(
    PackedBinaryMatrix& matrix,
    const uint32_t* image, size_t bytes_per_row,
    uint32_t mins, uint32_t maxs
){
#if 0
#elif defined PA_Arch_x64_AVX512
    Compressor_RgbRange_x64_AVX512 compressor(mins, maxs);
#elif defined PA_Arch_x64_AVX2
    Compressor_RgbRange_x64_AVX2 compressor(mins, maxs);
#elif defined PA_Arch_x64_SSE42
    Compressor_RgbRange_x64_SSE41 compressor(mins, maxs);
#else
    Compressor_RgbRange_Default compressor(mins, maxs);
#endif
    compress_rgb32_to_binary(matrix, image, bytes_per_row, compressor);
}
void compress2_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix& matrix1, uint32_t mins1, uint32_t maxs1
){
#if 0
#elif defined PA_Arch_x64_AVX512
    Compressor_RgbRange_x64_AVX512 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_AVX512 compressor1(mins1, maxs1);
#elif defined PA_Arch_x64_AVX2
    Compressor_RgbRange_x64_AVX2 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_AVX2 compressor1(mins1, maxs1);
#elif defined PA_Arch_x64_SSE42
    Compressor_RgbRange_x64_SSE41 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_SSE41 compressor1(mins1, maxs1);
#else
    Compressor_RgbRange_Default compressor0(mins0, maxs0);
    Compressor_RgbRange_Default compressor1(mins1, maxs1);
#endif
    compress2_rgb32_to_binary(
        image, bytes_per_row,
        matrix0, compressor0,
        matrix1, compressor1
    );
}
void compress4_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix& matrix3, uint32_t mins3, uint32_t maxs3
){
#if 0
#elif defined PA_Arch_x64_AVX512
    Compressor_RgbRange_x64_AVX512 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_AVX512 compressor1(mins1, maxs1);
    Compressor_RgbRange_x64_AVX512 compressor2(mins2, maxs2);
    Compressor_RgbRange_x64_AVX512 compressor3(mins3, maxs3);
#elif defined PA_Arch_x64_AVX2
    Compressor_RgbRange_x64_AVX2 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_AVX2 compressor1(mins1, maxs1);
    Compressor_RgbRange_x64_AVX2 compressor2(mins2, maxs2);
    Compressor_RgbRange_x64_AVX2 compressor3(mins3, maxs3);
#elif defined PA_Arch_x64_SSE42
    Compressor_RgbRange_x64_SSE41 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_SSE41 compressor1(mins1, maxs1);
    Compressor_RgbRange_x64_SSE41 compressor2(mins2, maxs2);
    Compressor_RgbRange_x64_SSE41 compressor3(mins3, maxs3);
#else
    Compressor_RgbRange_Default compressor0(mins0, maxs0);
    Compressor_RgbRange_Default compressor1(mins1, maxs1);
    Compressor_RgbRange_Default compressor2(mins2, maxs2);
    Compressor_RgbRange_Default compressor3(mins3, maxs3);
#endif
    compress4_rgb32_to_binary(
        image, bytes_per_row,
        matrix0, compressor0,
        matrix1, compressor1,
        matrix2, compressor2,
        matrix3, compressor3
    );
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
