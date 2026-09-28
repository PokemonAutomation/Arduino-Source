/*  Audio Resampling (x86 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <immintrin.h>
#include "Kernels_AudioResampling.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampling{



float dot_product_x86_AVX2(const float* a, const float* b, size_t length){
    __m256 sum0 = _mm256_setzero_ps();
    __m256 sum1 = _mm256_setzero_ps();
    size_t lc = length / 16;
    while (lc--){
        sum0 = _mm256_fmadd_ps(_mm256_loadu_ps(a + 0), _mm256_loadu_ps(b + 0), sum0);
        sum1 = _mm256_fmadd_ps(_mm256_loadu_ps(a + 8), _mm256_loadu_ps(b + 8), sum1);
        a += 16;
        b += 16;
    }
    if (length % 16 >= 8){
        sum0 = _mm256_fmadd_ps(_mm256_loadu_ps(a), _mm256_loadu_ps(b), sum0);
        a += 8;
        b += 8;
    }
    sum0 = _mm256_add_ps(sum0, sum1);

    //  Horizontal sum of the 8 lanes.
    __m128 sum = _mm_add_ps(_mm256_castps256_ps128(sum0), _mm256_extractf128_ps(sum0, 1));
    sum = _mm_add_ps(sum, _mm_movehl_ps(sum, sum));
    sum = _mm_add_ss(sum, _mm_shuffle_ps(sum, sum, 1));

    length %= 8;
    while (length--){
        sum = _mm_fmadd_ss(_mm_load_ss(a), _mm_load_ss(b), sum);
        a += 1;
        b += 1;
    }
    return _mm_cvtss_f32(sum);
}



}
}
}
#endif
