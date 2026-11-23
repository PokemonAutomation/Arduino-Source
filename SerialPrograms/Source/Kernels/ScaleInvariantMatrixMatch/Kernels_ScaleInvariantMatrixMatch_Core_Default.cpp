/*  Scale Invariant Matrix Match (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


struct Context_Default{
    using vtype = float;

    //  COMPILER-BUG-MSVC: MSVC 14.52 (VS 2026, the default on the windows-2025-vs2026
    //  runner image) miscompiles the 4-way unrolled do/while loop in
    //  SumATA2::accumulate() / SumError::accumulate() (in
    //  Kernels_ScaleInvariantMatrixMatch_Routines.h) when the trip count is a power of
    //  two: the accumulated sums come out wrong by up to ~10x, failing the SIMM kernel
    //  tests. /fp:strict has no effect (bit-identical wrong values), so this disables
    //  the unroll on the scalar path only; the SIMD contexts are unaffected and keep
    //  it. Re-enable on MSVC once a toolset compiles the loop correctly.
#ifdef _MSC_VER
    static constexpr bool UNROLL = false;
#else
    static constexpr bool UNROLL = true;
#endif

    static PA_FORCE_INLINE vtype vzero(){
        return 0;
    }
    static PA_FORCE_INLINE vtype vset(float a){
        return a;
    }
    static PA_FORCE_INLINE vtype vadd(vtype a, vtype b){
        return a + b;
    }
    static PA_FORCE_INLINE vtype vmul(vtype a, vtype b){
        return a * b;
    }
    static PA_FORCE_INLINE vtype vpma(vtype a, vtype b, vtype c){
        return a * b + c;
    }
    static PA_FORCE_INLINE vtype vpms(vtype a, vtype b, vtype c){
        return a * b - c;
    }
    static PA_FORCE_INLINE float vreduce(vtype a){
        return a;
    }

    static PA_FORCE_INLINE void load2_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        A = *(const float*)ptrA;
        B = *(const float*)ptrB;
    }
    static PA_FORCE_INLINE void load3_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        A = *(const float*)ptrA;
        B = *(const float*)ptrB;
        C = *(const float*)ptrC;
    }
    static PA_FORCE_INLINE void load2_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        A = *(const float*)ptrA;
        B = *(const float*)ptrB;
    }
    static PA_FORCE_INLINE void load3_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        A = *(const float*)ptrA;
        B = *(const float*)ptrB;
        C = *(const float*)ptrC;
    }
};



float compute_scale_Default(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2<Context_Default>>(width, height, A, T);
}
float compute_scale_Default(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_scale<SumATA2<Context_Default>>(width, height, A, TW, W);
}
float compute_error_Default(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
    return compute_error<SumError<Context_Default>>(width, height, scale, A, T);
}
float compute_error_Default(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_error<SumError<Context_Default>>(width, height, scale, A, TW, W);
}






}
}
}
