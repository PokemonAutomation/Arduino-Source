/*  Waterfill Algorithm
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/CpuId.h"
#include "Kernels_Waterfill.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



bool find_object_on_bit_Default     (PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y);
bool find_object_on_bit_x64_SSE42   (PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y);
bool find_object_on_bit_x64_AVX2    (PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y);
bool find_object_on_bit_x64_AVX512  (PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y);
bool find_object_on_bit_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y);

bool find_object_on_bit(PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        if (CPU_CAPABILITY_CURRENT.HW_GFNI){
            return find_object_on_bit_x64_AVX512GF(matrix, object, x, y);
        }else{
            return find_object_on_bit_x64_AVX512(matrix, object, x, y);
        }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return find_object_on_bit_x64_AVX2(matrix, object, x, y);
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return find_object_on_bit_x64_SSE42(matrix, object, x, y);
#endif
    default:
        return find_object_on_bit_Default(matrix, object, x, y);
    }
}





std::vector<WaterfillObject> find_objects_inplace_Default       (PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects);
std::vector<WaterfillObject> find_objects_inplace_x64_SSE42     (PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects);
std::vector<WaterfillObject> find_objects_inplace_x64_AVX2      (PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects);
std::vector<WaterfillObject> find_objects_inplace_x64_AVX512    (PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects);
std::vector<WaterfillObject> find_objects_inplace_x64_AVX512GF  (PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects);

std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        if (CPU_CAPABILITY_CURRENT.HW_GFNI){
            return find_objects_inplace_x64_AVX512GF(matrix, min_area, keep_objects);
        }else{
            return find_objects_inplace_x64_AVX512(matrix, min_area, keep_objects);
        }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return find_objects_inplace_x64_AVX2(matrix, min_area, keep_objects);
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return find_objects_inplace_x64_SSE42(matrix, min_area, keep_objects);
#endif
    default:
        return find_objects_inplace_Default(matrix, min_area, keep_objects);
    }
}
std::vector<WaterfillObject> find_objects(const PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects){
    std::unique_ptr<PackedBinaryMatrix_IB> m = matrix.clone();
    return find_objects_inplace(*m, min_area, keep_objects);
}





std::unique_ptr<WaterfillIterator2> make_WaterfillIterator_Default      (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator2> make_WaterfillIterator_x64_SSE42    (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator2> make_WaterfillIterator_x64_AVX2     (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator2> make_WaterfillIterator_x64_AVX512   (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::unique_ptr<WaterfillIterator2> make_WaterfillIterator_x64_AVX512GF (PackedBinaryMatrix_IB& matrix, size_t min_area);

std::unique_ptr<WaterfillIterator2> make_WaterfillIterator(PackedBinaryMatrix_IB& matrix, size_t min_area){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        if (CPU_CAPABILITY_CURRENT.HW_GFNI){
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
