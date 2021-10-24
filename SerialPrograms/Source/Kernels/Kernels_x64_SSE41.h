/*  Kernels (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_x64_SSE41_H
#define PokemonAutomation_Kernels_x64_SSE41_H

#include <stdint.h>
#include <smmintrin.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


PA_FORCE_INLINE uint64_t reduce32_x64_SSE41(__m128i x){
    uint64_t ret = _mm_cvtsi128_si32(x);
    ret += _mm_extract_epi32(x, 1);
    ret += _mm_extract_epi32(x, 2);
    ret += _mm_extract_epi32(x, 3);
    return ret;
}


}
}
#endif
