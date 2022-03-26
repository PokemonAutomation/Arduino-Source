/*  Waterfill Algorithm
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/CpuId.h"
#include "Kernels_Waterfill.h"
#include "Kernels_Waterfill_Session.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



std::vector<WaterfillObject> find_objects_inplace_Default       (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_x64_SSE42     (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_x64_AVX2      (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_x64_AVX512    (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_x64_AVX512GF  (PackedBinaryMatrix_IB& matrix, size_t min_area);

std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrix_IB& matrix, size_t min_area){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return find_objects_inplace_x64_AVX512GF(matrix, min_area);
        }else{
            return find_objects_inplace_x64_AVX512(matrix, min_area);
        }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return find_objects_inplace_x64_AVX2(matrix, min_area);
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return find_objects_inplace_x64_SSE42(matrix, min_area);
#endif
    default:
        return find_objects_inplace_Default(matrix, min_area);
    }
}




std::unique_ptr<WaterfillIterator> make_WaterfillIterator_Default      (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator> make_WaterfillIterator_x64_SSE42    (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator> make_WaterfillIterator_x64_AVX2     (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator> make_WaterfillIterator_x64_AVX512   (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator> make_WaterfillIterator_x64_AVX512GF (PackedBinaryMatrix_IB& matrix, size_t min_area);

std::unique_ptr<WaterfillIterator> make_WaterfillIterator(PackedBinaryMatrix_IB& matrix, size_t min_area){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return make_WaterfillIterator_x64_AVX512GF(matrix, min_area);
        }else{
            return make_WaterfillIterator_x64_AVX512(matrix, min_area);
        }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return make_WaterfillIterator_x64_AVX2(matrix, min_area);
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return make_WaterfillIterator_x64_SSE42(matrix, min_area);
#endif
    default:
        return make_WaterfillIterator_Default(matrix, min_area);
    }
}






}
}
}
