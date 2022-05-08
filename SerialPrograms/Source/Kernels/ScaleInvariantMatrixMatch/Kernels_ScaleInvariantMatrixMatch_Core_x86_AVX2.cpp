/*  Scale Invariant Matrix Match (x86 FMA3)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_13_Haswell

#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



struct SumATA2_u16_x86_FMA3{
    using vtype = __m256;

    __m256 sum_AT = _mm256_setzero_ps();
    __m256 sum_A2 = _mm256_setzero_ps();

    PA_FORCE_INLINE float scale() const{
        return reduce32_x64_AVX(sum_AT) / reduce32_x64_AVX(sum_A2);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        __m256 sum_at0 = _mm256_setzero_ps();
        __m256 sum_at1 = _mm256_setzero_ps();
        __m256 sum_as0 = _mm256_setzero_ps();
        __m256 sum_as1 = _mm256_setzero_ps();
        const __m256* ptrA = (const __m256*)A;
        const __m256* ptrT = (const __m256*)T;
        size_t lc = length / 16;
        do{
            __m256 a0 = ptrA[0];
            __m256 a1 = ptrA[1];
            sum_at0 = _mm256_fmadd_ps(a0, ptrT[0], sum_at0);
            sum_at1 = _mm256_fmadd_ps(a1, ptrT[1], sum_at1);
            sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
            sum_as1 = _mm256_fmadd_ps(a1, a1, sum_as1);
            ptrA += 2;
            ptrT += 2;
        }while (--lc);
        sum_at0 = _mm256_add_ps(sum_at0, sum_at1);
        sum_as0 = _mm256_add_ps(sum_as0, sum_as1);
        sum_AT = _mm256_add_ps(sum_AT, sum_at0);
        sum_A2 = _mm256_add_ps(sum_A2, sum_as0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW2, const float* W2){
        __m256 sum_at0 = _mm256_setzero_ps();
        __m256 sum_at1 = _mm256_setzero_ps();
        __m256 sum_as0 = _mm256_setzero_ps();
        __m256 sum_as1 = _mm256_setzero_ps();
        const __m256* ptrA = (const __m256*)A;
        const __m256* ptrT = (const __m256*)TW2;
        const __m256* ptrW = (const __m256*)W2;
        size_t lc = length / 16;
        do{
            __m256 a0 = ptrA[0];
            __m256 a1 = ptrA[1];
            sum_at0 = _mm256_fmadd_ps(a0, ptrT[0], sum_at0);
            sum_at1 = _mm256_fmadd_ps(a1, ptrT[1], sum_at1);
            a0 = _mm256_mul_ps(a0, a0);
            a1 = _mm256_mul_ps(a1, a1);
            sum_as0 = _mm256_fmadd_ps(a0, ptrW[0], sum_as0);
            sum_as1 = _mm256_fmadd_ps(a1, ptrW[1], sum_as1);
            ptrA += 2;
            ptrT += 2;
            ptrW += 2;
        }while (--lc);
        sum_at0 = _mm256_add_ps(sum_at0, sum_at1);
        sum_as0 = _mm256_add_ps(sum_as0, sum_as1);
        sum_AT = _mm256_add_ps(sum_AT, sum_at0);
        sum_A2 = _mm256_add_ps(sum_A2, sum_as0);
    }
};


float compute_scale_u16_x86_FMA3(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2_u16_x86_FMA3>(width, height, A, T);
}
float compute_scale_u16_x86_FMA3(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,
    float const* const* W2
){
    return compute_scale<SumATA2_u16_x86_FMA3>(width, height, A, TW2, W2);
}



}
}
}
#endif
