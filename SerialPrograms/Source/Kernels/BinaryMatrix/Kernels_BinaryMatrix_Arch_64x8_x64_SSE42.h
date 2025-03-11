/*  Binary Matrix (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_x64_SSE42_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_x64_SSE42_H

#include "Kernels_BinaryMatrixTile_64x8_x64_SSE42.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_64x8_x64_SSE42 = PackedBinaryMatrixCore<BinaryTile_64x8_x64_SSE42>;
using SparseBinaryMatrixCore_64x8_x64_SSE42 = SparseBinaryMatrixCore<BinaryTile_64x8_x64_SSE42>;

using PackedBinaryMatrix_64x8_x64_SSE42 = PackedBinaryMatrix_t<BinaryTile_64x8_x64_SSE42>;
using SparseBinaryMatrix_64x8_x64_SSE42 = SparseBinaryMatrix_t<BinaryTile_64x8_x64_SSE42>;



}
}
#endif
