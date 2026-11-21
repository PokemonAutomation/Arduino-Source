/*  Scale Invariant Matrix Match MSVC probe
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Temporary diagnostic test for the CI-only failure
 *
 *      Kernels::ScaleInvariantMatrixMatch - x86_SSE(w=512, h=513, weights)
 *      Message: compute_scale: default=... isa=... (out of tolerance)
 *
 *  On this branch every other test is commented out (see Kernels_Tests.cpp
 *  and ComputerPrograms_UnitTestRunner.cpp), so this is the only test the
 *  CI suite runs. It reproduces the data setup of the failing test
 *  exactly, and logs the intermediate steps so a failing run's log
 *  localizes where the Default and SSE results diverge:
 *
 *  - the compiler version of the binary;
 *  - a data dump (sums, and the first values with bit patterns), to
 *    compare the std::random stream across platforms and across CI
 *    runner images;
 *  - the alignment of the row pointers;
 *  - every row's ref (double) / def (Default kernel) / sse (SSE
 *    kernel), each computed by one-row calls of the real kernels;
 *  - on x86, the SSE kernel's row-reduced sum_at / sum_as lanes for the
 *    first rows, the diverging rows and the last row: lane j holds the
 *    contributions of the elements whose index is j mod 16, so a broken
 *    lane localizes the corruption to a quarter of the row stride;
 *  - the full-matrix results (unweighted and weighted), exactly like the
 *    real test, as the verdict.
 *
 *  The per-row and full-matrix Default/SSE values come from the real
 *  kernel functions. Only the lane dump uses a local copy of the SSE
 *  main loop (the real kernel does not expose its intermediate
 *  accumulators); the copy covers exactly the code that runs for this
 *  data: the rows are 64-byte aligned, so the partial-word-access
 *  back/front loads are dead, and the width is a multiple of 16, so the
 *  tail loop is empty.
 */

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"

#ifdef PA_AutoDispatch_x64_08_Nehalem
#include <immintrin.h>
#endif

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


