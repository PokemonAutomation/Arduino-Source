/*  MSVC probe: ScaleInvariantMatrixMatch unweighted compute_scale, SSE
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Minimal instrumented probe for the CI-only failure
 *
 *      Kernels::ScaleInvariantMatrixMatch - x86_SSE(w=512, h=513, ...)
 *      Message: compute_scale: default=... isa=... (out of tolerance)
 *
 *  It computes s = sum(A*T) / sum(A*A) over a 512 x 513 float matrix,
 *  with the same data stream as the failing CI test (same seed and
 *  std::mt19937 / std::uniform_real_distribution sequence as the weighted
 *  (512, 513) instance, the W draw per element is discarded but must
 *  happen to keep the stream aligned; on MSVC the A/T values are
 *  bit-identical to the CI test's), and logs the intermediate steps so a
 *  failing CI run localizes the divergence:
 *
 *    1. the exact compiler build (_MSC_FULL_VER) - the failing pool runs
 *       and passing ones must be distinguishable;
 *    2. a data dump (sums + first values, bit patterns included) to check
 *       the data is what the platform's <random> is supposed to produce,
 *       and is stable across runs;
 *    3. every row's ref / default / sse values (row-by-row calls of the
 *       verbatim kernels) - which row first diverges;
 *    4. the SSE kernel's four quarter-accumulators (the lanes of the
 *       sum_at / sum_as vectors) for the first rows and for diverging
 *       rows - which lane is wrong;
 *    5. the full-matrix result, exactly like CI, as the verdict.
 *
 *  The kernels themselves are verbatim copies of the CI sources
 *  (Kernels_ScaleInvariantMatrixMatch_Routines.h, Core_Default.cpp,
 *  Core_x86_SSE.cpp, Kernels_x64_SSE41.h,
 *  Kernels_PartialWordAccess_x64_SSE41.h). The rows are 64-byte aligned
 *  at runtime, as in CI, so the partial-row paths are dead; the row
 *  pointers are rerouted through a volatile so the compiler keeps those
 *  paths live in the binary, as in the CI build.
 *
 *  Build with the CI's MSVC flags:
 *
 *      cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /MP /utf-8 ^
 *         /external:anglebrackets msvc_simm_repro.cpp /Fe:probe.exe
 *      probe.exe
 *
 *  and, for flag bisection, the same without /MP. A conforming compiler
 *  prints PASS and exits 0; the misbehaving build is expected to print
 *  the diverging rows/lanes and FAIL, exit 1.
 */

#include <bit>
#include <immintrin.h>
#include <cstdint>
#include <cstdio>
#include <random>

//  Common/Compiler.h's PA_FORCE_INLINE.
#ifdef _MSC_VER
#define REPRO_FORCE_INLINE     inline __forceinline
#else
#define REPRO_FORCE_INLINE     inline __attribute__ ((always_inline))
#endif



//
//  From Kernels_PartialWordAccess_x64_SSE41.h (the load half).
//
struct PartialWordAccess_x64_SSE41{
public:
    REPRO_FORCE_INLINE PartialWordAccess_x64_SSE41(size_t bytes)
        : m_shift(16 - bytes)
    {
        m_front_mask = _mm_cmpgt_epi8(
            _mm_set1_epi8((uint8_t)bytes),
            _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)
        );

        m_shift_front = _mm_setr_epi8(-128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113);
        m_shift_front = _mm_sub_epi8(
            m_shift_front,
            _mm_set1_epi8((uint8_t)bytes)
        );

        m_shift_back = _mm_sub_epi8(
            _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
            _mm_set1_epi8((uint8_t)m_shift)
        );
    }

    REPRO_FORCE_INLINE __m128i load_int_no_read_past_end(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)((const char*)ptr - m_shift));
        return _mm_shuffle_epi8(x, m_shift_front);
    }
    REPRO_FORCE_INLINE __m128i load_int_no_read_before_ptr(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)ptr);
        return _mm_and_si128(x, m_front_mask);
    }
    REPRO_FORCE_INLINE __m128 load_f32_no_read_past_end(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)((const char*)ptr - m_shift));
        return _mm_castsi128_ps(_mm_shuffle_epi8(x, m_shift_front));
    }
    REPRO_FORCE_INLINE __m128 load_f32_no_read_before_ptr(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)ptr);
        return _mm_castsi128_ps(_mm_and_si128(x, m_front_mask));
    }

