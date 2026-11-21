/*  Minimal MSVC repro: the SIMM Default (scalar) kernel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The CI-only failure
 *
 *      Kernels::ScaleInvariantMatrixMatch - x86_SSE(w=512, h=513, ...)
 *      Message: compute_scale: default=-0.00225342088 isa=0.000351705559
 *      (1959531328 ulps, abs 0.002605 vs tolerance 0.000100)
 *
 *  was localized (tools: PR "Kernels: instrumented SIMM probe as the
 *  only test", run on the windows-2025-vs2026 runner image): the data
 *  is bit-identical across MSVC versions, the SSE kernel's results are
 *  correct and identical between MSVC 19.43 (VS 2022) and 19.52
 *  (VS 2026), while the scalar Default kernel's results are garbage
 *  under 19.52 - every row diverges.
 *
 *  This file is the smallest repro of that: verbatim copies of
 *  Context_Default, the unweighted SumATA2 and the unweighted
 *  compute_scale<> from the repo (Routines.h / Core_Default.cpp), plus
 *  the exact data stream of the failing test (512 x 513, weighted
 *  instance seed; per element the A, T and W values are drawn, as in
 *  the test - the W draw keeps the mt19937 stream aligned, its value
 *  is unused by this unweighted check). It prints the compiler
 *  version, each row's result against a double-precision reference and
 *  the full-matrix result, and exits 1 if the Default result diverges
 *  beyond the CI tolerance.
 *
 *  Build (the CI's exact MSVC flags):
 *      cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /MP /utf-8 ^
 *         /external:anglebrackets msvc_simm_default_repro.cpp /Fe:default_repro.exe
 *      default_repro.exe
 *
 *  A conforming compiler (gcc/clang, MSVC 19.43, ...) prints PASS and
 *  exits 0. MSVC 19.52 (VS 2026 18.0) is expected to FAIL: it prints
 *  the diverging rows and the full-matrix value.
 */

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <random>
#include <string>

//
//  Verbatim: Context_Default (Core_Default.cpp).
//
struct Context_Default{
    using vtype = float;

    static inline vtype vzero(){
        return 0.0f;
    }
    static inline vtype vadd(vtype a, vtype b){
        return a + b;
    }
    static inline vtype vmul(vtype a, vtype b){
        return a * b;
    }
    static inline vtype vpma(vtype a, vtype b, vtype c){
        return a * b + c;
    }
    static inline vtype vpms(vtype a, vtype b, vtype c){
        return a * b - c;
    }
    static inline float vreduce(vtype a){
        return a;
    }
};

//
//  Verbatim: the unweighted parts of SumATA2 (Routines.h),
//  VectorLength = 1 for the Default context.
//
template <class Context>
struct SumATA2{
    using vtype = typename Context::vtype;
    static constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    vtype sum_AT;
    vtype sum_A2;

    SumATA2()
        : sum_AT(Context::vzero())
        , sum_A2(Context::vzero())
    {}

    inline float scale() const{
        return Context::vreduce(sum_AT) / Context::vreduce(sum_A2);
    }

