/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{


void compress_rgb32_to_binary_range_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress_rgb32_to_binary_range_Default(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filter, size_t filter_count
);

void compress_rgb32_to_binary_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress_rgb32_to_binary_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filter, size_t filter_count
);

void compress_rgb32_to_binary_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress_rgb32_to_binary_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filter, size_t filter_count
);

void compress_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filter, size_t filter_count
);

void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
){
    switch (matrix0.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::i64x64_AVX512:
        compress_rgb32_to_binary_range_x64_AVX512(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::i64x16_AVX2:
        compress_rgb32_to_binary_range_x64_AVX2(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::i64x8_SSE42:
        compress_rgb32_to_binary_range_x64_SSE42(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
#endif
    case BinaryMatrixType::i64x4_Default:
        compress_rgb32_to_binary_range_Default(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}
void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filter, size_t filter_count
){
    if (filter_count == 0){
        return;
    }
    BinaryMatrixType type = filter[0].matrix.type();
    for (size_t c = 1; c < filter_count; c++){
        if (type != filter[c].matrix.type()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching matrix formats.");
        }
    }
    switch (type){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::i64x64_AVX512:
        compress_rgb32_to_binary_range_x64_AVX512(image, bytes_per_row, filter, filter_count);
        return;
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::i64x16_AVX2:
        compress_rgb32_to_binary_range_x64_AVX2(image, bytes_per_row, filter, filter_count);
        return;
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::i64x8_SSE42:
        compress_rgb32_to_binary_range_x64_SSE42(image, bytes_per_row, filter, filter_count);
        return;
#endif
    case BinaryMatrixType::i64x4_Default:
        compress_rgb32_to_binary_range_Default(image, bytes_per_row, filter, filter_count);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}






void filter_rgb32_Default       (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_rgb32_x64_SSE42     (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_rgb32_x64_AVX2      (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_rgb32_x64_AVX512    (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);

void filter_rgb32(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with,
    bool replace_if_zero    //  If false, replace if one.
){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::i64x64_AVX512:
        filter_rgb32_x64_AVX512(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::i64x16_AVX2:
        filter_rgb32_x64_AVX2(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::i64x8_SSE42:
        filter_rgb32_x64_SSE42(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
    case BinaryMatrixType::i64x4_Default:
        filter_rgb32_Default(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}







}
}