//  The kernels this probe compares: defined in the per-ISA source files,
//  the Default one on every platform.
float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
#ifdef PA_AutoDispatch_x64_08_Nehalem
float compute_scale_min4_x86_SSE(size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min4_x86_SSE(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
#endif


//  The size of the failing CI test: 512 x 513, with weights.
static constexpr size_t kWidth = 512;
static constexpr size_t kHeight = 513;
//  Same head padding as the real tests.
static constexpr size_t HEAD_PAD = 16;

//
//  Same tolerance as the real tests: a result passes if it is within
//  4096 ULPs of the other, or within 1e-4 of the natural magnitude of
//  the accumulated sum.
//
static constexpr uint64_t MAX_ULP_DIFF = 4096;
static constexpr double ABS_TOLERANCE = 1e-4;

static uint64_t ulp_distance(float a, float b){
    //  Monotonic mapping of the float bits to a signed order.
    auto U = [](float f) -> uint64_t{
        int32_t i = std::bit_cast<int32_t>(f);
        if (i >= 0)
            return (uint64_t)(uint32_t)i + 0x80000000ull;
        return (uint64_t)(uint32_t)(-i);
    };
    uint64_t x = U(a);
    uint64_t y = U(b);
    return x > y ? x - y : y - x;
}
static bool within_tolerance(float x, float y, double magnitude){
    if (x == y)
        return true;
    if (ulp_distance(x, y) <= MAX_ULP_DIFF)
        return true;
    return std::fabs((double)x - (double)y) <= ABS_TOLERANCE * std::fmax(1.0, magnitude);
}

#ifdef PA_AutoDispatch_x64_08_Nehalem
struct ProbeLanes{
    float at[4];
    float as[4];
};
//  The row-reduced SSE accumulators of one row, i.e. the lanes of
//  sum_at / sum_as after the per-row quarter reduction. A copy of the
//  main loop of SumATA2<Context_x86_SSE41>::accumulate
//  (Kernels_ScaleInvariantMatrixMatch_Routines.h) with the same loop
//  body and the same reduction; see the file header for the reasons
//  this copy, rather than the real kernel, is used for the lane dump.
//  The row must be 16-byte aligned and the length a multiple of 16.
static ProbeLanes sse_row_lanes(const float* A, const float* T, size_t length){
    __m128 sum_as0 = _mm_setzero_ps();
    __m128 sum_as1 = _mm_setzero_ps();
    __m128 sum_as2 = _mm_setzero_ps();
    __m128 sum_as3 = _mm_setzero_ps();
    __m128 sum_at0 = _mm_setzero_ps();
    __m128 sum_at1 = _mm_setzero_ps();
    __m128 sum_at2 = _mm_setzero_ps();
    __m128 sum_at3 = _mm_setzero_ps();

    const __m128* ptrA = (const __m128*)A;
    const __m128* ptrT = (const __m128*)T;

    size_t lc = length / (4 * 4);
    if (lc){
        do{
            __m128 a0 = ptrA[0];
            __m128 a1 = ptrA[1];
            __m128 a2 = ptrA[2];
            __m128 a3 = ptrA[3];
            sum_as0 = _mm_add_ps(_mm_mul_ps(a0, a0), sum_as0);
            sum_as1 = _mm_add_ps(_mm_mul_ps(a1, a1), sum_as1);
            sum_as2 = _mm_add_ps(_mm_mul_ps(a2, a2), sum_as2);
            sum_as3 = _mm_add_ps(_mm_mul_ps(a3, a3), sum_as3);
            sum_at0 = _mm_add_ps(_mm_mul_ps(a0, ptrT[0]), sum_at0);
            sum_at1 = _mm_add_ps(_mm_mul_ps(a1, ptrT[1]), sum_at1);
            sum_at2 = _mm_add_ps(_mm_mul_ps(a2, ptrT[2]), sum_at2);
            sum_at3 = _mm_add_ps(_mm_mul_ps(a3, ptrT[3]), sum_at3);
            ptrA += 4;
            ptrT += 4;
        }while (--lc);
        sum_as0 = _mm_add_ps(sum_as0, sum_as1);
        sum_at0 = _mm_add_ps(sum_at0, sum_at1);
        sum_as2 = _mm_add_ps(sum_as2, sum_as3);
        sum_at2 = _mm_add_ps(sum_at2, sum_at3);
        sum_as0 = _mm_add_ps(sum_as0, sum_as2);
        sum_at0 = _mm_add_ps(sum_at0, sum_at2);
    }

    ProbeLanes l;
    union { __m128 v; float f[4]; } uat = { sum_at0 };
    union { __m128 v; float f[4]; } uas = { sum_as0 };
    for (int i = 0; i < 4; i++){
        l.at[i] = uat.f[i];
        l.as[i] = uas.f[i];
    }
    return l;
}
#endif



class Probe : public UnitTest{
public:
    Probe()
        : UnitTest(
            "Kernels::ScaleInvariantMatrixMatch - MsvcProbe(w=512, h=513, weights)")
    {
        //  The probe allocates A, T, W and TW, 4 matrices of
        //  kWidth x kHeight floats (plus a guard page each).
        m_memory = (uint64_t)kWidth * (uint64_t)kHeight * 4 * 4;
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        scope.throw_if_cancelled();
        const size_t n = kWidth * kHeight;

        auto say = [&](const char* s){
            logger.log(std::string("probe: ") + s);
        };
        char buf[1024];

#ifdef _MSC_VER
        std::snprintf(buf, sizeof(buf),
            "compiler MSVC _MSC_VER=%d _MSC_FULL_VER=%d _MSC_BUILD=%d",
            _MSC_VER, _MSC_FULL_VER, _MSC_BUILD
        );
#else
        std::snprintf(buf, sizeof(buf), "compiler non-MSVC");
#endif
        say(buf);
#if defined(_M_X64) || defined(__x86_64__)
        say("arch x86_64");
#elif defined(__aarch64__) || defined(_M_ARM64)
        say("arch aarch64");
#else
        say("arch unknown");
#endif

        //
        //  The data of the failing test: the weighted (512, 513)
        //  instance. Same seed, same distributions, same order of
        //  draws, same buffers and head padding as Test::run, so on a
        //  given stdlib the values are bit-identical to the real
        //  test's.
        //
        GuardPageBuffer<float> A(n + HEAD_PAD);
        GuardPageBuffer<float> T(n + HEAD_PAD);
        GuardPageBuffer<float> W(n + HEAD_PAD);
        GuardPageBuffer<float> TW(n + HEAD_PAD);
        AlignedVector<const float*> rowsA(kHeight);
        AlignedVector<const float*> rowsT(kHeight);
        AlignedVector<const float*> rowsW(kHeight);
        AlignedVector<const float*> rowsTW(kHeight);

        std::mt19937 generator((unsigned int)(kWidth * 1000003u + kHeight * 7919u + 2654435761u));
        std::uniform_real_distribution<float> data_distribution(-1.0f, 1.0f);
        std::uniform_real_distribution<float> weight_distribution(0.0f, 1.0f);
        double sumA = 0, sumT = 0, sumW = 0;
        for (size_t c = HEAD_PAD; c < HEAD_PAD + n; c++){
            A[c] = data_distribution(generator);
            T[c] = data_distribution(generator);
            W[c] = weight_distribution(generator);
            TW[c] = T[c] * W[c];
            sumA += A[c];
            sumT += T[c];
            sumW += W[c];
        }
        for (size_t r = 0; r < kHeight; r++){
            rowsA[r] = A.data() + HEAD_PAD + r * kWidth;
            rowsT[r] = T.data() + HEAD_PAD + r * kWidth;
            rowsW[r] = W.data() + HEAD_PAD + r * kWidth;
            rowsTW[r] = TW.data() + HEAD_PAD + r * kWidth;
        }

        std::snprintf(buf, sizeof(buf),
            "data sum(A)=%.9g sum(T)=%.9g sum(W)=%.9g over %zu elements",
            sumA, sumT, sumW, n
        );
        say(buf);
        const float* pA = rowsA[0];
        const float* pT = rowsT[0];
        for (int pass = 0; pass < 2; pass++){
            const float* p = pass ? pT : pA;
            std::snprintf(buf, sizeof(buf),
                "data %s[0..3]: 0x%08X (%.9g) 0x%08X (%.9g) 0x%08X (%.9g) 0x%08X (%.9g)",
                pass ? "T" : "A",
                std::bit_cast<uint32_t>(p[0]), (double)p[0],
                std::bit_cast<uint32_t>(p[1]), (double)p[1],
                std::bit_cast<uint32_t>(p[2]), (double)p[2],
                std::bit_cast<uint32_t>(p[3]), (double)p[3]
            );
            say(buf);
        }
        std::snprintf(buf, sizeof(buf),
            "rows A[0]=0x%llX (mod 16 = %zu, mod 64 = %zu), T[0]=0x%llX (mod 16 = %zu, mod 64 = %zu)",
            (unsigned long long)(uintptr_t)rowsA[0], (size_t)(uintptr_t)rowsA[0] % 16, (size_t)(uintptr_t)rowsA[0] % 64,
            (unsigned long long)(uintptr_t)rowsT[0], (size_t)(uintptr_t)rowsT[0] % 16, (size_t)(uintptr_t)rowsT[0] % 64
        );
        say(buf);

        //
        //  Every row, through the real kernels (one-row calls): the
        //  first diverging row, if any.
        //
#ifdef PA_AutoDispatch_x64_08_Nehalem
        say("rows (ref = double, def = Default kernel, sse = SSE kernel):");
#else
        say("rows (ref = double, def = Default kernel):");
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
        size_t diverging_rows = 0;
        size_t first_diverging = (size_t)-1;
        size_t lane_dumps = 0;
        const bool lanes_possible = ((size_t)(uintptr_t)rowsT[0] % 16) == 0 && (kWidth % 16) == 0;
#endif
        for (size_t r = 0; r < kHeight; r++){
            double row_at = 0, row_as = 0;
            for (size_t c = 0; c < kWidth; c++){
                row_at += (double)rowsA[r][c] * (double)rowsT[r][c];
                row_as += (double)rowsA[r][c] * (double)rowsA[r][c];
            }
            const float ref_row = (float)(row_at / row_as);
            const float def_row = compute_scale_Default(kWidth, 1, &rowsA[r], &rowsT[r]);
#ifdef PA_AutoDispatch_x64_08_Nehalem
            const float sse_row = compute_scale_min4_x86_SSE(kWidth, 1, &rowsA[r], &rowsT[r]);
            const double mag = std::fmax(std::fabs((double)def_row), std::fabs((double)sse_row));
            const bool ok = within_tolerance(def_row, sse_row, mag);
            if (!ok){
                diverging_rows++;
                if (first_diverging == (size_t)-1)
                    first_diverging = r;
            }
            std::snprintf(buf, sizeof(buf),
                "row %3zu: ref=%.9g def=%.9g sse=%.9g%s",
                r, (double)ref_row, (double)def_row, (double)sse_row, ok ? "" : "  ** DIVERGING **"
            );
            say(buf);
            if (lanes_possible && lane_dumps < 32 &&
                (r < 4 || (!ok && lane_dumps < 16) || r == kHeight - 1)){
                ProbeLanes l = sse_row_lanes(rowsA[r], rowsT[r], kWidth);
                std::snprintf(buf, sizeof(buf),
                    "    sse lanes row %zu: at=[%.9g %.9g %.9g %.9g] as=[%.9g %.9g %.9g %.9g]",
                    r,
                    (double)l.at[0], (double)l.at[1], (double)l.at[2], (double)l.at[3],
                    (double)l.as[0], (double)l.as[1], (double)l.as[2], (double)l.as[3]
                );
                say(buf);
                lane_dumps++;
            }
#else
            std::snprintf(buf, sizeof(buf),
                "row %3zu: ref=%.9g def=%.9g",
                r, (double)ref_row, (double)def_row
            );
            say(buf);
#endif
        }

        //
        //  The full-matrix results, exactly like the real test.
        //
        double mat_at = 0, mat_as = 0, matW_at = 0, matW_as = 0;
        for (size_t r = 0; r < kHeight; r++){
            for (size_t c = 0; c < kWidth; c++){
                mat_at += (double)rowsA[r][c] * (double)rowsT[r][c];
                mat_as += (double)rowsA[r][c] * (double)rowsA[r][c];
                const double aw = (double)((float)rowsA[r][c] * (float)rowsW[r][c]);
                matW_at += aw * (double)rowsTW[r][c];
                matW_as += aw * aw;
            }
        }
        const float ref = (float)(mat_at / mat_as);
        const float refW = (float)(matW_at / matW_as);
        const float def = compute_scale_Default(kWidth, kHeight, rowsA.data(), rowsT.data());
        const float defW = compute_scale_Default(kWidth, kHeight, rowsA.data(), rowsTW.data(), rowsW.data());

#ifdef PA_AutoDispatch_x64_08_Nehalem
        const float sse = compute_scale_min4_x86_SSE(kWidth, kHeight, rowsA.data(), rowsT.data());
        const float sseW = compute_scale_min4_x86_SSE(kWidth, kHeight, rowsA.data(), rowsTW.data(), rowsW.data());
        const double mag = std::fmax(std::fabs((double)def), std::fabs((double)sse));
        const double magW = std::fmax(std::fabs((double)defW), std::fabs((double)sseW));
        const bool ok = within_tolerance(def, sse, mag);
        const bool okW = within_tolerance(defW, sseW, magW);
        std::snprintf(buf, sizeof(buf),
            "total unweighted: ref=%.9g def=%.9g sse=%.9g (def ulps from ref: %llu, sse ulps from ref: %llu, diverging rows: %zu, first at row %zu)",
            (double)ref, (double)def, (double)sse,
            (unsigned long long)ulp_distance(def, ref), (unsigned long long)ulp_distance(sse, ref),
            diverging_rows, first_diverging == (size_t)-1 ? kHeight : first_diverging
        );
        say(buf);
        std::snprintf(buf, sizeof(buf),
            "total weighted: ref=%.9g def=%.9g sse=%.9g",
            (double)refW, (double)defW, (double)sseW
        );
        say(buf);

        char v0[32], v1[32];
        if (!ok){
            std::snprintf(v0, sizeof(v0), "%.9g", (double)def);
            std::snprintf(v1, sizeof(v1), "%.9g", (double)sse);
            return std::string("MsvcProbe compute_scale: default=") + v0 + " sse=" + v1 +
                " (" + std::to_string(ulp_distance(def, sse)) + " ulps, abs " +
                std::to_string(std::fabs((double)def - (double)sse)) + " vs tolerance " +
                std::to_string(ABS_TOLERANCE * std::fmax(1.0, mag)) + ")";
        }
        if (!okW){
            std::snprintf(v0, sizeof(v0), "%.9g", (double)defW);
            std::snprintf(v1, sizeof(v1), "%.9g", (double)sseW);
            return std::string("MsvcProbe compute_scale (weighted): default=") + v0 + " sse=" + v1 +
                " (" + std::to_string(ulp_distance(defW, sseW)) + " ulps, abs " +
                std::to_string(std::fabs((double)defW - (double)sseW)) + " vs tolerance " +
                std::to_string(ABS_TOLERANCE * std::fmax(1.0, magW)) + ")";
        }
        return true;
#else
        const double mag = std::fmax(std::fabs((double)def), std::fabs((double)ref));
        std::snprintf(buf, sizeof(buf),
            "total unweighted: ref=%.9g def=%.9g (sse: n/a on this platform)",
            (double)ref, (double)def
        );
        say(buf);
        std::snprintf(buf, sizeof(buf),
            "total weighted: ref=%.9g def=%.9g (sse: n/a on this platform)",
            (double)refW, (double)defW
        );
        say(buf);
        if (!within_tolerance(def, ref, mag)){
            char v0[32], v1[32];
            std::snprintf(v0, sizeof(v0), "%.9g", (double)def);
            std::snprintf(v1, sizeof(v1), "%.9g", (double)ref);
            return std::string("MsvcProbe (no SSE on this platform) compute_scale: default=") + v0 +
                " ref=" + v1 +
                " (" + std::to_string(ulp_distance(def, ref)) + " ulps)";
        }
        return true;
#endif
    }
};



void add_tests_MsvcProbe(UnitTestDatabase& database){
    database.add<Probe>();
}



}
}
}
