/*  Scale Invariant Matrix Match (x86 FMA3)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


struct Context_x86_AVX2{
    using vtype = __m256;

    static PA_FORCE_INLINE vtype vzero(){
        return _mm256_setzero_ps();
    }
    static PA_FORCE_INLINE vtype vset(float a){
        return _mm256_set1_ps(a);
    }
    static PA_FORCE_INLINE vtype vadd(vtype a, vtype b){
        return _mm256_add_ps(a, b);
    }
    static PA_FORCE_INLINE vtype vmul(vtype a, vtype b){
        return _mm256_mul_ps(a, b);
    }
    static PA_FORCE_INLINE vtype vpma(vtype a, vtype b, vtype c){
        return _mm256_fmadd_ps(a, b, c);
    }
    static PA_FORCE_INLINE vtype vpms(vtype a, vtype b, vtype c){
        return _mm256_fmsub_ps(a, b, c);
    }
    static PA_FORCE_INLINE float vreduce(vtype a){
        return reduce32_x64_AVX(a);
    }

    static PA_FORCE_INLINE void load2_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        __m256i mask = _mm256_cmpgt_epi32(
            _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8),
            _mm256_set1_epi32((uint32_t)length)
        );
        A = _mm256_maskload_ps((const float*)ptrA, mask);
        B = _mm256_maskload_ps((const float*)ptrB, mask);
    }
    static PA_FORCE_INLINE void load3_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        __m256i mask = _mm256_cmpgt_epi32(
            _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8),
            _mm256_set1_epi32((uint32_t)length)
        );
        A = _mm256_maskload_ps((const float*)ptrA, mask);
        B = _mm256_maskload_ps((const float*)ptrB, mask);
        C = _mm256_maskload_ps((const float*)ptrC, mask);
    }
    static PA_FORCE_INLINE void load2_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        PartialWordAccess32_x64_AVX2 access(length);
        A = access.load_f32(ptrA);
        B = access.load_f32(ptrB);
    }
    static PA_FORCE_INLINE void load3_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        PartialWordAccess32_x64_AVX2 access(length);
        A = access.load_f32(ptrA);
        B = access.load_f32(ptrB);
        C = access.load_f32(ptrC);
    }
};



float compute_scale_min8_x86_AVX2(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2<Context_x86_AVX2>>(width, height, A, T);
}
float compute_scale_min8_x86_AVX2(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_scale<SumATA2<Context_x86_AVX2>>(width, height, A, TW, W);
}
float compute_error_min8_x86_AVX2(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
    return compute_error<SumError<Context_x86_AVX2>>(width, height, scale, A, T);
}
float compute_error_min8_x86_AVX2(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_error<SumError<Context_x86_AVX2>>(width, height, scale, A, TW, W);
}



}
}
}
#endif
