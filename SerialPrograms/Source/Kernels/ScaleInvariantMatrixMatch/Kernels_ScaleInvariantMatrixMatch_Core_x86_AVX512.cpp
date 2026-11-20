/*  Scale Invariant Matrix Match (x86 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <cmath>
#include <cstdio>
#include <immintrin.h>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Containers/AlignedMalloc.h"
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

//
//  Masked-load probe
//
//  MSVC has miscompiled AVX-512 masked memory operations before (see the
//  COMPILER-BUG-MSVC workaround in Kernels_ImageScaleBrightness_x64_AVX512.cpp),
//  and this kernel is the only one in the project that uses the aligned
//  masked load (_mm512_maskz_load_ps). This probe pins down that specific
//  failure mode - the masked load being emitted without its mask - instead
//  of relying on the normal tests' guard-page crash or value mismatch.
//
//  Part 1 loads sentinel data through each 512-bit masked-load form with a
//  partial mask and reports whether the masked-out lanes come back zeroed
//  (report-only: the kernel is free to use either form).
//
//  Part 2 runs the real kernel on a single row bracketed by sentinel
//  values, so that a mask-dropping compiler produces an exactly
//  predictable (wrong) result: it sums the sentinels along with the row,
//  which is the same as the Default implementation run on the extended
//  row [pre | row | post]. The test fails explicitly if it observes that
//  value, and distinguishes it from both the correct result and any other
//  failure mode.
//
class MaskLoadProbeTest : public UnitTest{
public:
    MaskLoadProbeTest()
        : UnitTest("Kernels::ScaleInvariantMatrixMatch - x86_AVX512 (masked-load probe)")
    {
        //  Declared larger than the matrix tests (16 MB) so the runner -
        //  which starts with the largest test - runs this probe first: if
        //  the kernel is miscompiled, the matrix tests can crash the
        //  process on the guard page, and this verdict has to reach the
        //  log before that happens.
        m_memory = 32 * 1024 * 1024;
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        scope.throw_if_cancelled();

        //
        //  Part 1: one sentinel load through each intrinsic form.
        //
        void* raw = aligned_malloc(256, 64);
        float* buf = (float*)raw;
        for (int i = 0; i < 16; i++)
            buf[i] = 1000.0f * (float)(i + 1);

        const __mmask16 mask_lo5 = 0x001F;  //  enable only lanes 0..4
        float lanes[16];
        auto report = [&](const char* name, __m512 v) -> void{
            _mm512_storeu_ps(lanes, v);
            unsigned int leaked = 0;
            for (int i = 5; i < 16; i++)
                if (lanes[i] != 0.0f)
                    leaked |= (1u << i);
            char b[96];
            if (leaked)
                std::snprintf(b, sizeof(b), "mask NOT honored - masked-out lanes hold sentinel data (lane mask 0x%x)", leaked);
            else
                std::snprintf(b, sizeof(b), "mask honored");
            logger.log(std::string("    probe: ") + name + b);
        };
        report("_mm512_maskz_load_ps   (aligned):   ", _mm512_maskz_load_ps(mask_lo5, buf));
        report("_mm512_maskz_loadu_ps (unaligned): ", _mm512_maskz_loadu_ps(mask_lo5, buf));
        aligned_free(raw);

        //
        //  Part 2: the kernel itself, on a row with sentinels around it.
        //
        //  A single 27-float row placed 3 floats past a 64-byte boundary:
        //  the back load masks out the 3 floats before the row, and the
        //  front load (14 valid lanes) masks out the 2 floats after the
        //  row. If the mask is dropped, the result matches the Default
        //  implementation on the 32-element extended row [pre | row |
        //  post0..1].
        //
        const size_t W = 27, PRE = 3, POST = 16;
        float* a = (float*)aligned_malloc((PRE + W + POST) * sizeof(float), 64);
        float* t = (float*)aligned_malloc((PRE + W + POST) * sizeof(float), 64);
        for (size_t i = 0; i < PRE + W + POST; i++){
            a[i] = 0.01f * (float)(i + 1);
            t[i] = -0.02f * (float)(i + 1);
        }
        for (size_t i = 0; i < PRE; i++){
            a[i] = 250.0f + (float)i;
            t[i] = 450.0f + (float)i;
        }
        for (size_t i = 0; i < POST; i++){
            a[PRE + W + i] = 350.0f + (float)i;
            t[PRE + W + i] = 550.0f + (float)i;
        }
        const float* const rowsA[1] = { a + PRE };
        const float* const rowsT[1] = { t + PRE };
        const float* const extA[1] = { a };
        const float* const extT[1] = { t };

        const float correct = compute_scale_Default(W, 1, rowsA, rowsT);
        const float polluted = compute_scale_Default(W + PRE + 2, 1, extA, extT);
        const float observed = compute_scale_min16_x86_AVX512(W, 1, rowsA, rowsT);
        aligned_free(a);
        aligned_free(t);

        auto fmt = [](float f) -> std::string{
            char b[32];
            std::snprintf(b, sizeof(b), "%.9g", (double)f);
            return b;
        };
        auto close = [](float x, float y) -> bool{
            double m = std::fmax(1.0, std::fmax(std::fabs((double)x), std::fabs((double)y)));
            return std::fabs((double)x - (double)y) <= 1e-4 * m;
        };

        if (close(observed, correct))
            return true;
        logger.log("    probe: observed=" + fmt(observed) +
            ", expected=" + fmt(correct) + ", polluted-prediction=" + fmt(polluted));
        if (close(observed, polluted))
            return "mask NOT honored: the kernel's result matches the sentinel-polluted prediction; the AVX-512 masked load was compiled without its mask";
        return "unknown failure mode: observed=" + fmt(observed) +
            ", expected=" + fmt(correct) + ", polluted-prediction=" + fmt(polluted);
    }
};

void add_tests_x86_AVX512(UnitTestDatabase& database){
    database.add<MaskLoadProbeTest>();
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
