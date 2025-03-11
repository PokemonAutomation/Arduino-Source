/*  Binary Matrix (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix_Arch_64x32_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{

template class PackedBinaryMatrixCore<BinaryTile_64x32_x64_AVX512>;
template class SparseBinaryMatrixCore<BinaryTile_64x32_x64_AVX512>;
template class PackedBinaryMatrix_t<BinaryTile_64x32_x64_AVX512>;
template class SparseBinaryMatrix_t<BinaryTile_64x32_x64_AVX512>;


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x32_x64_AVX512(){
    return std::make_unique<PackedBinaryMatrix_64x32_x64_AVX512>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x32_x64_AVX512(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_64x32_x64_AVX512>(width, height);
}

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x32_x64_AVX512(){
    return std::make_unique<SparseBinaryMatrix_64x32_x64_AVX512>();
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x32_x64_AVX512(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_64x32_x64_AVX512>(width, height);
}


}
}
#endif
