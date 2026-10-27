/*  Scale Invariant Matrix Match (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_ScaleInvariantMatrixMatch_Routines.h"
#include "Kernels_ScaleInvariantMatrixMatch_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


struct Context_arm64_NEON{
    using vtype = float32x4_t;

    static PA_FORCE_INLINE vtype vzero(){
        return vdupq_n_f32(0.0f);
    }
    static PA_FORCE_INLINE vtype vset(float a){
        return vdupq_n_f32(a);
    }
    static PA_FORCE_INLINE vtype vadd(vtype a, vtype b){
        return vaddq_f32(a, b);
    }
    static PA_FORCE_INLINE vtype vmul(vtype a, vtype b){
        return vmulq_f32(a, b);
    }
    static PA_FORCE_INLINE vtype vpma(vtype a, vtype b, vtype c){
        return vfmaq_f32(c, a, b);
    }
    static PA_FORCE_INLINE vtype vpms(vtype a, vtype b, vtype c){
        return vfmsq_f32(c, a, b);
    }
    static PA_FORCE_INLINE float vreduce(vtype a){
        return vaddvq_f32(a);
    }

    //
    //  "ptr" points `length` floats before the row start (the caller shifted
    //  it back). Load the vector starting at the row start itself and zero
    //  the last "length" lanes: those row elements are re-counted by the
    //  next vector (the pointer advances past them), and loading at the row
    //  start keeps every read in-bounds (the dispatcher only routes rows of
    //  at least 4 floats to this path, so the 16-byte load stays inside the
    //  row). No back-read, no read past the end.
    //
    static PA_FORCE_INLINE vtype load_back(const float* ptr, size_t length){
        const float zero = 0.0f;
        vtype out = vld1q_f32(ptr + length);
        if (length > 0){
            out = vld1q_lane_f32(&zero, out, 3);
        }
        if (length > 1){
            out = vld1q_lane_f32(&zero, out, 2);
        }
        if (length > 2){
            out = vld1q_lane_f32(&zero, out, 1);
        }
        return out;
    }

    //
    //  "ptr" is valid for exactly "length" floats (1..3): insert them one
    //  lane at a time into a zeroed vector, so there is no out-of-bounds
    //  read in either direction.
    //
    static PA_FORCE_INLINE vtype load_front(const float* ptr, size_t length){
        vtype out = vdupq_n_f32(0.0f);
        out = vld1q_lane_f32(ptr, out, 0);
        if (length > 1){
            out = vld1q_lane_f32(ptr + 1, out, 1);
        }
        if (length > 2){
            out = vld1q_lane_f32(ptr + 2, out, 2);
        }
        return out;
    }

    static PA_FORCE_INLINE void load2_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        A = load_back((const float*)ptrA, length);
        B = load_back((const float*)ptrB, length);
    }
    static PA_FORCE_INLINE void load3_partial_back(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        A = load_back((const float*)ptrA, length);
        B = load_back((const float*)ptrB, length);
        C = load_back((const float*)ptrC, length);
    }
    static PA_FORCE_INLINE void load2_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB
    ){
        A = load_front((const float*)ptrA, length);
        B = load_front((const float*)ptrB, length);
    }
    static PA_FORCE_INLINE void load3_partial_front(
        size_t length,
        vtype& A, const void* ptrA,
        vtype& B, const void* ptrB,
        vtype& C, const void* ptrC
    ){
        A = load_front((const float*)ptrA, length);
        B = load_front((const float*)ptrB, length);
        C = load_front((const float*)ptrC, length);
    }
};



float compute_scale_min4_arm64_NEON(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    return compute_scale<SumATA2<Context_arm64_NEON>>(width, height, A, T);
}
float compute_scale_min4_arm64_NEON(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_scale<SumATA2<Context_arm64_NEON>>(width, height, A, TW, W);
}
float compute_error_min4_arm64_NEON(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
    return compute_error<SumError<Context_arm64_NEON>>(width, height, scale, A, T);
}
float compute_error_min4_arm64_NEON(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    return compute_error<SumError<Context_arm64_NEON>>(width, height, scale, A, TW, W);
}



float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_Default(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_Default(size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_Default(size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);

class Test_arm64_NEON : public Test{
public:
    Test_arm64_NEON(size_t width, size_t height, bool use_weights)
        : Test(
            "arm64_NEON",
            width, height,
            use_weights,
            compute_scale_Default,
            compute_scale_min4_arm64_NEON,
            compute_scale_Default,
            compute_scale_min4_arm64_NEON,
            compute_error_Default,
            compute_error_min4_arm64_NEON,
            compute_error_Default,
            compute_error_min4_arm64_NEON
        )
    {}
};
void add_tests_arm64_NEON(UnitTestDatabase& database){
    //  The kernel requires a minimum row width (the partial-row loads span up to
    //  a full vector); sizes below that are not registered for this ISA.
    for (size_t i = 0; i < sizeof(kTestSizes) / sizeof(kTestSizes[0]); i++){
        if (kTestSizes[i][0] < 4){
            continue;
        }
        database.add<Test_arm64_NEON>(kTestSizes[i][0], kTestSizes[i][1], false);
        database.add<Test_arm64_NEON>(kTestSizes[i][0], kTestSizes[i][1], true);
    }
}



}
}
}
#endif
