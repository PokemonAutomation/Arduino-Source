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


PA_FORCE_INLINE __m256i mm256_permuteaj64_00(__m256i a, __m256i b){
    return _mm256_unpacklo_epi64(a, b);
}
PA_FORCE_INLINE __m256i mm256_permuteaj64_11(__m256i a, __m256i b){
    return _mm256_unpackhi_epi64(a, b);
}
PA_FORCE_INLINE __m256i mm256_permuteaj128_00(__m256i a, __m256i b){
    return _mm256_permute2x128_si256(a, b, 32);
}
PA_FORCE_INLINE __m256i mm256_permuteaj128_11(__m256i a, __m256i b){
    return _mm256_permute2x128_si256(a, b, 49);
}
PA_FORCE_INLINE void transpose_i64_4x4_AVX2(__m256i& r0, __m256i& r1, __m256i& r2, __m256i& r3){
    __m256i a0, a1, a2, a3;
    a0 = mm256_permuteaj128_00(r0, r2);
    a1 = mm256_permuteaj128_11(r0, r2);
    a2 = mm256_permuteaj128_00(r1, r3);
    a3 = mm256_permuteaj128_11(r1, r3);
    r0 = mm256_permuteaj64_00(a0, a2);
    r1 = mm256_permuteaj64_11(a0, a2);
    r2 = mm256_permuteaj64_00(a1, a3);
    r3 = mm256_permuteaj64_11(a1, a3);
}



}
}
#endif
