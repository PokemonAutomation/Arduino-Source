/*  Binary Matrix (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_64x16_x64_AVX2_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_64x16_x64_AVX2_H

#include "Kernels_BinaryMatrixTile_64x16_x64_AVX2.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_64x16_x64_AVX2 = PackedBinaryMatrixCore<BinaryTile_64x16_x64_AVX2>;
using SparseBinaryMatrixCore_64x16_x64_AVX2 = SparseBinaryMatrixCore<BinaryTile_64x16_x64_AVX2>;

using PackedBinaryMatrix_64x16_x64_AVX2 = PackedBinaryMatrix_t<BinaryTile_64x16_x64_AVX2>;
using SparseBinaryMatrix_64x16_x64_AVX2 = SparseBinaryMatrix_t<BinaryTile_64x16_x64_AVX2>;



}
}
#endif
