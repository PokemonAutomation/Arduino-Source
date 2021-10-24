/*  Kernels (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_x64_AVX2_H
#define PokemonAutomation_Kernels_x64_AVX2_H

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_x64_SSE41.h"

namespace PokemonAutomation{
namespace Kernels{


PA_FORCE_INLINE uint64_t reduce32_x64_AVX2(__m256i ymm){
    __m128i xmm = _mm_add_epi32(
        _mm256_castsi256_si128(ymm),
        _mm256_extracti128_si256(ymm, 1)
    );
    return reduce32_x64_SSE41(xmm);
}


}
}
#endif
