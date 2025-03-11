/*  Bit Scanning
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BitScaning_H
#define PokemonAutomation_Kernels_BitScaning_H

#include <stdint.h>
#include <cstddef>
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
    PA_FORCE_INLINE size_t bitlength(uint64_t x){
        unsigned long index;
        return _BitScanReverse64(&index, x) ? index + 1 : 0;
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
    PA_FORCE_INLINE size_t bitlength(uint64_t x){
        return x == 0 ? 0 : 64 - __builtin_clzll(x);
    }
}
}
#else
#error "No intrinsic for this compiler."
#endif

#endif
