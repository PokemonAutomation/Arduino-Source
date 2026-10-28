/*  Binary Matrix (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_08_Nehalem

#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix_Arch_x64_SSE42.h"

namespace PokemonAutomation{
namespace Kernels{


template class PackedBinaryMatrixCore<BinaryTile_SSE42>;
template class SparseBinaryMatrixCore<BinaryTile_SSE42>;
template class PackedBinaryMatrix_t<BinaryTile_SSE42>;
template class SparseBinaryMatrix_t<BinaryTile_SSE42>;


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_SSE42(){
    return std::make_unique<PackedBinaryMatrix_x64_SSE42>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_SSE42(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_x64_SSE42>(width, height);
}

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_SSE42(){
    return std::make_unique<SparseBinaryMatrix_x64_SSE42>();
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_SSE42(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_x64_SSE42>(width, height);
}



}
}
#endif