    inline void accumulate(size_t length, const float* A, const float* T){
        vtype sum_as0 = Context::vzero();
        vtype sum_as1 = Context::vzero();
        vtype sum_as2 = Context::vzero();
        vtype sum_as3 = Context::vzero();
        vtype sum_at0 = Context::vzero();
        vtype sum_at1 = Context::vzero();
        vtype sum_at2 = Context::vzero();
        vtype sum_at3 = Context::vzero();

        const vtype* ptrA = (const vtype*)A;
        const vtype* ptrT = (const vtype*)T;

        size_t lc = length / (4 * VECTOR_LENGTH);
        if (lc){
            do{
                vtype a0 = ptrA[0];
                vtype a1 = ptrA[1];
                vtype a2 = ptrA[2];
                vtype a3 = ptrA[3];
                sum_as0 = Context::vpma(a0, a0, sum_as0);
                sum_as1 = Context::vpma(a1, a1, sum_as1);
                sum_as2 = Context::vpma(a2, a2, sum_as2);
                sum_as3 = Context::vpma(a3, a3, sum_as3);
                sum_at0 = Context::vpma(a0, ptrT[0], sum_at0);
                sum_at1 = Context::vpma(a1, ptrT[1], sum_at1);
                sum_at2 = Context::vpma(a2, ptrT[2], sum_at2);
                sum_at3 = Context::vpma(a3, ptrT[3], sum_at3);
                ptrA += 4;
                ptrT += 4;
            }while (--lc);
            sum_as0 = Context::vadd(sum_as0, sum_as1);
            sum_at0 = Context::vadd(sum_at0, sum_at1);
            sum_as2 = Context::vadd(sum_as2, sum_as3);
            sum_at2 = Context::vadd(sum_at2, sum_at3);
            sum_as0 = Context::vadd(sum_as0, sum_as2);
            sum_at0 = Context::vadd(sum_at0, sum_at2);
        }

        length %= 4 * VECTOR_LENGTH;
        while (length >= VECTOR_LENGTH){
            vtype a0 = ptrA[0];
            sum_as0 = Context::vpma(a0, a0, sum_as0);
            sum_at0 = Context::vpma(a0, ptrT[0], sum_at0);
            ptrA += 1;
            ptrT += 1;
            length -= VECTOR_LENGTH;
        }

        sum_A2 = Context::vadd(sum_A2, sum_as0);
        sum_AT = Context::vadd(sum_AT, sum_at0);
    }
};

//
//  Verbatim: the unweighted compute_scale (Routines.h); the
//  alignment check can't fail for this data (all rows are 64-byte
//  aligned, as in the CI test).
//
template <typename SumATA2Type>
inline float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    constexpr size_t ALIGNMENT = alignof(typename SumATA2Type::vtype);
    SumATA2Type sum;
    for (size_t r = 0; r < height; r++){
        const float* ptrA = A[r];
        const float* ptrT = T[r];
        if ((size_t)ptrA % ALIGNMENT != (size_t)ptrT % ALIGNMENT){
            return std::numeric_limits<float>::quiet_NaN();
        }
        sum.accumulate(width, ptrA, ptrT);
    }
    return sum.scale();
}


//  The failing test: 512 x 513, the weighted instance's seed.
static constexpr size_t WIDTH = 512;
static constexpr size_t HEIGHT = 513;
static constexpr size_t HEAD_PAD = 16;

//  Same tolerance as the CI test.
static constexpr uint64_t MAX_ULP_DIFF = 4096;
static constexpr double ABS_TOLERANCE = 1e-4;

