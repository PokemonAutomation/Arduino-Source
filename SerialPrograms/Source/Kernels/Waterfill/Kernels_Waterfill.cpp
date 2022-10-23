/*  Waterfill Algorithm
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_Waterfill.h"
#include "Kernels_Waterfill_Session.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



std::vector<WaterfillObject> find_objects_inplace_64x4_Default      (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x8_x64_SSE42    (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x16_x64_AVX2    (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x32_x64_AVX512  (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x64_x64_AVX512  (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x32_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x64_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, size_t min_area);

std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrix_IB& matrix, size_t min_area){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return find_objects_inplace_64x64_x64_AVX512GF(matrix, min_area);
        }else{
            return find_objects_inplace_64x64_x64_AVX512(matrix, min_area);
        }
    case BinaryMatrixType::i64x32_x64_AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return find_objects_inplace_64x32_x64_AVX512GF(matrix, min_area);
        }else{
            return find_objects_inplace_64x32_x64_AVX512(matrix, min_area);
        }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        return find_objects_inplace_64x16_x64_AVX2(matrix, min_area);
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        return find_objects_inplace_64x8_x64_SSE42(matrix, min_area);
#endif
    default:
        return find_objects_inplace_64x4_Default(matrix, min_area);
    }
}




}
}
}
