/*  Scale Invariant Matrix Match (x86 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_17_Skylake

#include <immintrin.h>
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



struct SumATA2_u32_x86_AVX512{
    using vtype = __m512;

    __m512 sum_AT = _mm512_setzero_ps();
    __m512 sum_A2 = _mm512_setzero_ps();

    PA_FORCE_INLINE float scale() const{
        return _mm512_reduce_add_ps(sum_AT) / _mm512_reduce_add_ps(sum_A2);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        __m512 sum_at0 = _mm512_setzero_ps();
        __m512 sum_at1 = _mm512_setzero_ps();
        __m512 sum_as0 = _mm512_setzero_ps();
        __m512 sum_as1 = _mm512_setzero_ps();
        const __m512* ptrA = (const __m512*)A;
        const __m512* ptrT = (const __m512*)T;
        size_t lc = length / 32;
        do{
            __m512 a0 = ptrA[0];
            __m512 a1 = ptrA[1];
            sum_at0 = _mm512_fmadd_ps(a0, ptrT[0], sum_at0);
            sum_at1 = _mm512_fmadd_ps(a1, ptrT[1], sum_at1);
            sum_as0 = _mm512_fmadd_ps(a0, a0, sum_as0);
            sum_as1 = _mm512_fmadd_ps(a1, a1, sum_as1);
            ptrA += 2;
            ptrT += 2;
        }while (--lc);
        sum_at0 = _mm512_add_ps(sum_at0, sum_at1);
        sum_as0 = _mm512_add_ps(sum_as0, sum_as1);
        sum_AT = _mm512_add_ps(sum_AT, sum_at0);
        sum_A2 = _mm512_add_ps(sum_A2, sum_as0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW2, const float* W2){
        __m512 sum_at0 = _mm512_setzero_ps();
        __m512 sum_at1 = _mm512_setzero_ps();
        __m512 sum_as0 = _mm512_setzero_ps();
        __m512 sum_as1 = _mm512_setzero_ps();
        const __m512* ptrA = (const __m512*)A;
        const __m512* ptrT = (const __m512*)TW2;
        const __m512* ptrW = (const __m512*)W2;
        size_t lc = length / 32;
        do{
            __m512 a0 = ptrA[0];
            __m512 a1 = ptrA[1];
            sum_at0 = _mm512_fmadd_ps(a0, ptrT[0], sum_at0);
            sum_at1 = _mm512_fmadd_ps(a1, ptrT[1], sum_at1);
            a0 = _mm512_mul_ps(a0, a0);
            a1 = _mm512_mul_ps(a1, a1);
            sum_as0 = _mm512_fmadd_ps(a0, ptrW[0], sum_as0);
            sum_as1 = _mm512_fmadd_ps(a1, ptrW[1], sum_as1);
            ptrA += 2;
            ptrT += 2;
            ptrW += 2;
        }while (--lc);
        sum_at0 = _mm512_add_ps(sum_at0, sum_at1);
        sum_as0 = _mm512_add_ps(sum_as0, sum_as1);
        sum_AT = _mm512_add_ps(sum_AT, sum_at0);
        sum_A2 = _mm512_add_ps(sum_A2, sum_as0);
    }
};


float compute_scale_u32_x86_AVX512(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2_u32_x86_AVX512>(width, height, A, T);
}
float compute_scale_u32_x86_AVX512(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,
    float const* const* W2
){
    return compute_scale<SumATA2_u32_x86_AVX512>(width, height, A, TW2, W2);
}



}
}
}
#endif
