/*  Binary Matrix (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix_Arch_64x16_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{


template class PackedBinaryMatrixCore<BinaryTile_64x16_x64_AVX2>;
template class SparseBinaryMatrixCore<BinaryTile_64x16_x64_AVX2>;
template class PackedBinaryMatrix_t<BinaryTile_64x16_x64_AVX2>;
template class SparseBinaryMatrix_t<BinaryTile_64x16_x64_AVX2>;


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x16_x64_AVX2(){
    return std::make_unique<PackedBinaryMatrix_64x16_x64_AVX2>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x16_x64_AVX2(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_64x16_x64_AVX2>(width, height);
}

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x16_x64_AVX2(){
    return std::make_unique<SparseBinaryMatrix_64x16_x64_AVX2>();
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x16_x64_AVX2(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_64x16_x64_AVX2>(width, height);
}



}
}
#endif
