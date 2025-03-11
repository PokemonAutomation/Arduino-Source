/*  Scale Invariant Matrix Match (x86 SSE)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <immintrin.h>
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


struct Context_x86_SSE41{
    using vtype = __m128;

    static PA_FORCE_INLINE vtype vzero(){
        return _mm_setzero_ps();
    }
    static PA_FORCE_INLINE vtype vset(float a){
        return _mm_set1_ps(a);
    }
    static PA_FORCE_INLINE vtype vadd(vtype a, vtype b){
        return _mm_add_ps(a, b);
    }
    static PA_FORCE_INLINE vtype vmul(vtype a, vtype b){
        return _mm_mul_ps(a, b);
    }
    static PA_FORCE_INLINE vtype vpma(vtype a, vtype b, vtype c){
        return _mm_add_ps(_mm_mul_ps(a, b), c);
    }
    static PA_FORCE_INLINE vtype vpms(vtype a, vtype b, vtype c){
        return _mm_sub_ps(_mm_mul_ps(a, b), c);
    }
    static PA_FORCE_INLINE float vreduce(vtype a){
        return reduce32_x64_SSE(a);
    }

    static PA_FORCE_INLINE void load2_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        PartialWordAccess_x64_SSE41 access(16 - length * 4);
        A = access.load_f32_no_read_before_ptr((const float*)ptrA + length);
        B = access.load_f32_no_read_before_ptr((const float*)ptrB + length);
    }
    static PA_FORCE_INLINE void load3_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        PartialWordAccess_x64_SSE41 access(16 - length * 4);
        A = access.load_f32_no_read_before_ptr((const float*)ptrA + length);
        B = access.load_f32_no_read_before_ptr((const float*)ptrB + length);
        C = access.load_f32_no_read_before_ptr((const float*)ptrC + length);
    }
    static PA_FORCE_INLINE void load2_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        PartialWordAccess_x64_SSE41 access(length * 4);
        A = access.load_f32_no_read_past_end(ptrA);
        B = access.load_f32_no_read_past_end(ptrB);
    }
    static PA_FORCE_INLINE void load3_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        PartialWordAccess_x64_SSE41 access(length * 4);
        A = access.load_f32_no_read_past_end(ptrA);
        B = access.load_f32_no_read_past_end(ptrB);
        C = access.load_f32_no_read_past_end(ptrC);
    }
};



float compute_scale_min4_x86_SSE(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2<Context_x86_SSE41>>(width, height, A, T);
}
float compute_scale_min4_x86_SSE(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_scale<SumATA2<Context_x86_SSE41>>(width, height, A, TW, W);
}
float compute_error_min4_x86_SSE(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
    return compute_error<SumError<Context_x86_SSE41>>(width, height, scale, A, T);
}
float compute_error_min4_x86_SSE(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_error<SumError<Context_x86_SSE41>>(width, height, scale, A, TW, W);
}



}
}
}
#endif
