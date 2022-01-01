/*  Trailing Zeros (Bit Scan Forward)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_TrailingZeros_H
#define PokemonAutomation_Kernels_TrailingZeros_H

#include <stdint.h>
#include "Common/Compiler.h"

#if 0

#elif _WIN64
#include <intrin.h>
namespace PokemonAutomation{
namespace Kernels{
    PA_FORCE_INLINE bool trailing_zeros(size_t& zeros, uint64_t x){
        unsigned long tmp;
        char ret = _BitScanForward64(&tmp, x);
        zeros = tmp;
        return ret != 0;
    }
}
}
#elif __GNUC__
namespace PokemonAutomation{
namespace Kernels{
    PA_FORCE_INLINE bool trailing_zeros(size_t& zeros, uint64_t x){
        zeros = __builtin_ctzll(x);
        return x != 0;
    }
}
}
#else
#error "No intrinsic for this compiler."
#endif

#endif
