/*  Binary Image Basic Filters (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_08_Nehalem

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x8_x64_SSE42.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters_64x8_x64_SSE42.h"

namespace PokemonAutomation{
namespace Kernels{



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
    CompressRgb32ToBinaryRangeFilter* filter, size_t filter_count
){
    compress_rgb32_to_binary<PackedBinaryMatrix_64x8_x64_SSE42, Compressor_RgbRange_x64_SSE41>(
        image, bytes_per_row, filter, filter_count
    );
}



void filter_rgb32_64x8_x64_SSE42(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with, bool replace_if_zero
){
    Filter_RgbRange_x64_SSE41 filter(replace_with, replace_if_zero);
    filter_rgb32(static_cast<const PackedBinaryMatrix_64x8_x64_SSE42&>(matrix).get(), image, bytes_per_row, filter);
}




}
}
#endif
