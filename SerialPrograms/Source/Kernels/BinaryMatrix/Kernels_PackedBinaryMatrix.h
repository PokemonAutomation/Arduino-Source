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
#include "Kernels_BinaryMatrixBase.h"

#if 0
#elif defined PA_Arch_x64_AVX512
#include "Kernels_BinaryMatrixTile_x64_AVX512.h"
namespace PokemonAutomation{
namespace Kernels{
    using PackedBinaryMatrix = BinaryMatrixBase<BinaryTile_AVX512>;
}
}
#elif defined PA_Arch_x64_AVX2
#include "Kernels_BinaryMatrixTile_x64_AVX2.h"
namespace PokemonAutomation{
namespace Kernels{
    using PackedBinaryMatrix = BinaryMatrixBase<BinaryTile_AVX2>;
}
}
#elif defined PA_Arch_x64_SSE42
#include "Kernels_BinaryMatrixTile_x64_SSE42.h"
namespace PokemonAutomation{
namespace Kernels{
    using PackedBinaryMatrix = BinaryMatrixBase<BinaryTile_SSE42>;
}
}
#else
#include "Kernels_BinaryMatrixTile_Default.h"
namespace PokemonAutomation{
namespace Kernels{
    using PackedBinaryMatrix = BinaryMatrixBase<BinaryTile_Default>;
}
}
#endif

#endif