private:
    uint8_t m_shift;
    __m128i m_front_mask;
    __m128i m_shift_front;
    __m128i m_shift_back;
};



//  From Kernels_x64_SSE41.h.
REPRO_FORCE_INLINE float reduce32_x64_SSE(__m128 x){
    x = _mm_add_ps(x, _mm_shuffle_ps(x, x, 78));
    x = _mm_add_ps(x, _mm_shuffle_ps(x, x, 177));
    return _mm_cvtss_f32(x);
}



//
//  From Kernels_ScaleInvariantMatrixMatch_Routines.h: the unweighted
//  SumATA2 template, verbatim.
//
template <typename Context>
struct SumATA2{
    using vtype = typename Context::vtype;
    static constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    vtype sum_AT;
    vtype sum_A2;

    REPRO_FORCE_INLINE SumATA2()
        : sum_AT(Context::vzero())
        , sum_A2(Context::vzero())
    {}

    REPRO_FORCE_INLINE float scale() const{
        return Context::vreduce(sum_AT) / Context::vreduce(sum_A2);
    }

    REPRO_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        vtype sum_as0 = Context::vzero();
        vtype sum_as1 = Context::vzero();
        vtype sum_as2 = Context::vzero();
        vtype sum_as3 = Context::vzero();
        vtype sum_at0 = Context::vzero();
        vtype sum_at1 = Context::vzero();
        vtype sum_at2 = Context::vzero();
        vtype sum_at3 = Context::vzero();

        if constexpr (VECTOR_LENGTH > 1){
            size_t align = (size_t)T % (VECTOR_LENGTH * sizeof(float));
            if (align){
                align /= sizeof(float);
                A -= align;
                T -= align;

                vtype a0, t0;
                Context::load2_partial_back(align, a0, A, t0, T);
                sum_as0 = Context::vpma(a0, a0, sum_as0);
                sum_at0 = Context::vpma(a0, t0, sum_at0);

                A += VECTOR_LENGTH;
                T += VECTOR_LENGTH;
                length -= VECTOR_LENGTH - align;
            }
        }

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
        if constexpr (VECTOR_LENGTH > 1){
            if (length){
                vtype a0, t0;
                Context::load2_partial_front(length, a0, ptrA, t0, ptrT);
                sum_at0 = Context::vpma(a0, t0, sum_at0);
                sum_as0 = Context::vpma(a0, a0, sum_as0);
            }
        }

        sum_A2 = Context::vadd(sum_A2, sum_as0);
        sum_AT = Context::vadd(sum_AT, sum_at0);
    }
};



//  From Kernels_ScaleInvariantMatrixMatch_Core_Default.cpp.
struct Context_Default{
    using vtype = float;

    static REPRO_FORCE_INLINE vtype vzero(){
        return 0;
    }
    static REPRO_FORCE_INLINE vtype vadd(vtype a, vtype b){
        return a + b;
    }
    static REPRO_FORCE_INLINE vtype vmul(vtype a, vtype b){
        return a * b;
    }
    static REPRO_FORCE_INLINE vtype vpma(vtype a, vtype b, vtype c){
        return a * b + c;
    }
    static REPRO_FORCE_INLINE vtype vpms(vtype a, vtype b, vtype c){
        return a * b - c;
    }
    static REPRO_FORCE_INLINE float vreduce(vtype a){
        return a;
    }

    static REPRO_FORCE_INLINE void load2_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        A = *(const float*)ptrA;
        B = *(const float*)ptrB;
    }
    static REPRO_FORCE_INLINE void load2_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        A = *(const float*)ptrA;
        B = *(const float*)ptrB;
    }
};



