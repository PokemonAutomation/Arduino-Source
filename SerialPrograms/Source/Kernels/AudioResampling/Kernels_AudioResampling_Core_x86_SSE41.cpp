/*  Audio Resampling (x86 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <immintrin.h>
#include <smmintrin.h>
#include "Kernels_AudioResampling.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampling{



float dot_product_x86_SSE41(const float* a, const float* b, size_t length){
    __m128 sum0 = _mm_setzero_ps();
    __m128 sum1 = _mm_setzero_ps();
    size_t lc = length / 8;
    while (lc--){
        sum0 = _mm_add_ps(sum0, _mm_mul_ps(_mm_loadu_ps(a + 0), _mm_loadu_ps(b + 0)));
        sum1 = _mm_add_ps(sum1, _mm_mul_ps(_mm_loadu_ps(a + 4), _mm_loadu_ps(b + 4)));
        a += 8;
        b += 8;
    }
    if (length % 8 >= 4){
        sum0 = _mm_add_ps(sum0, _mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
        a += 4;
        b += 4;
    }
    sum0 = _mm_add_ps(sum0, sum1);

    //  Horizontal sum of the 4 lanes.
    sum0 = _mm_add_ps(sum0, _mm_movehl_ps(sum0, sum0));
    sum0 = _mm_add_ss(sum0, _mm_shuffle_ps(sum0, sum0, 1));

    length %= 4;
    while (length--){
        sum0 = _mm_add_ss(sum0, _mm_mul_ss(_mm_load_ss(a), _mm_load_ss(b)));
        a += 1;
        b += 1;
    }
    return _mm_cvtss_f32(sum0);
}



}
}
}
#endif
