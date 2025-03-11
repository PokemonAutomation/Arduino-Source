/*  Binary Matrix (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_arm64_NEON_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_arm64_NEON_H

#include "Kernels_BinaryMatrixTile_64x8_arm64_NEON.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_64x8_arm64_NEON = PackedBinaryMatrixCore<BinaryTile_64x8_arm64_NEON>;
using SparseBinaryMatrixCore_64x8_arm64_NEON = SparseBinaryMatrixCore<BinaryTile_64x8_arm64_NEON>;

using PackedBinaryMatrix_64x8_arm64_NEON = PackedBinaryMatrix_t<BinaryTile_64x8_arm64_NEON>;
using SparseBinaryMatrix_64x8_arm64_NEON = SparseBinaryMatrix_t<BinaryTile_64x8_arm64_NEON>;



}
}
#endif
