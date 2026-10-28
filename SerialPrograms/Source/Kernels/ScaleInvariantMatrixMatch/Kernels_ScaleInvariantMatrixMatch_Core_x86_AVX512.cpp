/*  Scale Invariant Matrix Match (x86 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"
#include "Kernels_ScaleInvariantMatrixMatch_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


struct Context_x86_AVX512{
    using vtype = __m512;

    static PA_FORCE_INLINE vtype vzero(){
        return _mm512_setzero_ps();
    }
    static PA_FORCE_INLINE vtype vset(float a){
        return _mm512_set1_ps(a);
    }
    static PA_FORCE_INLINE vtype vadd(vtype a, vtype b){
        return _mm512_add_ps(a, b);
    }
    static PA_FORCE_INLINE vtype vmul(vtype a, vtype b){
        return _mm512_mul_ps(a, b);
    }
    static PA_FORCE_INLINE vtype vpma(vtype a, vtype b, vtype c){
        return _mm512_fmadd_ps(a, b, c);
    }
    static PA_FORCE_INLINE vtype vpms(vtype a, vtype b, vtype c){
        return _mm512_fmsub_ps(a, b, c);
    }
    static PA_FORCE_INLINE float vreduce(vtype a){
        return _mm512_reduce_add_ps(a);
    }

    static PA_FORCE_INLINE void load2_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        __mmask16 mask = ~(((uint16_t)1 << length) - 1);
        A = _mm512_maskz_load_ps(mask, ptrA);
        B = _mm512_maskz_load_ps(mask, ptrB);
    }
    static PA_FORCE_INLINE void load3_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        __mmask16 mask = ~(((uint16_t)1 << length) - 1);
        A = _mm512_maskz_load_ps(mask, ptrA);
        B = _mm512_maskz_load_ps(mask, ptrB);
        C = _mm512_maskz_load_ps(mask, ptrC);
    }
    static PA_FORCE_INLINE void load2_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        __mmask16 mask = ((uint16_t)1 << length) - 1;
        A = _mm512_maskz_load_ps(mask, ptrA);
        B = _mm512_maskz_load_ps(mask, ptrB);
    }
    static PA_FORCE_INLINE void load3_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        __mmask16 mask = ((uint16_t)1 << length) - 1;
        A = _mm512_maskz_load_ps(mask, ptrA);
        B = _mm512_maskz_load_ps(mask, ptrB);
        C = _mm512_maskz_load_ps(mask, ptrC);
    }
};



float compute_scale_min16_x86_AVX512(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2<Context_x86_AVX512>>(width, height, A, T);
}
float compute_scale_min16_x86_AVX512(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_scale<SumATA2<Context_x86_AVX512>>(width, height, A, TW, W);
}
float compute_error_min16_x86_AVX512(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
    return compute_error<SumError<Context_x86_AVX512>>(width, height, scale, A, T);
}
float compute_error_min16_x86_AVX512(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_error<SumError<Context_x86_AVX512>>(width, height, scale, A, TW, W);
}



float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_Default(size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_Default(size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);

class Test_x86_AVX512 : public Test{
public:
    Test_x86_AVX512(size_t width, size_t height, bool use_weights)
        : Test(
            "x86_AVX512",
            width, height,
            use_weights,
            compute_scale_Default,
            compute_scale_min16_x86_AVX512,
            compute_scale_Default,
            compute_scale_min16_x86_AVX512,
            compute_error_Default,
            compute_error_min16_x86_AVX512,
            compute_error_Default,
            compute_error_min16_x86_AVX512
        )
    {}
};
void add_tests_x86_AVX512(UnitTestDatabase& database){
    //  The kernel requires a minimum row width (the partial-row loads span up to
    //  a full vector); sizes below that are not registered for this ISA.
    for (size_t i = 0; i < sizeof(kTestSizes) / sizeof(kTestSizes[0]); i++){
        if (kTestSizes[i][0] < 16){
            continue;
        }
        database.add<Test_x86_AVX512>(kTestSizes[i][0], kTestSizes[i][1], false);
        database.add<Test_x86_AVX512>(kTestSizes[i][0], kTestSizes[i][1], true);
    }
}



}
}
}
#endif
