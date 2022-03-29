/*  Binary Matrix (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_64x4_Default_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_64x4_Default_H

#include "Kernels_BinaryMatrixTile_64x4_Default.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_x64_Default = PackedBinaryMatrixCore<BinaryTile_64x4_Default>;
using SparseBinaryMatrixCore_x64_Default = SparseBinaryMatrixCore<BinaryTile_64x4_Default>;

using PackedBinaryMatrix_Default = PackedBinaryMatrix_t<BinaryTile_64x4_Default>;
using SparseBinaryMatrix_x64_Default = SparseBinaryMatrix_t<BinaryTile_64x4_Default>;



}
}
#endif
