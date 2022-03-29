/*  Binary Matrix (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix_Arch_64x4_Default.h"

namespace PokemonAutomation{
namespace Kernels{


template class PackedBinaryMatrixCore<BinaryTile_64x4_Default>;
template class SparseBinaryMatrixCore<BinaryTile_64x4_Default>;
template class PackedBinaryMatrix_t<BinaryTile_64x4_Default>;
template class SparseBinaryMatrix_t<BinaryTile_64x4_Default>;


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_Default(){
    return std::make_unique<PackedBinaryMatrix_Default>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_Default(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_Default>(width, height);
}

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_Default(){
    return std::make_unique<SparseBinaryMatrix_x64_Default>();
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_Default(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_x64_Default>(width, height);
}


}
}
