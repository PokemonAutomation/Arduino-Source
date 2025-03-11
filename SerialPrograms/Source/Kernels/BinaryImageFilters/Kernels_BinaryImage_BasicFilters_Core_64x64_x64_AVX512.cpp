/*  Binary Image Basic Filters (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x64_x64_AVX512.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{



void filter_by_mask_64x64_x64_AVX512(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with, bool replace_if_zero
){
    FilterByMask_x64_AVX512 filter(replace_with, replace_if_zero);
    filter_by_mask(static_cast<const PackedBinaryMatrix_64x64_x64_AVX512&>(matrix).get(), image, bytes_per_row, filter);
}



void compress_rgb32_to_binary_range_64x64_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
){
    Compressor_RgbRange_x64_AVX512 compressor0(mins0, maxs0);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x64_x64_AVX512&>(matrix0).get(), compressor0
    );
}
void compress_rgb32_to_binary_range_64x64_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
){
    compress_rgb32_to_binary<PackedBinaryMatrix_64x64_x64_AVX512, Compressor_RgbRange_x64_AVX512>(
        image, bytes_per_row, filters, filter_count
    );
}



void compress_rgb32_to_binary_euclidean_64x64_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
){
    Compressor_RgbEuclidean_x64_AVX512 compressor(expected, max_euclidean_distance);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_64x64_x64_AVX512&>(matrix).get(), compressor
    );
}





}
}
#endif
