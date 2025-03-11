/*  Binary Image Basic Filters (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x8_x64_SSE42.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters_x64_SSE42.h"

namespace PokemonAutomation{
namespace Kernels{



void filter_by_mask_64x8_x64_SSE42(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with, bool replace_if_zero
){
    FilterByMask_x64_SSE41 filter(replace_with, replace_if_zero);
    filter_by_mask(static_cast<const PackedBinaryMatrix_64x8_x64_SSE42&>(matrix).get(), image, bytes_per_row, filter);
}



void compress_rgb32_to_binary_range_64x8_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
){
    Compressor_RgbRange_x64_SSE41 compressor0(mins0, maxs0);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x8_x64_SSE42&>(matrix0).get(), compressor0
    );
}
void compress_rgb32_to_binary_range_64x8_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
){
    compress_rgb32_to_binary<PackedBinaryMatrix_64x8_x64_SSE42, Compressor_RgbRange_x64_SSE41>(
        image, bytes_per_row, filters, filter_count
    );
}



void compress_rgb32_to_binary_euclidean_64x8_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
){
    Compressor_RgbEuclidean_x64_SSE41 compressor(expected, max_euclidean_distance);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x8_x64_SSE42&>(matrix).get(), compressor
    );
}




}
}
#endif