//  From Kernels_ScaleInvariantMatrixMatch_Core_x86_SSE.cpp.
struct Context_x86_SSE41{
    using vtype = __m128;

    static REPRO_FORCE_INLINE vtype vzero(){
        return _mm_setzero_ps();
    }
    static REPRO_FORCE_INLINE vtype vadd(vtype a, vtype b){
        return _mm_add_ps(a, b);
    }
    static REPRO_FORCE_INLINE vtype vmul(vtype a, vtype b){
        return _mm_mul_ps(a, b);
    }
    static REPRO_FORCE_INLINE vtype vpma(vtype a, vtype b, vtype c){
        return _mm_add_ps(_mm_mul_ps(a, b), c);
    }
    static REPRO_FORCE_INLINE vtype vpms(vtype a, vtype b, vtype c){
        return _mm_sub_ps(_mm_mul_ps(a, b), c);
    }
    static REPRO_FORCE_INLINE float vreduce(vtype a){
        return reduce32_x64_SSE(a);
    }

    static REPRO_FORCE_INLINE void load2_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        PartialWordAccess_x64_SSE41 access(16 - length * 4);
        A = access.load_f32_no_read_before_ptr((const float*)ptrA + length);
        B = access.load_f32_no_read_before_ptr((const float*)ptrB + length);
    }
    static REPRO_FORCE_INLINE void load2_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        PartialWordAccess_x64_SSE41 access(length * 4);
        A = access.load_f32_no_read_past_end(ptrA);
        B = access.load_f32_no_read_past_end(ptrB);
    }
};



//  From Kernels_ScaleInvariantMatrixMatch_Routines.h (the CI version also
//  checks the row alignment and throws on a mismatch; dead at w = 512).
template <typename SumATA2>
REPRO_FORCE_INLINE float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    SumATA2 sum;
    for (size_t r = 0; r < height; r++){
        sum.accumulate(width, A[r], T[r]);
    }
    return sum.scale();
}



//
//  Test data: the exact stream of the failing CI test.
//
constexpr size_t W = 512;
constexpr size_t H = 513;
constexpr size_t HEAD_PAD = 16;

//  64-byte aligned at runtime, exactly like CI's GuardPageBuffer rows at
//  this size.
alignas(64) static float A_data[HEAD_PAD + W * H];
alignas(64) static float T_data[HEAD_PAD + W * H];

//  CI's row pointers come out of mmap(), whose alignment the compiler
//  cannot see; the kernel therefore keeps its partial-row paths live in
//  the binary. Reroute the pointer through a volatile so this probe
//  emits the same code.
static const float* drop_alignment(const float* p){
    const volatile uint64_t* v = reinterpret_cast<const volatile uint64_t*>(&p);
    return reinterpret_cast<const float*>(*v);
}

//  Double-precision reference, full matrix.
static double ref_scale(float const* const* A, float const* const* T){
    double sat = 0.0, saa = 0.0;
    for (size_t r = 0; r < H; r++){
        for (size_t c = 0; c < W; c++){
            sat += (double)A[r][c] * (double)T[r][c];
            saa += (double)A[r][c] * (double)A[r][c];
        }
    }
    return sat / saa;
}
//  Double-precision reference, one row.
static double ref_row(const float* A, const float* T){
    double sat = 0.0, saa = 0.0;
    for (size_t c = 0; c < W; c++){
        sat += (double)A[c] * (double)T[c];
        saa += (double)A[c] * (double)A[c];
    }
    return sat / saa;
}



