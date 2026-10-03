/*  Audio Resampling (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_AudioResampling.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampling{



float dot_product_Default(const float* a, const float* b, size_t length){
    //  4 independent accumulators so the additions can overlap.
    float sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
    size_t lc = length / 4;
    while (lc--){
        sum0 += a[0] * b[0];
        sum1 += a[1] * b[1];
        sum2 += a[2] * b[2];
        sum3 += a[3] * b[3];
        a += 4;
        b += 4;
    }
    length %= 4;
    while (length--){
        sum0 += a[0] * b[0];
        a += 1;
        b += 1;
    }
    return (sum0 + sum1) + (sum2 + sum3);
}



}
}
}
