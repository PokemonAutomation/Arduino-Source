/*  Scale Invariant Matrix Match (x86 SSE)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_08_Nehalem

#include <immintrin.h>
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



struct SumATA2_u8_x86_SSE{
    using vtype = __m128;

    __m128 sum_AT = _mm_setzero_ps();
    __m128 sum_A2 = _mm_setzero_ps();

    PA_FORCE_INLINE float scale() const{
        return reduce32_x64_SSE(sum_AT) / reduce32_x64_SSE(sum_A2);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        __m128 sum_at0 = _mm_setzero_ps();
        __m128 sum_at1 = _mm_setzero_ps();
        __m128 sum_as0 = _mm_setzero_ps();
        __m128 sum_as1 = _mm_setzero_ps();
        const __m128* ptrA = (const __m128*)A;
        const __m128* ptrT = (const __m128*)T;
        size_t lc = length / 8;
        do{
            __m128 a0 = ptrA[0];
            __m128 a1 = ptrA[1];
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, ptrT[0]));
            sum_at1 = _mm_add_ps(sum_at1, _mm_mul_ps(a1, ptrT[1]));
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_as1 = _mm_add_ps(sum_as1, _mm_mul_ps(a1, a1));
            ptrA += 2;
            ptrT += 2;
        }while (--lc);
        sum_at0 = _mm_add_ps(sum_at0, sum_at1);
        sum_as0 = _mm_add_ps(sum_as0, sum_as1);
        sum_AT = _mm_add_ps(sum_AT, sum_at0);
        sum_A2 = _mm_add_ps(sum_A2, sum_as0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW2, const float* W2){
        __m128 sum_at0 = _mm_setzero_ps();
        __m128 sum_at1 = _mm_setzero_ps();
        __m128 sum_as0 = _mm_setzero_ps();
        __m128 sum_as1 = _mm_setzero_ps();
        const __m128* ptrA = (const __m128*)A;
        const __m128* ptrT = (const __m128*)TW2;
        const __m128* ptrW = (const __m128*)W2;
        size_t lc = length / 8;
        do{
            __m128 a0 = ptrA[0];
            __m128 a1 = ptrA[1];
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, ptrT[0]));
            sum_at1 = _mm_add_ps(sum_at1, _mm_mul_ps(a1, ptrT[1]));
            a0 = _mm_mul_ps(a0, a0);
            a1 = _mm_mul_ps(a1, a1);
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, ptrW[0]));
            sum_as1 = _mm_add_ps(sum_as1, _mm_mul_ps(a1, ptrW[1]));
            ptrA += 2;
            ptrT += 2;
            ptrW += 2;
        }while (--lc);
        sum_at0 = _mm_add_ps(sum_at0, sum_at1);
        sum_as0 = _mm_add_ps(sum_as0, sum_as1);
        sum_AT = _mm_add_ps(sum_AT, sum_at0);
        sum_A2 = _mm_add_ps(sum_A2, sum_as0);
    }
};


float compute_scale_u8_x86_SSE(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2_u8_x86_SSE>(width, height, A, T);
}
float compute_scale_u8_x86_SSE(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,
    float const* const* W2
){
    return compute_scale<SumATA2_u8_x86_SSE>(width, height, A, TW2, W2);
}



}
}
}
#endif