//
//  Lane localization: the SSE kernel's row-reduced accumulators for one
//  row, i.e. the lanes of sum_at / sum_as after the per-row quarter
//  reduction - lane j holds the contributions of the elements whose index
//  is j mod 16. A copy of the main loop of
//  SumATA2<Context_x86_SSE41>::accumulate (the back/front partial-row
//  paths are dead at w = 512 and are omitted here); the loop body and the
//  reduction are identical, so a miscompiling compiler is expected to
//  miscompile both. This copy is instrumentation: its values localize the
//  bug, while the verdict below uses the verbatim kernel.
//
struct RowLanes{
    float at[4];
    float as[4];
};
static RowLanes sse_row_lanes(const float* A, const float* T){
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

    size_t lc = W / (4 * 4);
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

    //  The per-row reduction, verbatim from SumATA2::accumulate.
    sum_as0 = _mm_add_ps(sum_as0, sum_as1);
    sum_at0 = _mm_add_ps(sum_at0, sum_at1);
    sum_as2 = _mm_add_ps(sum_as2, sum_as3);
    sum_at2 = _mm_add_ps(sum_at2, sum_at3);
    sum_as0 = _mm_add_ps(sum_as0, sum_as2);
    sum_at0 = _mm_add_ps(sum_at0, sum_at2);

    RowLanes l;
    union { __m128 v; float f[4]; } uat = { sum_at0 };
    union { __m128 v; float f[4]; } uas = { sum_as0 };
    for (int i = 0; i < 4; i++){
        l.at[i] = uat.f[i];
        l.as[i] = uas.f[i];
    }
    return l;
}



//  From Kernels_ScaleInvariantMatrixMatch_TestInfra.cpp.
static constexpr uint64_t MAX_ULP_DIFF = 4096;
static constexpr double ABS_TOLERANCE = 1e-4;

