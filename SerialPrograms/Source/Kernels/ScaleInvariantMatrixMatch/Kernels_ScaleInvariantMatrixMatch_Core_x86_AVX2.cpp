/*  Scale Invariant Matrix Match (x86 FMA3)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_13_Haswell

#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



struct SumATA2_min8_x86_FMA3{
    using vtype = __m256;

    __m256 sum_AT = _mm256_setzero_ps();
    __m256 sum_A2 = _mm256_setzero_ps();

    PA_FORCE_INLINE float scale() const{
        return reduce32_x64_AVX(sum_AT) / reduce32_x64_AVX(sum_A2);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        __m256 sum_as0 = _mm256_setzero_ps();
        __m256 sum_as1 = _mm256_setzero_ps();
        __m256 sum_as2 = _mm256_setzero_ps();
        __m256 sum_as3 = _mm256_setzero_ps();
        __m256 sum_at0 = _mm256_setzero_ps();
        __m256 sum_at1 = _mm256_setzero_ps();
        __m256 sum_at2 = _mm256_setzero_ps();
        __m256 sum_at3 = _mm256_setzero_ps();

        size_t align = (size_t)T % 32;
        if (align){
            align /= sizeof(float);
            A -= align;
            T -= align;

            __m256i mask = _mm256_cmpgt_epi32(
                _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8),
                _mm256_set1_epi32((uint32_t)align)
            );

            __m256 a0 = _mm256_maskload_ps(A, mask);
            __m256 t0 = _mm256_maskload_ps(T, mask);
            sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
            sum_at0 = _mm256_fmadd_ps(a0, t0, sum_at0);

            A += 8;
            T += 8;
            length -= 8 - align;
        }

        const __m256* ptrA = (const __m256*)A;
        const __m256* ptrT = (const __m256*)T;

        size_t lc = length / 32;
        if (lc){
            do{
                __m256 a0 = ptrA[0];
                __m256 a1 = ptrA[1];
                __m256 a2 = ptrA[2];
                __m256 a3 = ptrA[3];
                sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
                sum_as1 = _mm256_fmadd_ps(a1, a1, sum_as1);
                sum_as2 = _mm256_fmadd_ps(a2, a2, sum_as2);
                sum_as3 = _mm256_fmadd_ps(a3, a3, sum_as3);
                sum_at0 = _mm256_fmadd_ps(a0, ptrT[0], sum_at0);
                sum_at1 = _mm256_fmadd_ps(a1, ptrT[1], sum_at1);
                sum_at2 = _mm256_fmadd_ps(a2, ptrT[2], sum_at2);
                sum_at3 = _mm256_fmadd_ps(a3, ptrT[3], sum_at3);
                ptrA += 4;
                ptrT += 4;
            }while (--lc);
            sum_as1 = _mm256_add_ps(sum_as1, sum_as2);
            sum_at1 = _mm256_add_ps(sum_at1, sum_at2);
            sum_as1 = _mm256_add_ps(sum_as1, sum_as3);
            sum_at1 = _mm256_add_ps(sum_at1, sum_at3);
            sum_as0 = _mm256_add_ps(sum_as0, sum_as1);
            sum_at0 = _mm256_add_ps(sum_at0, sum_at1);
        }

        length %= 32;
        while (length >= 8){
            __m256 a0 = ptrA[0];
            sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
            sum_at0 = _mm256_fmadd_ps(a0, ptrT[0], sum_at0);
            ptrA += 1;
            ptrT += 1;
            length -= 8;
        }
        if (length){
            PartialWordAccess32_x64_AVX2 access(length);
            __m256 a0 = access.load_f32(ptrA);
            __m256 t0 = access.load_f32(ptrT);
            sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
            sum_at0 = _mm256_fmadd_ps(a0, t0, sum_at0);
        }

        sum_A2 = _mm256_add_ps(sum_A2, sum_as0);
        sum_AT = _mm256_add_ps(sum_AT, sum_at0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW, const float* W){
        __m256 sum_as0 = _mm256_setzero_ps();
        __m256 sum_as1 = _mm256_setzero_ps();
        __m256 sum_as2 = _mm256_setzero_ps();
        __m256 sum_as3 = _mm256_setzero_ps();
        __m256 sum_at0 = _mm256_setzero_ps();
        __m256 sum_at1 = _mm256_setzero_ps();
        __m256 sum_at2 = _mm256_setzero_ps();
        __m256 sum_at3 = _mm256_setzero_ps();

        size_t align = (size_t)TW % 32;
        if (align){
            align /= sizeof(float);
            A -= align;
            TW -= align;
            W -= align;

            __m256i mask = _mm256_cmpgt_epi32(
                _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8),
                _mm256_set1_epi32((uint32_t)align)
            );

            __m256 a0 = _mm256_maskload_ps(A, mask);
            __m256 t0 = _mm256_maskload_ps(TW, mask);
            __m256 w0 = _mm256_maskload_ps(W, mask);
            a0 = _mm256_mul_ps(a0, w0);
            sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
            sum_at0 = _mm256_fmadd_ps(a0, t0, sum_at0);

            A += 8;
            TW += 8;
            W += 8;
            length -= 8 - align;
        }

        const __m256* ptrA = (const __m256*)A;
        const __m256* ptrT = (const __m256*)TW;
        const __m256* ptrW = (const __m256*)W;

        size_t lc = length / 32;
        if (lc){
            do{
                __m256 a0 = ptrA[0];
                __m256 a1 = ptrA[1];
                __m256 a2 = ptrA[2];
                __m256 a3 = ptrA[3];
                a0 = _mm256_mul_ps(a0, ptrW[0]);
                a1 = _mm256_mul_ps(a1, ptrW[1]);
                a2 = _mm256_mul_ps(a2, ptrW[2]);
                a3 = _mm256_mul_ps(a3, ptrW[3]);
                sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
                sum_as1 = _mm256_fmadd_ps(a1, a1, sum_as1);
                sum_as2 = _mm256_fmadd_ps(a2, a2, sum_as2);
                sum_as3 = _mm256_fmadd_ps(a3, a3, sum_as3);
                sum_at0 = _mm256_fmadd_ps(a0, ptrT[0], sum_at0);
                sum_at1 = _mm256_fmadd_ps(a1, ptrT[1], sum_at1);
                sum_at2 = _mm256_fmadd_ps(a2, ptrT[2], sum_at2);
                sum_at3 = _mm256_fmadd_ps(a3, ptrT[3], sum_at3);
                ptrA += 4;
                ptrT += 4;
                ptrW += 4;
            }while (--lc);
            sum_as1 = _mm256_add_ps(sum_as1, sum_as2);
            sum_at1 = _mm256_add_ps(sum_at1, sum_at2);
            sum_as1 = _mm256_add_ps(sum_as1, sum_as3);
            sum_at1 = _mm256_add_ps(sum_at1, sum_at3);
            sum_as0 = _mm256_add_ps(sum_as0, sum_as1);
            sum_at0 = _mm256_add_ps(sum_at0, sum_at1);
        }

        length %= 32;
        while (length >= 8){
            __m256 a0 = ptrA[0];
            a0 = _mm256_mul_ps(a0, ptrW[0]);
            sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
            sum_at0 = _mm256_fmadd_ps(a0, ptrT[0], sum_at0);
            ptrA += 1;
            ptrT += 1;
            ptrW += 1;
            length -= 8;
        }
        if (length){
            PartialWordAccess32_x64_AVX2 access(length);
            __m256 a0 = access.load_f32(ptrA);
            __m256 t0 = access.load_f32(ptrT);
            __m256 w0 = access.load_f32(ptrW);
            a0 = _mm256_mul_ps(a0, w0);
            sum_as0 = _mm256_fmadd_ps(a0, a0, sum_as0);
            sum_at0 = _mm256_fmadd_ps(a0, t0, sum_at0);
        }

        sum_A2 = _mm256_add_ps(sum_A2, sum_as0);
        sum_AT = _mm256_add_ps(sum_AT, sum_at0);
    }
};


float compute_scale_min8_x86_FMA3(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2_min8_x86_FMA3>(width, height, A, T);
}
float compute_scale_min8_x86_FMA3(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,
    float const* const* W2
){
    return compute_scale<SumATA2_min8_x86_FMA3>(width, height, A, TW2, W2);
}



}
}
}
#endif
