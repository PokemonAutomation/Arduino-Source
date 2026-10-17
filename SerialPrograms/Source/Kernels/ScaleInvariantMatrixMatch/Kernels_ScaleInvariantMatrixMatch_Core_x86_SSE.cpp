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
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <random>
#include <vector>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_ScaleInvariantMatrixMatch_TestInfra.h"

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



float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_Default(size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_Default(size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);

class Test_x86_SSE : public Test{
public:
    Test_x86_SSE(size_t width, size_t height, bool use_weights)
        : Test(
            "x86_SSE",
            width, height,
            use_weights,
            compute_scale_Default,
            compute_scale_min4_x86_SSE,
            compute_scale_Default,
            compute_scale_min4_x86_SSE,
            compute_error_Default,
            compute_error_min4_x86_SSE,
            compute_error_Default,
            compute_error_min4_x86_SSE
        )
    {}
};

//
//  DIAG: calls the SSE kernel repeatedly on identical data to check whether
//  its result is deterministic (a miscompilation of fixed code) or varies
//  from call to call (an uninitialized-memory read). Always passes; the
//  values are only reported to the log.
//
class Probe_x86_SSE : public UnitTest{
public:
    Probe_x86_SSE(size_t width, size_t height)
        : UnitTest("Kernels::ScaleInvariantMatrixMatch - x86_SSE_PROBE(w=" + std::to_string(width) + ", h=" + std::to_string(height) + ")")
        , m_width(width)
        , m_height(height)
    {}
    UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        (void)scope;
        const size_t n = m_width * m_height;
        AlignedVector<float> A(n + 16 + 15);
        AlignedVector<float> T(n + 16 + 15);
        AlignedVector<const float*> rowsA(m_height);
        AlignedVector<const float*> rowsT(m_height);
        std::mt19937 gen((unsigned int)(m_width * 1000003u + m_height * 7919u));
        std::uniform_real_distribution<float> data(-1.0f, 1.0f);
        for (size_t c = 16; c < 16 + n; c++){
            A[c] = data(gen);
            T[c] = data(gen);
        }
        for (size_t r = 0; r < m_height; r++){
            rowsA[r] = A.data() + 16 + r * m_width;
            rowsT[r] = T.data() + 16 + r * m_width;
        }
        float ref = compute_scale_Default(m_width, m_height, rowsA.data(), rowsT.data());
        //
        //  Portable lane-by-lane emulation of the SSE4.1 algorithm: same
        //  prefix / 4xVL main loop / tail split, same per-lane math, and
        //  the PSHUFB tail zero-fill semantics (unused lanes are zero).
        //  If this matches the SSE kernel, the algorithm itself is wrong;
        //  if it does not, the compiler/ISA is.
        //
        double em_at = 0.0, em_a2 = 0.0;
        for (size_t r = 0; r < m_height; r++){
            const float* a = rowsA[r];
            const float* t = rowsT[r];
            size_t length = m_width;
            float at0 = 0.0f, as0 = 0.0f;
            size_t align = (size_t)t % 16;
            if (align){
                align /= 4;
                a -= (ptrdiff_t)align;
                t -= (ptrdiff_t)align;
                size_t keep = 4 - align;
                for (size_t i = 0; i < 4; i++){
                    float av = (i < keep) ? a[i] : 0.0f;
                    float tv = (i < keep) ? t[i] : 0.0f;
                    at0 += av * tv;
                    as0 += av * av;
                }
                a += 4;
                t += 4;
                length -= 4 - align;
            }
            size_t lc = length / 16;
            for (size_t c = 0; c < lc; c++){
                for (size_t i = 0; i < 64; i++){
                    at0 += a[i] * t[i];
                    as0 += a[i] * a[i];
                }
                a += 16;
                t += 16;
            }
            length %= 16;
            while (length >= 4){
                for (size_t i = 0; i < 4; i++){
                    at0 += a[i] * t[i];
                    as0 += a[i] * a[i];
                }
                a += 4;
                t += 4;
                length -= 4;
            }
            if (length){
                for (size_t i = 0; i < 4; i++){
                    float av = (i < length) ? a[i] : 0.0f;
                    float tv = (i < length) ? t[i] : 0.0f;
                    at0 += av * tv;
                    as0 += av * av;
                }
            }
            em_at += at0;
            em_a2 += as0;
        }
        float emu = (float)(em_at / em_a2);
        std::string line = "PROBE ref=" + std::to_string(ref) + " emu=" + std::to_string(emu) + " sse=";
        float first = 0.0f;
        bool all_same = true;
        float mn = 1e38f, mx = -1e38f;
        const size_t bufsz = n + 16 + 15;
        //  Full byte snapshots (canaries included) so we can detect and
        //  fingerprint any concurrent write into this test's memory.
        std::vector<uint8_t> sa((const uint8_t*)A.data() - 16, (const uint8_t*)A.data() - 16 + bufsz * 4 + 32);
        std::vector<uint8_t> st((const uint8_t*)T.data() - 16, (const uint8_t*)T.data() - 16 + bufsz * 4 + 32);
        std::vector<uint8_t> sra(rowsA.data(), rowsA.data() + m_height);
        std::vector<uint8_t> srt(rowsT.data(), rowsT.data() + m_height);
        int first_dirty = -1;
        std::string dirty_info;
        for (int i = 0; i < 64; i++){
            float v = compute_scale_min4_x86_SSE(m_width, m_height, rowsA.data(), rowsT.data());
            if (i == 0){
                first = v;
            }
            if (v != first){
                all_same = false;
            }
            if (v < mn){ mn = v; }
            if (v > mx){ mx = v; }
            if (i < 8){
                line += std::to_string(v) + " ";
            }
            if (first_dirty == -1){
                char msg[512];
                msg[0] = 0;
                auto* ca = (const uint8_t*)A.data() - 16;
                auto* ct = (const uint8_t*)T.data() - 16;
                for (size_t b = 0; b < sa.size(); b++){
                    if (ca[b] != sa[b]){
                        snprintf(msg, sizeof msg, "A+%zd was %02x now %02x", (ptrdiff_t)(A.data()) + (ptrdiff_t)b - 16, sa[b], ca[b]);
                        break;
                    }
                }
                if (msg[0] == 0){
                    for (size_t b = 0; b < st.size(); b++){
                        if (ct[b] != st[b]){
                            snprintf(msg, sizeof msg, "T+%zd was %02x now %02x", (ptrdiff_t)(T.data()) + (ptrdiff_t)b - 16, st[b], ct[b]);
                            break;
                        }
                    }
                }
                if (msg[0] == 0){
                    auto* cra = (const uint8_t*)rowsA.data();
                    auto* crt = (const uint8_t*)rowsT.data();
                    for (size_t b = 0; b < sra.size(); b++){
                        if (cra[b] != sra[b] || crt[b] != srt[b]){
                            snprintf(msg, sizeof msg, "rows+%zu", b);
                            break;
                        }
                    }
                }
                if (msg[0]){
                    first_dirty = i;
                    dirty_info = " dirty_at_call=" + std::to_string(i) + " " + msg + " v=" + std::to_string(v);
                    break;
                }
            }
        }
        line += "... all_same=" + std::to_string(all_same ? 1 : 0) + " min=" + std::to_string(mn) + " max=" + std::to_string(mx) + dirty_info;
        logger.log(line);
        logger.log(std::string("PROBE addrs A=") + std::to_string((size_t)(uintptr_t)A.data()) + " T=" + std::to_string((size_t)(uintptr_t)T.data()) + " rowsA=" + std::to_string((size_t)(uintptr_t)rowsA.data()) + " rowsT=" + std::to_string((size_t)(uintptr_t)rowsT.data()));
        return true;
    }
private:
    size_t m_width;
    size_t m_height;
};

void add_tests_x86_SSE(UnitTestDatabase& database){
    //  The kernel requires a minimum row width (the partial-row loads span up to
    //  a full vector); sizes below that are not registered for this ISA.
    for (size_t i = 0; i < sizeof(kTestSizes) / sizeof(kTestSizes[0]); i++){
        if (kTestSizes[i][0] < 4){
            continue;
        }
        for (bool use_weights : { false, true }){
            database.add<Test_x86_SSE>(kTestSizes[i][0], kTestSizes[i][1], use_weights);
        }
    }
    database.add<Probe_x86_SSE>(8, 33);
    database.add<Probe_x86_SSE>(4, 1);
    database.add<Probe_x86_SSE>(8, 1);
}



}
}
}
#endif
