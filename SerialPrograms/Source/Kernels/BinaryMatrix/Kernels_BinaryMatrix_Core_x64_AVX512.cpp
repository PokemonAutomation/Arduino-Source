/*  Binary Matrix (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_17_Skylake

#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix_Arch_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{


template class PackedBinaryMatrixCore<BinaryTile_AVX512>;
template class SparseBinaryMatrixCore<BinaryTile_AVX512>;
template class PackedBinaryMatrix_t<BinaryTile_AVX512>;
template class SparseBinaryMatrix_t<BinaryTile_AVX512>;


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_AVX512(){
    return std::make_unique<PackedBinaryMatrix_x64_AVX512>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_AVX512(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_x64_AVX512>(width, height);
}

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_AVX512(){
    return std::make_unique<SparseBinaryMatrix_x64_AVX512>();
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_AVX512(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_x64_AVX512>(width, height);
}


}
}
#endif
