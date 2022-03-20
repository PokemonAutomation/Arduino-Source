/*  Binary Matrix (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_x64_AVX2_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_x64_AVX2_H

#include "Kernels_BinaryMatrixTile_x64_AVX2.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_x64_AVX2 = PackedBinaryMatrixCore<BinaryTile_AVX2>;
using SparseBinaryMatrixCore_x64_AVX2 = SparseBinaryMatrixCore<BinaryTile_AVX2>;

using PackedBinaryMatrix_x64_AVX2 = PackedBinaryMatrix_t<BinaryTile_AVX2>;
using SparseBinaryMatrix_x64_AVX2 = SparseBinaryMatrix_t<BinaryTile_AVX2>;



}
}
#endif
