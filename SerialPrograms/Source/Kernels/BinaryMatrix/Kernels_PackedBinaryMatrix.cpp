/*  Packed Binary Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_BinaryMatrixBase.tpp"

#include "Kernels/Kernels_Arch.h"

#include "Kernels_BinaryMatrixTile_Default.h"
namespace PokemonAutomation{
namespace Kernels{
    template class BinaryMatrixBase<BinaryTile_Default>;
}
}

#ifdef PA_Arch_x64_SSE42
#include "Kernels_BinaryMatrixTile_x64_SSE42.h"
namespace PokemonAutomation{
namespace Kernels{
    template class BinaryMatrixBase<BinaryTile_SSE42>;
}
}
#endif
#ifdef PA_Arch_x64_AVX2
#include "Kernels_BinaryMatrixTile_x64_AVX2.h"
namespace PokemonAutomation{
namespace Kernels{
    template class BinaryMatrixBase<BinaryTile_AVX2>;
}
}
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels_BinaryMatrixTile_x64_AVX512.h"
namespace PokemonAutomation{
namespace Kernels{
    template class BinaryMatrixBase<BinaryTile_AVX512>;
}
}
#endif

