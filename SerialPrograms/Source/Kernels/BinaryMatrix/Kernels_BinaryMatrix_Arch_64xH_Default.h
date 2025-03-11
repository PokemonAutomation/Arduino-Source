/*  Binary Matrix (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_64x4_Default_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_64x4_Default_H

#include "Kernels_BinaryMatrixTile_64x4_Default.h"
#include "Kernels_BinaryMatrixTile_64xH_Default.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_64x4_Default = PackedBinaryMatrixCore<BinaryTile_64x4_Default>;
using SparseBinaryMatrixCore_64x4_Default = SparseBinaryMatrixCore<BinaryTile_64x4_Default>;
using PackedBinaryMatrix_64x4_Default = PackedBinaryMatrix_t<BinaryTile_64x4_Default>;
using SparseBinaryMatrix_64x4_Default = SparseBinaryMatrix_t<BinaryTile_64x4_Default>;

using BinaryTile_64x8_Default = BinaryTile_64xH_Default<8, BinaryMatrixType::i64x8_Default>;
using PackedBinaryMatrixCore_64x8_Default = PackedBinaryMatrixCore<BinaryTile_64x8_Default>;
using SparseBinaryMatrixCore_64x8_Default = SparseBinaryMatrixCore<BinaryTile_64x8_Default>;
using PackedBinaryMatrix_64x8_Default = PackedBinaryMatrix_t<BinaryTile_64x8_Default>;
using SparseBinaryMatrix_64x8_Default = SparseBinaryMatrix_t<BinaryTile_64x8_Default>;



}
}
#endif