static uint64_t ulp_distance(float a, float b){
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


int main(){
    setvbuf(stdout, nullptr, _IONBF, 0);
#ifdef _MSC_VER
    char head[128];
    std::snprintf(head, sizeof(head),
        "compiler MSVC _MSC_VER=%d _MSC_FULL_VER=%d _MSC_BUILD=%d",
        _MSC_VER, _MSC_FULL_VER, _MSC_BUILD
    );
    std::printf("%s\n", head);
#else
    std::printf("compiler non-MSVC\n");
#endif

    //
    //  The A/T values of the failing test: same seed, same generator,
    //  same distributions, same draw order (A, T, W per element; the W
    //  draw is discarded but must happen, to keep the mt19937 stream
    //  aligned with the test's). On MSVC the A/T values are
    //  bit-identical to the CI test's.
    //
    //  64-byte aligned rows, as in the CI test (GuardPageBuffer with
    //  this width puts every row on a 64-byte boundary).
    //
    static float A[HEIGHT * WIDTH + HEAD_PAD] alignas(64);
    static float T[HEIGHT * WIDTH + HEAD_PAD] alignas(64);
    std::mt19937 generator((unsigned int)(WIDTH * 1000003u + HEIGHT * 7919u + 2654435761u));
    std::uniform_real_distribution<float> data_distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> weight_distribution(0.0f, 1.0f);
    for (size_t c = HEAD_PAD; c < HEAD_PAD + (size_t)HEIGHT * WIDTH; c++){
        A[c] = data_distribution(generator);
        T[c] = data_distribution(generator);
        (void)weight_distribution(generator);
    }
    const float* rowsA[HEIGHT];
    const float* rowsT[HEIGHT];
    for (size_t r = 0; r < HEIGHT; r++){
        rowsA[r] = A + HEAD_PAD + r * WIDTH;
        rowsT[r] = T + HEAD_PAD + r * WIDTH;
    }
    std::printf("data: sum(A)=%.9g sum(T)=%.9g over %zu elements\n",
        [](const float* p, size_t n){ double s = 0; for (size_t i = 0; i < n; i++) s += p[i]; return s; }(A + HEAD_PAD, HEIGHT * WIDTH),
        [](const float* p, size_t n){ double s = 0; for (size_t i = 0; i < n; i++) s += p[i]; return s; }(T + HEAD_PAD, HEIGHT * WIDTH),
        (size_t)HEIGHT * WIDTH
    );

    //
    //  Every row, against a double-precision reference.
    //
    size_t diverging = 0;
    size_t first_diverging = (size_t)-1;
    for (size_t r = 0; r < HEIGHT; r++){
        double ref_at = 0, ref_as = 0;
        for (size_t c = 0; c < WIDTH; c++){
            ref_at += (double)rowsA[r][c] * (double)rowsT[r][c];
            ref_as += (double)rowsA[r][c] * (double)rowsA[r][c];
        }
        const float ref = (float)(ref_at / ref_as);
        const float def = compute_scale<SumATA2<Context_Default>>(WIDTH, 1, &rowsA[r], &rowsT[r]);
        const bool ok = within_tolerance(def, ref, std::fmax(std::fabs((double)def), std::fabs((double)ref)));
        if (!ok){
            diverging++;
            if (first_diverging == (size_t)-1)
                first_diverging = r;
        }
        if (!ok || r < 4 || r == HEIGHT - 1){
            std::printf("row %3zu: ref=%.9g default=%.9g%s\n",
                r, (double)ref, (double)def, ok ? "" : "  ** DIVERGING **"
            );
        }
    }

    //
    //  The full-matrix result, exactly like the failing CI test.
    //
    double mat_at = 0, mat_as = 0;
    for (size_t r = 0; r < HEIGHT; r++){
        for (size_t c = 0; c < WIDTH; c++){
            mat_at += (double)rowsA[r][c] * (double)rowsT[r][c];
            mat_as += (double)rowsA[r][c] * (double)rowsA[r][c];
        }
    }
    const float ref = (float)(mat_at / mat_as);
    const float def = compute_scale<SumATA2<Context_Default>>(WIDTH, HEIGHT, rowsA, rowsT);
    const double mag = std::fmax(std::fabs((double)def), std::fabs((double)ref));
    std::printf("total: ref=%.9g default=%.9g (default ulps from ref: %llu, diverging rows: %zu, first at row %zu)\n",
        (double)ref, (double)def,
        (unsigned long long)ulp_distance(def, ref),
        diverging, first_diverging == (size_t)-1 ? HEIGHT : first_diverging
    );

    if (!within_tolerance(def, ref, mag)){
        char v0[32], v1[32];
        std::snprintf(v0, sizeof(v0), "%.9g", (double)ref);
        std::snprintf(v1, sizeof(v1), "%.9g", (double)def);
        std::printf("FAIL: Default compute_scale diverges from the reference: reference=%s default=%s (%llu ulps, abs %s vs tolerance %s)\n",
            v0, v1,
            (unsigned long long)ulp_distance(def, ref),
            std::to_string(std::fabs((double)def - (double)ref)).c_str(),
            std::to_string(ABS_TOLERANCE * std::fmax(1.0, mag)).c_str()
        );
        return 1;
    }
    std::printf("PASS: Default compute_scale agrees with the reference within tolerance\n");
    return 0;
}
