/*  Binary Matrix (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix_Arch_64x8_arm64_NEON.h"

namespace PokemonAutomation{
namespace Kernels{


template class PackedBinaryMatrixCore<BinaryTile_64x8_arm64_NEON>;
template class SparseBinaryMatrixCore<BinaryTile_64x8_arm64_NEON>;
template class PackedBinaryMatrix_t<BinaryTile_64x8_arm64_NEON>;
template class SparseBinaryMatrix_t<BinaryTile_64x8_arm64_NEON>;


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_arm64_NEON(){
    return std::make_unique<PackedBinaryMatrix_64x8_arm64_NEON>();
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_arm64_NEON(size_t width, size_t height){
    return std::make_unique<PackedBinaryMatrix_64x8_arm64_NEON>(width, height);
}

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_arm64_NEON(){
    return std::make_unique<SparseBinaryMatrix_64x8_arm64_NEON>(); 
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_arm64_NEON(size_t width, size_t height){
    return std::make_unique<SparseBinaryMatrix_64x8_arm64_NEON>(width, height);
}



}
}
#endif
