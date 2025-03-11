/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/
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



void filter_by_mask_64x4_Default      (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x8_x64_SSE42    (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x16_x64_AVX2    (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x32_x64_AVX512  (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x64_x64_AVX512  (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x8_arm64_NEON    (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);

void filter_by_mask(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with,
    bool replace_if_zero    //  If false, replace if one.
){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        filter_by_mask_64x64_x64_AVX512(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
    case BinaryMatrixType::i64x32_x64_AVX512:
        filter_by_mask_64x32_x64_AVX512(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        filter_by_mask_64x16_x64_AVX2(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        filter_by_mask_64x8_x64_SSE42(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        filter_by_mask_64x8_arm64_NEON(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
    case BinaryMatrixType::i64x4_Default:
        filter_by_mask_64x4_Default(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format in filter_by_mask().");
    }
}



void compress_rgb32_to_binary_range_64x4_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
);
void compress_rgb32_to_binary_range_64x4_Default(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);

void compress_rgb32_to_binary_range_64x8_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
);
void compress_rgb32_to_binary_range_64x8_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);

void compress_rgb32_to_binary_range_64x16_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
);
void compress_rgb32_to_binary_range_64x16_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);

void compress_rgb32_to_binary_range_64x32_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
);
void compress_rgb32_to_binary_range_64x32_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);

void compress_rgb32_to_binary_range_64x64_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
);
void compress_rgb32_to_binary_range_64x64_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);

void compress_rgb32_to_binary_range_64x8_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress_rgb32_to_binary_range_64x8_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);

void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        compress_rgb32_to_binary_range_64x64_x64_AVX512(image, bytes_per_row, matrix, mins, maxs);
        return;
    case BinaryMatrixType::i64x32_x64_AVX512:
        compress_rgb32_to_binary_range_64x32_x64_AVX512(image, bytes_per_row, matrix, mins, maxs);
        return;
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        compress_rgb32_to_binary_range_64x16_x64_AVX2(image, bytes_per_row, matrix, mins, maxs);
        return;
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        compress_rgb32_to_binary_range_64x8_x64_SSE42(image, bytes_per_row, matrix, mins, maxs);
        return;
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        compress_rgb32_to_binary_range_64x8_arm64_NEON(image, bytes_per_row, matrix, mins, maxs);
        return;
#endif
    case BinaryMatrixType::i64x4_Default:
        compress_rgb32_to_binary_range_64x4_Default(image, bytes_per_row, matrix, mins, maxs);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}
void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
){
    if (filter_count == 0){
        return;
    }
    BinaryMatrixType type = filters[0].matrix.type();
    for (size_t c = 1; c < filter_count; c++){
        if (type != filters[c].matrix.type()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching matrix formats.");
        }
    }
    switch (type){
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        compress_rgb32_to_binary_range_64x64_x64_AVX512(image, bytes_per_row, filters, filter_count);
        return;
    case BinaryMatrixType::i64x32_x64_AVX512:
        compress_rgb32_to_binary_range_64x32_x64_AVX512(image, bytes_per_row, filters, filter_count);
        return;
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        compress_rgb32_to_binary_range_64x16_x64_AVX2(image, bytes_per_row, filters, filter_count);
        return;
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        compress_rgb32_to_binary_range_64x8_x64_SSE42(image, bytes_per_row, filters, filter_count);
        return;
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        compress_rgb32_to_binary_range_64x8_arm64_NEON(image, bytes_per_row, filters, filter_count);
        return;
#endif
    case BinaryMatrixType::i64x4_Default:
        compress_rgb32_to_binary_range_64x4_Default(image, bytes_per_row, filters, filter_count);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}


void compress_rgb32_to_binary_euclidean_64x64_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
);
void compress_rgb32_to_binary_euclidean_64x32_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
);
void compress_rgb32_to_binary_euclidean_64x16_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
);
void compress_rgb32_to_binary_euclidean_64x8_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
);
void compress_rgb32_to_binary_euclidean_64x8_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
);
void compress_rgb32_to_binary_euclidean_64x4_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
);
void compress_rgb32_to_binary_euclidean(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        compress_rgb32_to_binary_euclidean_64x64_x64_AVX512(image, bytes_per_row, matrix, expected, max_euclidean_distance);
        return;
    case BinaryMatrixType::i64x32_x64_AVX512:
        compress_rgb32_to_binary_euclidean_64x32_x64_AVX512(image, bytes_per_row, matrix, expected, max_euclidean_distance);
        return;
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        compress_rgb32_to_binary_euclidean_64x16_x64_AVX2(image, bytes_per_row, matrix, expected, max_euclidean_distance);
        return;
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        compress_rgb32_to_binary_euclidean_64x8_x64_SSE42(image, bytes_per_row, matrix, expected, max_euclidean_distance);
        return;
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        compress_rgb32_to_binary_euclidean_64x8_arm64_NEON(image, bytes_per_row, matrix, expected, max_euclidean_distance);
        return;
#endif
    case BinaryMatrixType::i64x4_Default:
        compress_rgb32_to_binary_euclidean_64x4_Default(image, bytes_per_row, matrix, expected, max_euclidean_distance);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}








}
}
