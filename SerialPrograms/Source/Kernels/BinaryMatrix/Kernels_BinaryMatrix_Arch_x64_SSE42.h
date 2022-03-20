/*  Binary Matrix (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrix_Arch_x64_SSE42_H
#define PokemonAutomation_Kernels_BinaryMatrix_Arch_x64_SSE42_H

#include "Kernels_BinaryMatrixTile_x64_SSE42.h"
#include "Kernels_BinaryMatrix_t.h"
#include "Kernels_PackedBinaryMatrixCore.h"
#include "Kernels_SparseBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


using PackedBinaryMatrixCore_x64_SSE42 = PackedBinaryMatrixCore<BinaryTile_SSE42>;
using SparseBinaryMatrixCore_x64_SSE42 = SparseBinaryMatrixCore<BinaryTile_SSE42>;

using PackedBinaryMatrix_x64_SSE42 = PackedBinaryMatrix_t<BinaryTile_SSE42>;
using SparseBinaryMatrix_x64_SSE42 = SparseBinaryMatrix_t<BinaryTile_SSE42>;



}
}
#endif
