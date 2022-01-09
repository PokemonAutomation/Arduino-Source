/*  Packed Binary Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This class represents a memory-efficient binary matrix where each bit
 *  is stored as just one bit in memory.
 *
 *  The representation uses "tiles". So instead of having each row contiguous
 *  in memory, the space is broken up into "tiles" that provide some vertical
 *  spatial locality.
 *
 *  This class is mainly used by the waterfill algorithm which needs vertical
 *  locality in memory.
 *
 */

#ifndef PokemonAutomation_Kernels_PackedBinaryMatrix_H
#define PokemonAutomation_Kernels_PackedBinaryMatrix_H

#include "Kernels/Kernels_Arch.h"
#include "Kernels_BinaryMatrixTile.h"
#include "Kernels_PackedBinaryMatrixBase.h"
#include "Kernels_SparseBinaryMatrixBase.h"

namespace PokemonAutomation{
namespace Kernels{

using PackedBinaryMatrix = PackedBinaryMatrixBase<BinaryMatrixTile>;
using SparseBinaryMatrix = SparseBinaryMatrixBase<BinaryMatrixTile>;

}
}
#endif
