/*  Binary Image Basic Filters (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x8_arm64_NEON.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters_arm64_NEON.h"


namespace PokemonAutomation{
namespace Kernels{


void filter_by_mask_64x8_arm64_NEON(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replacement_color, bool replace_zero_bits
){
    FilterByMask_arm64_NEON filter(replacement_color, replace_zero_bits);
    filter_by_mask(static_cast<const PackedBinaryMatrix_64x8_arm64_NEON&>(matrix).get(), image, bytes_per_row, filter);
}




void compress_rgb32_to_binary_range_64x8_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
){
    Compressor_RgbRange_arm64_NEON compressor0(mins0, maxs0);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x8_arm64_NEON&>(matrix0).get(), compressor0
    );
}
void compress_rgb32_to_binary_range_64x8_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
){
    compress_rgb32_to_binary<PackedBinaryMatrix_64x8_arm64_NEON, Compressor_RgbRange_arm64_NEON>(
        image, bytes_per_row, filters, filter_count
    );
}


void compress_rgb32_to_binary_euclidean_64x8_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
){
    Compressor_RgbEuclidean_arm64_NEON compressor(expected, max_euclidean_distance);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x8_arm64_NEON&>(matrix).get(), compressor
    );
}



}
}
#endif
