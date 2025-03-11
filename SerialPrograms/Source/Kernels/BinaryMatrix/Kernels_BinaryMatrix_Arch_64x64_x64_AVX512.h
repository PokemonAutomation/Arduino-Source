/*  Binary Matrix (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_64x64_x64_AVX512_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_64x64_x64_AVX512_H

#include "Kernels_BinaryMatrixTile_64x64_x64_AVX512.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_64x64_x64_AVX512 = PackedBinaryMatrixCore<BinaryTile_64x64_x64_AVX512>;
using SparseBinaryMatrixCore_64x64_x64_AVX512 = SparseBinaryMatrixCore<BinaryTile_64x64_x64_AVX512>;

using PackedBinaryMatrix_64x64_x64_AVX512 = PackedBinaryMatrix_t<BinaryTile_64x64_x64_AVX512>;
using SparseBinaryMatrix_64x64_x64_AVX512 = SparseBinaryMatrix_t<BinaryTile_64x64_x64_AVX512>;




}
}
#endif
