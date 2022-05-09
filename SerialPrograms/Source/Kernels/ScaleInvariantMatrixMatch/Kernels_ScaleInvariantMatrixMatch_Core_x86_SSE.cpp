/*  Scale Invariant Matrix Match (x86 SSE)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_08_Nehalem

#include <immintrin.h>
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



struct SumATA2_min16_x86_SSE{
    using vtype = __m128;

    __m128 sum_AT = _mm_setzero_ps();
    __m128 sum_A2 = _mm_setzero_ps();

    PA_FORCE_INLINE float scale() const{
        return reduce32_x64_SSE(sum_AT) / reduce32_x64_SSE(sum_A2);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        __m128 sum_as0 = _mm_setzero_ps();
        __m128 sum_as1 = _mm_setzero_ps();
        __m128 sum_at0 = _mm_setzero_ps();
        __m128 sum_at1 = _mm_setzero_ps();

        size_t align = (size_t)T % 16;
        if (align){
            align /= sizeof(float);

            PartialWordAccess_x64_SSE41 access(16 - align * 4);

            __m128 a0 = access.load_f32_no_read_before_ptr(A);
            __m128 t0 = access.load_f32_no_read_before_ptr(T);
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, t0));

            A += 4 - align;
            T += 4 - align;
            length -= 4 - align;
        }

        const __m128* ptrA = (const __m128*)A;
        const __m128* ptrT = (const __m128*)T;

        size_t lc = length / 8;
        do{
            __m128 a0 = ptrA[0];
            __m128 a1 = ptrA[1];
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_as1 = _mm_add_ps(sum_as1, _mm_mul_ps(a1, a1));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, ptrT[0]));
            sum_at1 = _mm_add_ps(sum_at1, _mm_mul_ps(a1, ptrT[1]));
            ptrA += 2;
            ptrT += 2;
        }while (--lc);
        sum_as0 = _mm_add_ps(sum_as0, sum_as1);
        sum_at0 = _mm_add_ps(sum_at0, sum_at1);

        length %= 8;
        if (length >= 4){
            __m128 a0 = ptrA[0];
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, ptrT[0]));
            ptrA += 1;
            ptrT += 1;
            length -= 4;
        }
        if (length){
            PartialWordAccess_x64_SSE41 access(length * 4);
            __m128 a0 = access.load_f32_no_read_past_end(ptrA);
            __m128 t0 = access.load_f32_no_read_past_end(ptrT);
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, t0));
        }

        sum_A2 = _mm_add_ps(sum_A2, sum_as0);
        sum_AT = _mm_add_ps(sum_AT, sum_at0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW, const float* W){
        __m128 sum_as0 = _mm_setzero_ps();
        __m128 sum_as1 = _mm_setzero_ps();
        __m128 sum_at0 = _mm_setzero_ps();
        __m128 sum_at1 = _mm_setzero_ps();

        size_t align = (size_t)TW % 16;
        if (align){
            align /= sizeof(float);

            PartialWordAccess_x64_SSE41 access(16 - align * 4);

            __m128 a0 = access.load_f32_no_read_before_ptr(A);
            __m128 t0 = access.load_f32_no_read_before_ptr(TW);
            __m128 w0 = access.load_f32_no_read_before_ptr(W);
            a0 = _mm_mul_ps(a0, w0);
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, t0));

            A += 4 - align;
            TW += 4 - align;
            W += 4 - align;
            length -= 4 - align;
        }

        const __m128* ptrA = (const __m128*)A;
        const __m128* ptrT = (const __m128*)TW;
        const __m128* ptrW = (const __m128*)W;

        size_t lc = length / 8;
        do{
            __m128 a0 = ptrA[0];
            __m128 a1 = ptrA[1];
            a0 = _mm_mul_ps(a0, ptrW[0]);
            a1 = _mm_mul_ps(a1, ptrW[1]);
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_as1 = _mm_add_ps(sum_as1, _mm_mul_ps(a1, a1));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, ptrT[0]));
            sum_at1 = _mm_add_ps(sum_at1, _mm_mul_ps(a1, ptrT[1]));
            ptrA += 2;
            ptrT += 2;
            ptrW += 2;
        }while (--lc);
        sum_as0 = _mm_add_ps(sum_as0, sum_as1);
        sum_at0 = _mm_add_ps(sum_at0, sum_at1);

        length %= 8;
        if (length >= 4){
            __m128 a0 = ptrA[0];
            a0 = _mm_mul_ps(a0, ptrW[0]);
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, ptrT[0]));
            ptrA += 1;
            ptrT += 1;
            ptrW += 1;
            length -= 4;
        }
        if (length){
            PartialWordAccess_x64_SSE41 access(length * 4);
            __m128 a0 = access.load_f32_no_read_past_end(ptrA);
            __m128 t0 = access.load_f32_no_read_past_end(ptrT);
            __m128 w0 = access.load_f32_no_read_past_end(ptrW);
            a0 = _mm_mul_ps(a0, w0);
            sum_as0 = _mm_add_ps(sum_as0, _mm_mul_ps(a0, a0));
            sum_at0 = _mm_add_ps(sum_at0, _mm_mul_ps(a0, t0));
        }

        sum_A2 = _mm_add_ps(sum_A2, sum_as0);
        sum_AT = _mm_add_ps(sum_AT, sum_at0);
    }
};
struct SumError_min16_x86_SSE{
    using vtype = __m128;

    __m128 scale;
    __m128 sum = _mm_setzero_ps();

    PA_FORCE_INLINE SumError_min16_x86_SSE(float p_scale)
        : scale(_mm_set1_ps(p_scale))
    {}

    PA_FORCE_INLINE float sum_sqr() const{
        return reduce32_x64_SSE(sum);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        __m128 sum0 = _mm_setzero_ps();
        __m128 sum1 = _mm_setzero_ps();

        size_t align = (size_t)T % 16;
        if (align){
            align /= sizeof(float);

            PartialWordAccess_x64_SSE41 access(16 - align * 4);

            __m128 a0 = access.load_f32_no_read_before_ptr(A);
            __m128 t0 = access.load_f32_no_read_before_ptr(T);
            a0 = _mm_sub_ps(_mm_mul_ps(scale, a0), t0);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));

            A += 4 - align;
            T += 4 - align;
            length -= 4 - align;
        }

        const __m128* ptrA = (const __m128*)A;
        const __m128* ptrT = (const __m128*)T;

        size_t lc = length / 8;
        do{
            __m128 a0 = _mm_sub_ps(_mm_mul_ps(scale, ptrA[0]), ptrT[0]);
            __m128 a1 = _mm_sub_ps(_mm_mul_ps(scale, ptrA[1]), ptrT[1]);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));
            sum1 = _mm_add_ps(sum1, _mm_mul_ps(a1, a1));
            ptrA += 2;
            ptrT += 2;
        }while (--lc);
        sum0 = _mm_add_ps(sum0, sum1);

        length %= 8;
        if (length >= 4){
            __m128 a0 = _mm_sub_ps(_mm_mul_ps(scale, ptrA[0]), ptrT[0]);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));
            ptrA += 1;
            ptrT += 1;
            length -= 4;
        }
        if (length){
            PartialWordAccess_x64_SSE41 access(length * 4);
            __m128 a0 = access.load_f32_no_read_past_end(ptrA);
            __m128 t0 = access.load_f32_no_read_past_end(ptrT);
            a0 = _mm_sub_ps(_mm_mul_ps(scale, a0), t0);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));
        }

        sum = _mm_add_ps(sum, sum0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW, const float* W){
        __m128 sum0 = _mm_setzero_ps();
        __m128 sum1 = _mm_setzero_ps();

        size_t align = (size_t)TW % 16;
        if (align){
            align /= sizeof(float);

            PartialWordAccess_x64_SSE41 access(16 - align * 4);

            __m128 a0 = access.load_f32_no_read_before_ptr(A);
            __m128 t0 = access.load_f32_no_read_before_ptr(TW);
            __m128 w0 = access.load_f32_no_read_before_ptr(W);
            a0 = _mm_mul_ps(scale, a0);
            a0 = _mm_sub_ps(_mm_mul_ps(a0, w0), t0);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));

            A += 4 - align;
            TW += 4 - align;
            W += 4 - align;
            length -= 4 - align;
        }

        const __m128* ptrA = (const __m128*)A;
        const __m128* ptrT = (const __m128*)TW;
        const __m128* ptrW = (const __m128*)W;

        size_t lc = length / 8;
        do{
            __m128 a0 = _mm_mul_ps(scale, ptrA[0]);
            __m128 a1 = _mm_mul_ps(scale, ptrA[1]);
            a0 = _mm_sub_ps(_mm_mul_ps(a0, ptrW[0]), ptrT[0]);
            a1 = _mm_sub_ps(_mm_mul_ps(a1, ptrW[1]), ptrT[1]);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));
            sum1 = _mm_add_ps(sum1, _mm_mul_ps(a1, a1));
            ptrA += 2;
            ptrT += 2;
            ptrW += 2;
        }while (--lc);
        sum0 = _mm_add_ps(sum0, sum1);

        length %= 8;
        if (length >= 4){
            __m128 a0 = _mm_mul_ps(scale, ptrA[0]);
            a0 = _mm_sub_ps(_mm_mul_ps(a0, ptrW[0]), ptrT[0]);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));
            ptrA += 1;
            ptrT += 1;
            ptrW += 1;
            length -= 4;
        }
        if (length){
            PartialWordAccess_x64_SSE41 access(length * 4);
            __m128 a0 = access.load_f32_no_read_past_end(ptrA);
            __m128 t0 = access.load_f32_no_read_past_end(ptrT);
            __m128 w0 = access.load_f32_no_read_past_end(ptrW);
            a0 = _mm_mul_ps(scale, a0);
            a0 = _mm_sub_ps(_mm_mul_ps(a0, w0), t0);
            sum0 = _mm_add_ps(sum0, _mm_mul_ps(a0, a0));
        }

        sum = _mm_add_ps(sum, sum0);
    }
};


float compute_scale_min16_x86_SSE(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2_min16_x86_SSE>(width, height, A, T);
}
float compute_scale_min16_x86_SSE(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_scale<SumATA2_min16_x86_SSE>(width, height, A, TW, W);
}
float compute_error_min16_x86_SSE(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
    return compute_error<SumError_min16_x86_SSE>(width, height, scale, A, T);
}
float compute_error_min16_x86_SSE(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_error<SumError_min16_x86_SSE>(width, height, scale, A, TW, W);
}



}
}
}
#endif
