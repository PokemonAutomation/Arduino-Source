/*  Binary Image Basic Filters (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64xH_Default.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters_Default.h"

namespace PokemonAutomation{
namespace Kernels{



void filter_by_mask_64x4_Default(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with, bool replace_if_zero
){
    FilterByMask_Default filter(replace_with, replace_if_zero);
    filter_by_mask(static_cast<const PackedBinaryMatrix_64x4_Default&>(matrix).get(), image, bytes_per_row, filter);
}



void compress_rgb32_to_binary_range_64x4_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix, uint32_t mins, uint32_t maxs
){
    Compressor_RgbRange_Default compressor(mins, maxs);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x4_Default&>(matrix).get(), compressor
    );
}
void compress_rgb32_to_binary_range_64x4_Default(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
){
    compress_rgb32_to_binary<PackedBinaryMatrix_64x4_Default, Compressor_RgbRange_Default>(
        image, bytes_per_row, filters, filter_count
    );
}



void compress_rgb32_to_binary_euclidean_64x4_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
){
    Compressor_RgbEuclidean_Default compressor(expected, max_euclidean_distance);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x4_Default&>(matrix).get(), compressor
    );
}



}
}