static uint64_t ulp_distance(float a, float b){
    auto U = [](float f) -> uint64_t{
        int32_t i = (int32_t)std::bit_cast<uint32_t>(f);
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
    double tol = ABS_TOLERANCE * (magnitude > 1.0 ? magnitude : 1.0);
    double d = (double)x - (double)y;
    if (d < 0.0)
        d = -d;
    return d <= tol;
}

static void print_floats(const char* label, const float* p, int n){
    std::printf("%s:", label);
    for (int i = 0; i < n; i++){
        uint32_t bits = std::bit_cast<uint32_t>(p[i]);
        std::printf(" 0x%08X (%.9g)", bits, (double)p[i]);
    }
    std::printf("\n");
}



int main(){
    //  0. Which compiler built this? The pool's runners have been
    //     rotating, and this number is the first thing to compare between
    //     a failing and a passing run.
#ifdef _MSC_VER
    std::printf("compiler: MSVC _MSC_VER=%d _MSC_FULL_VER=%d _MSC_BUILD=%d\n",
        _MSC_VER, _MSC_FULL_VER, _MSC_BUILD);
#else
    std::printf("compiler: non-MSVC (sanity build)\n");
#endif

    //  1. The data, exactly like the CI weighted (512, 513) test: the
    //     same seed and stream; the W draw per element is discarded but
    //     must happen to keep the mt19937 stream aligned with the test's.
    std::mt19937 generator(512u * 1000003u + 513u * 7919u + 2654435761u);
    std::uniform_real_distribution<float> data_distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> weight_distribution(0.0f, 1.0f);
    for (size_t c = HEAD_PAD; c < HEAD_PAD + W * H; c++){
        A_data[c] = data_distribution(generator);
        T_data[c] = data_distribution(generator);
        (void)weight_distribution(generator);
    }

    const float* rowsA[H];
    const float* rowsT[H];
    const float* baseA = drop_alignment(A_data);
    const float* baseT = drop_alignment(T_data);
    for (size_t r = 0; r < H; r++){
        rowsA[r] = baseA + HEAD_PAD + r * W;
        rowsT[r] = baseT + HEAD_PAD + r * W;
    }

    //  2. Data dump: check the stream is what this platform's <random>
    //     produces, and stable across runs. The row alignment must be 0.
    double sumA = 0.0, sumT = 0.0;
    for (size_t c = HEAD_PAD; c < HEAD_PAD + W * H; c++){
        sumA += A_data[c];
        sumT += T_data[c];
    }
    std::printf("data: sum(A)=%.15g sum(T)=%.15g over %zu elements\n",
        sumA, sumT, W * H);
    print_floats("data A[0..3]", A_data + HEAD_PAD, 4);
    print_floats("data T[0..3]", T_data + HEAD_PAD, 4);
    std::printf(
        "rows: A[0] = %p (mod 16 = %llu, mod 64 = %llu), "
        "T[0] = %p (mod 16 = %llu, mod 64 = %llu)\n",
        (const void*)rowsA[0],
        (unsigned long long)((size_t)rowsA[0] % 16),
        (unsigned long long)((size_t)rowsA[0] % 64),
        (const void*)rowsT[0],
        (unsigned long long)((size_t)rowsT[0] % 16),
        (unsigned long long)((size_t)rowsT[0] % 64)
    );

    //  3. Row by row: ref / default / sse per row, from the verbatim
    //     kernels. The first diverging row localizes the bug to a row
    //     (or to the accumulation across rows, if all rows agree).
    std::printf("rows (ref = double, def = scalar kernel, sse = SSE kernel):\n");
    int diverging = 0;
    size_t first_diverging_row = H;
    size_t lane_dumped = 0;
    for (size_t r = 0; r < H; r++){
        const float* ra = rowsA[r];
        const float* rt = rowsT[r];
        const float* one_ra[1] = { ra };
        const float* one_rt[1] = { rt };
        double rref = ref_row(ra, rt);
        float rdef = compute_scale<SumATA2<Context_Default>>(W, 1, one_ra, one_rt);
        float rsse = compute_scale<SumATA2<Context_x86_SSE41>>(W, 1, one_ra, one_rt);
        double magD = rdef < 0 ? -(double)rdef : (double)rdef;
        double magS = rsse < 0 ? -(double)rsse : (double)rsse;
        double mag = magD > magS ? magD : magS;
        bool ok = within_tolerance((float)rref, rdef, mag)
            && within_tolerance((float)rref, rsse, mag);
        std::printf("row %3zu: ref=%.9g def=%.9g sse=%.9g%s\n",
            r, (double)rref, (double)rdef, (double)rsse, ok ? "" : "  <-- DIVERGES");
        if (!ok){
            diverging++;
            if (first_diverging_row == H)
                first_diverging_row = r;
        }
        if ((r < 4 || (!ok && lane_dumped < 16) || r == H - 1) && lane_dumped < 32){
            RowLanes l = sse_row_lanes(ra, rt);
            std::printf(
                "      sse lanes: at=[%.9g %.9g %.9g %.9g] as=[%.9g %.9g %.9g %.9g]\n",
                (double)l.at[0], (double)l.at[1], (double)l.at[2], (double)l.at[3],
                (double)l.as[0], (double)l.as[1], (double)l.as[2], (double)l.as[3]
            );
            lane_dumped++;
        }
    }

    //  4. The full-matrix result, exactly like CI: the verdict.
    double ref = ref_scale(rowsA, rowsT);
    float deflt = compute_scale<SumATA2<Context_Default>>(W, H, rowsA, rowsT);
    float sse = compute_scale<SumATA2<Context_x86_SSE41>>(W, H, rowsA, rowsT);

    std::printf(
        "total: ref=%.9g def=%.9g sse=%.9g "
        "(def ulps from ref: %llu, sse ulps from ref: %llu, "
        "diverging rows: %d, first at row %zu)\n",
        (double)ref, (double)deflt, (double)sse,
        (unsigned long long)ulp_distance(deflt, (float)ref),
        (unsigned long long)ulp_distance(sse, (float)ref),
        diverging, first_diverging_row
    );

    double magD = deflt < 0 ? -(double)deflt : (double)deflt;
    double magS = sse < 0 ? -(double)sse : (double)sse;
    double mag = magD > magS ? magD : magS;
    bool deflt_ok = within_tolerance(deflt, (float)ref, mag);
    bool sse_ok = within_tolerance(sse, (float)ref, mag);
    if (deflt_ok && sse_ok){
        std::printf("PASS: default and sse agree with the reference within tolerance\n");
        return 0;
    }

    std::printf("FAIL: default %s, sse %s the reference\n",
        deflt_ok ? "matches" : "MISMATCHES", sse_ok ? "matches" : "MISMATCHES");
    return 1;
}
