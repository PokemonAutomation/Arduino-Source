/*  Binary Matrix (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix_Arch_64xH_Default.h"

namespace PokemonAutomation{
namespace Kernels{


template class PackedBinaryMatrixCore<BinaryTile_64x4_Default>;
template class SparseBinaryMatrixCore<BinaryTile_64x4_Default>;
template class PackedBinaryMatrix_t<BinaryTile_64x4_Default>;
template class SparseBinaryMatrix_t<BinaryTile_64x4_Default>;
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x4_Default(){
    return std::make_unique<PackedBinaryMatrix_64x4_Default>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x4_Default(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_64x4_Default>(width, height);
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x4_Default(){
    return std::make_unique<SparseBinaryMatrix_64x4_Default>();
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x4_Default(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_64x4_Default>(width, height);
}


template class PackedBinaryMatrixCore<BinaryTile_64x8_Default>;
template class SparseBinaryMatrixCore<BinaryTile_64x8_Default>;
template class PackedBinaryMatrix_t<BinaryTile_64x8_Default>;
template class SparseBinaryMatrix_t<BinaryTile_64x8_Default>;
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_Default(){
    return std::make_unique<PackedBinaryMatrix_64x8_Default>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_Default(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_64x8_Default>(width, height);
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_Default(){
    return std::make_unique<SparseBinaryMatrix_64x8_Default>();
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_Default(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_64x8_Default>(width, height);
}


}
}
