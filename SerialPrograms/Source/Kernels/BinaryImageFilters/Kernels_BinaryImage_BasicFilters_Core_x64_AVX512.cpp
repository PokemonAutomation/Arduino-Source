/*  Binary Image Basic Filters (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_17_Skylake

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_x64_AVX512.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{



void compress_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
){
    Compressor_RgbRange_x64_AVX512 compressor0(mins0, maxs0);
    compress_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix0).get(), compressor0
    );
}
void compress2_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1
){
    Compressor_RgbRange_x64_AVX512 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_AVX512 compressor1(mins1, maxs1);
    compress2_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix0).get(), compressor0,
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix1).get(), compressor1
    );
}
void compress4_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix_IB& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix_IB& matrix3, uint32_t mins3, uint32_t maxs3
){
    Compressor_RgbRange_x64_AVX512 compressor0(mins0, maxs0);
    Compressor_RgbRange_x64_AVX512 compressor1(mins1, maxs1);
    Compressor_RgbRange_x64_AVX512 compressor2(mins2, maxs2);
    Compressor_RgbRange_x64_AVX512 compressor3(mins3, maxs3);
    compress4_rgb32_to_binary(
        image, bytes_per_row,
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix0).get(), compressor0,
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix1).get(), compressor1,
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix2).get(), compressor2,
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix3).get(), compressor3
    );
}



void filter_rgb32_x64_AVX512(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with, bool replace_if_zero
){
    Filter_RgbRange_x64_AVX512 filter(replace_with, replace_if_zero);
    filter_rgb32(static_cast<const PackedBinaryMatrix_x64_AVX512&>(matrix).get(), image, bytes_per_row, filter);
}





}
}
#endif
