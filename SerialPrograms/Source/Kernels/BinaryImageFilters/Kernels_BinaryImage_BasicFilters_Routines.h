/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_Routines_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_Routines_H

#include <stddef.h>
#include <stdint.h>
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels_BinaryImage_BasicFilters.h"

namespace PokemonAutomation{
namespace Kernels{



template <typename BinaryMatrixType, typename Filter>
void filter_by_mask(
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



template <typename BinaryMatrixType, typename Compressor>
void compress_rgb32_to_binary(
    const uint32_t* image, size_t bytes_per_row,
    BinaryMatrixType& matrix, const Compressor& compressor
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
struct CompressRgb32ToBinaryRangeEntry{
    BinaryMatrixType& matrix;
    Compressor compressor;

    CompressRgb32ToBinaryRangeEntry(BinaryMatrixType& p_matrix, uint32_t mins, uint32_t maxs)
        : matrix(p_matrix)
        , compressor(mins, maxs)
    {}
};
template <typename BinaryMatrixType, typename Compressor>
void compress_rgb32_to_binary(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filter, size_t filter_count
){
    using Entry = CompressRgb32ToBinaryRangeEntry<BinaryMatrixType, Compressor>;
    FixedLimitVector<Entry> entries(filter_count);
    for (size_t c = 0; c < filter_count; c++){
        entries.emplace_back(static_cast<BinaryMatrixType&>(filter[c].matrix), filter[c].mins, filter[c].maxs);
    }

    size_t bit_width = entries[0].matrix.get().width();
    size_t word_height = entries[0].matrix.get().word64_height();
    for (size_t r = 0; r < word_height; r++){
        const uint32_t* img = image;
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            for (Entry& entry : entries){
                entry.matrix.get().word64(c, r) = entry.compressor.convert64(img);
            }
            c++;
            img += 64;
            left -= 64;
        }
        if (left > 0){
            for (Entry& entry : entries){
                entry.matrix.get().word64(c, r) = entry.compressor.convert64(img, left);
            }
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }
}


// Change pixel (as uint32_t) color of image based on bits in a binary matrix
// If `filter` is constructed with `replace_if_zero` being true, image pixels corresponding to 0-bits in `matrix`
//    are replaced with color `replace_with` which is provided by the filter.
// If `filter` is constructed with `replace_if_zero` being false, image pixels corresponding to 1-bits in `matrix`
//    are replaced with color `replace_with`.
// `BinaryMatrixType` is a PackedBinaryMatrixCore<BinaryMatrixTileType>,
// where BinaryMatrixTileType is an implementation of a tile, defined for every simd architecture.
template <typename BinaryMatrixType, typename Filter>
void filter_rgb32(
    const BinaryMatrixType& matrix,
    uint32_t* image, size_t bytes_per_row,
    const Filter& filter
){
    size_t bit_width = matrix.width();
//    size_t bit_height = binary_image.height();
//    size_t word_width = binary_image.word64_width();

    // How many words in a row. Each word is 64-bit long
    size_t word_height = matrix.word64_height();
    for (size_t r = 0; r < word_height; r++){ // For each row
        uint32_t* img = image;
        // c: current index of the word. One word has 64-bit wide
        size_t c = 0;
        size_t left = bit_width;
        while (left >= 64){
            // Modify some pixels in the 64-pixel-long area of the image,
            // starting at pointr `img`
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
