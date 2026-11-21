/*  MSVC repro: ScaleInvariantMatrixMatch unweighted compute_scale, SSE
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Standalone minimal repro of the intermittent CI failure
 *
 *      Kernels::ScaleInvariantMatrixMatch - x86_SSE(w=512, h=513, ...)
 *      Message: compute_scale: default=... isa=... (out of tolerance)
 *
 *  The failing check compares the scalar "Default" kernel and the SSE
 *  kernel on the same deterministic data. Both are instantiations of one
 *  template (SumATA2 in Kernels_ScaleInvariantMatrixMatch_Routines.h),
 *  which this file copies verbatim, together with the two contexts
 *  (Context_Default from Kernels_ScaleInvariantMatrixMatch_Core_Default.cpp
 *  and Context_x86_SSE41 from Kernels_ScaleInvariantMatrixMatch_Core_x86_SSE.cpp)
 *  and the SSE helpers they use (Kernels_PartialWordAccess_x64_SSE41.h and
 *  reduce32_x64_SSE from Kernels_x64_SSE41.h).
 *
 *  It computes
 *
 *      s = sum(A*T) / sum(A*A)      over a 512 x 513 matrix of floats
 *
 *  three ways:
 *    ref    : double precision scalar - ground truth,
 *    deflt  : the Default kernel (float scalar lanes),
 *    sse    : the SSE kernel (128-bit vector lanes),
 *
 *  and passes (exit 0) when sse and deflt are within the same tolerance
 *  the CI test uses: 4096 ULPs, or 1e-4 times the natural magnitude.
 *
 *  The test data is generated exactly like CI's failing test
 *  (Kernels_ScaleInvariantMatrixMatch_TestInfra.cpp): the same seed
 *  (the weighted (512, 513) instance, the one that fails in CI), the same
 *  std::mt19937 / std::uniform_real_distribution stream, with the W draw
 *  per element skipped. On MSVC this makes the data bit-identical to the
 *  data of the failing CI test, so `deflt` should print the same value CI
 *  prints for `default` (in one observed run: -0.00225342088). On gcc/
 *  clang the data differs (std::uniform_real_distribution is not
 *  standardized), but that only shifts the absolute values, not the
 *  PASS/FAIL logic.
 *
 *  Build with the CI's MSVC flags (RelWithDebInfo, no debug info):
 *
 *      cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /MP /utf-8 ^
 *         msvc_simm_repro.cpp /Fe:repro.exe
 *      repro.exe
 *
 *  A conforming compiler prints PASS and exits 0. The MSVC build that
 *  misbehaves in CI is expected to print FAIL, with sse far from ref.
 *  To bisect the flag set, rebuild without /MP:
 *
 *      cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /utf-8 ^
 *         msvc_simm_repro.cpp /Fe:repro_nom.exe
 *
 *  The same file also builds with clang/gcc (-std=c++23 -O2) for
 *  sanity: it must PASS there.
 */

#include <bit>
#include <immintrin.h>
#include <cstdint>
#include <cstdio>
#include <random>
#include <cstdlib>

//  Common/Compiler.h's PA_FORCE_INLINE.
#ifdef _MSC_VER
#define REPRO_FORCE_INLINE     inline __forceinline
#else
#define REPRO_FORCE_INLINE     inline __attribute__ ((always_inline))
#endif



//
//  From Kernels_PartialWordAccess_x64_SSE41.h (the load half; the stores are
//  not used by compute_scale).
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
//  SumATA2 template, verbatim (vpms is unused by compute_scale but kept so
//  the Contexts match the CI code exactly).
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
//  checks the alignment of A and T and throws on a mismatch; the check is
//  dead at w = 512, where every row has the same alignment).
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
//  Test data.
//
constexpr size_t W = 512;
constexpr size_t H = 513;
constexpr size_t HEAD_PAD = 16;

//  64-byte aligned at runtime, exactly like CI's GuardPageBuffer rows at
//  this size (the 16-float head pad plus 2048-byte rows keep every row on
//  a 64-byte boundary).
alignas(64) static float A_data[HEAD_PAD + W * H];
alignas(64) static float T_data[HEAD_PAD + W * H];

//  CI's row pointers come out of mmap(), whose alignment the compiler
//  cannot see; the kernel therefore keeps its partial-row paths live in
//  the binary. Reroute the pointer through a volatile so this repro emits
//  the same code.
static const float* drop_alignment(const float* p){
    const volatile uint64_t* v = reinterpret_cast<const volatile uint64_t*>(&p);
    return reinterpret_cast<const float*>(*v);
}

//  Ground truth, in double.
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



int main(){
    //  Fill the data, exactly like the CI weighted (512, 513) test: the
    //  same seed and stream, with the W draw per element skipped. HEAD_PAD
    //  stays zero, as it does in CI.
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

    double ref = ref_scale(rowsA, rowsT);
    float deflt = compute_scale<SumATA2<Context_Default>>(W, H, rowsA, rowsT);
    float sse = compute_scale<SumATA2<Context_x86_SSE41>>(W, H, rowsA, rowsT);

    std::printf(
        "row 0 alignment: A mod 16 = %llu, T mod 16 = %llu "
        "(0 in CI, where the rows come from a 64-byte-aligned mmap)\n",
        (unsigned long long)((size_t)rowsA[0] % 16),
        (unsigned long long)((size_t)rowsT[0] % 16)
    );

    char rbuf[32], dbuf[32], sbuf[32];
    std::snprintf(rbuf, sizeof(rbuf), "%.9g", (double)ref);
    std::snprintf(dbuf, sizeof(dbuf), "%.9g", (double)deflt);
    std::snprintf(sbuf, sizeof(sbuf), "%.9g", (double)sse);

    std::printf(
        "w=%zu h=%zu unweighted compute_scale (SIMM SSE repro)\n"
        "ref (double, ground truth) = %s\n"
        "default (float scalar)     = %s\n"
        "sse     (float vector)     = %s\n",
        W, H, rbuf, dbuf, sbuf
    );

    //  CI compares against fmax(|default|, |isa|).
    double magD = (double)deflt; if (magD < 0.0) magD = -magD;
    double magS = (double)sse; if (magS < 0.0) magS = -magS;
    double mag = magD > magS ? magD : magS;
    bool deflt_ok = within_tolerance(deflt, (float)ref, mag);
    bool sse_ok = within_tolerance(sse, (float)ref, mag);

    if (deflt_ok && sse_ok){
        std::printf("PASS: default and sse agree with the reference within tolerance\n");
        return 0;
    }

    std::printf(
        "FAIL: default=%s sse=%s ref=%s "
        "(default ulps from ref: %llu, sse ulps from ref: %llu)\n",
        dbuf, sbuf, rbuf,
        (unsigned long long)ulp_distance(deflt, (float)ref),
        (unsigned long long)ulp_distance(sse, (float)ref)
    );
    return 1;
}
