/*  Packed Binary Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Kernels_Arch.h"
#include "Kernels_BinaryMatrixTile.h"
#include "Kernels_PackedBinaryMatrixBase.tpp"
#include "Kernels_SparseBinaryMatrixBase.tpp"

namespace PokemonAutomation{
namespace Kernels{
    template class PackedBinaryMatrixBase<BinaryMatrixTile>;
    template class SparseBinaryMatrixBase<BinaryMatrixTile>;
}
}
