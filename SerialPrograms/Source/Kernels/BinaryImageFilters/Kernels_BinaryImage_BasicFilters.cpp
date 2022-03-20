/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels_BinaryImage_BasicFilters_Routines.h"
#include "Kernels_BinaryImage_BasicFilters.h"

namespace PokemonAutomation{
namespace Kernels{


void compress_rgb32_to_binary_range_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress2_rgb32_to_binary_range_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1
);
void compress4_rgb32_to_binary_range_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix_IB& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix_IB& matrix3, uint32_t mins3, uint32_t maxs3
);

void compress_rgb32_to_binary_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress2_rgb32_to_binary_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1
);
void compress4_rgb32_to_binary_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix_IB& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix_IB& matrix3, uint32_t mins3, uint32_t maxs3
);

void compress_rgb32_to_binary_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress2_rgb32_to_binary_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1
);
void compress4_rgb32_to_binary_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix_IB& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix_IB& matrix3, uint32_t mins3, uint32_t maxs3
);

void compress_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);
void compress2_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1
);
void compress4_rgb32_to_binary_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix_IB& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix_IB& matrix3, uint32_t mins3, uint32_t maxs3
);

void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
){
    switch (matrix0.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        compress_rgb32_to_binary_range_x64_AVX512(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        compress_rgb32_to_binary_range_x64_AVX2(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        compress_rgb32_to_binary_range_x64_SSE42(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
#endif
    case BinaryMatrixType::Default:
        compress_rgb32_to_binary_range_Default(image, bytes_per_row, matrix0, mins0, maxs0);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}
void compress2_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1
){
    switch (matrix0.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        compress2_rgb32_to_binary_range_x64_AVX512(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1
        );
        return;
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        compress2_rgb32_to_binary_range_x64_AVX2(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1
        );
        return;
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        compress2_rgb32_to_binary_range_x64_SSE42(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1
        );
        return;
#endif
    case BinaryMatrixType::Default:
        compress2_rgb32_to_binary_range_Default(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1
        );
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}
void compress4_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0,
    PackedBinaryMatrix_IB& matrix1, uint32_t mins1, uint32_t maxs1,
    PackedBinaryMatrix_IB& matrix2, uint32_t mins2, uint32_t maxs2,
    PackedBinaryMatrix_IB& matrix3, uint32_t mins3, uint32_t maxs3
){
    switch (matrix0.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        compress4_rgb32_to_binary_range_x64_AVX512(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1,
            matrix2, mins2, maxs2,
            matrix3, mins3, maxs3
        );
        return;
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        compress4_rgb32_to_binary_range_x64_AVX2(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1,
            matrix2, mins2, maxs2,
            matrix3, mins3, maxs3
        );
        return;
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        compress4_rgb32_to_binary_range_x64_SSE42(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1,
            matrix2, mins2, maxs2,
            matrix3, mins3, maxs3
        );
        return;
#endif
    case BinaryMatrixType::Default:
        compress4_rgb32_to_binary_range_Default(
            image, bytes_per_row,
            matrix0, mins0, maxs0,
            matrix1, mins1, maxs1,
            matrix2, mins2, maxs2,
            matrix3, mins3, maxs3
        );
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
    case BinaryMatrixType::AVX512:
        filter_rgb32_x64_AVX512(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        filter_rgb32_x64_AVX2(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        filter_rgb32_x64_SSE42(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
#endif
    case BinaryMatrixType::Default:
        filter_rgb32_Default(matrix, image, bytes_per_row, replace_with, replace_if_zero);
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported matrix format.");
    }
}







}
}
