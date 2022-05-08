/*  Scale Invariant Matrix Match (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <immintrin.h>
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



struct SumATA2_u2_Default{
    using vtype = float;

    float sum_AT = 0;
    float sum_A2 = 0;

    PA_FORCE_INLINE float scale() const{
        return sum_AT / sum_A2;
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        float sum_at0 = 0;
        float sum_at1 = 0;
        float sum_as0 = 0;
        float sum_as1 = 0;
        for (size_t c = 0; c < length; c += 2){
            float a0 = A[c + 0];
            float a1 = A[c + 1];
            sum_at0 += a0 * T[c + 0];
            sum_at1 += a1 * T[c + 1];
            sum_as0 += a0 * a0;
            sum_as1 += a1 * a1;
        }
        sum_AT += sum_at0 + sum_at1;
        sum_A2 += sum_as0 + sum_as1;
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW2, const float* W2){
        float sum_at0 = 0;
        float sum_at1 = 0;
        float sum_as0 = 0;
        float sum_as1 = 0;
        for (size_t c = 0; c < length; c += 2){
            float a0 = A[c + 0];
            float a1 = A[c + 1];
            sum_at0 += a0 * TW2[c + 0];
            sum_at1 += a1 * TW2[c + 1];
            sum_as0 += a0 * a0 * W2[c + 0];
            sum_as1 += a1 * a1 * W2[c + 1];
        }
        sum_AT += sum_at0 + sum_at1;
        sum_A2 += sum_as0 + sum_as1;
    }
};


float compute_scale_u2_Default(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2_u2_Default>(width, height, A, T);
}
float compute_scale_u2_Default(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,
    float const* const* W2
){
    return compute_scale<SumATA2_u2_Default>(width, height, A, TW2, W2);
}



}
}
}
