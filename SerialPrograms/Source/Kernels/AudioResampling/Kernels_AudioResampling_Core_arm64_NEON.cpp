/*  Audio Resampling (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
#include "Kernels_AudioResampling.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampling{



float dot_product_arm64_NEON(const float* a, const float* b, size_t length){
    float32x4_t sum0 = vdupq_n_f32(0);
    float32x4_t sum1 = vdupq_n_f32(0);
    size_t lc = length / 8;
    while (lc--){
        sum0 = vfmaq_f32(sum0, vld1q_f32(a + 0), vld1q_f32(b + 0));
        sum1 = vfmaq_f32(sum1, vld1q_f32(a + 4), vld1q_f32(b + 4));
        a += 8;
        b += 8;
    }
    if (length % 8 >= 4){
        sum0 = vfmaq_f32(sum0, vld1q_f32(a), vld1q_f32(b));
        a += 4;
        b += 4;
    }
    float sum = vaddvq_f32(vaddq_f32(sum0, sum1));

    length %= 4;
    while (length--){
        sum += a[0] * b[0];
        a += 1;
        b += 1;
    }
    return sum;
}



}
}
}
#endif
