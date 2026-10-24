/*  Spike Convolution (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
#include "Kernels_SpikeConvolution_Routines.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_SpikeConvolution_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


struct Context_arm64_NEON{
    using vtype = float32x4_t;

    static PA_FORCE_INLINE vtype broadcast(float x){
        return vdupq_n_f32(x);
    }
    static PA_FORCE_INLINE vtype load_partial(const float* ptr, size_t length){
        //  Insert the "length" valid floats one lane at a time, zero-filled
        //  otherwise: no out-of-bounds read in either direction ("ptr" is
        //  valid for exactly "length" floats).
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

    static PA_FORCE_INLINE vtype multiply(vtype k0, vtype in){
        return vmulq_f32(k0, in);
    }
    static PA_FORCE_INLINE void accumulate(vtype& out0, vtype k0, vtype in){
        out0 = vfmaq_f32(out0, k0, in);
    }
    static PA_FORCE_INLINE vtype multiply(vtype k0, const float* ptr){
        return multiply(k0, vld1q_f32(ptr));
    }
    static PA_FORCE_INLINE void accumulate(vtype& out0, vtype k0, const float* ptr){
        accumulate(out0, k0, vld1q_f32(ptr));
    }

    static PA_FORCE_INLINE vtype multiply_partial(vtype k0, const float* ptr, size_t length){
        return multiply(k0, load_partial(ptr, length));
    }
    static PA_FORCE_INLINE void accumulate_partial(vtype& out0, vtype k0, const float* ptr, size_t length){
        accumulate(out0, k0, load_partial(ptr, length));
    }
};


void compute_spike_kernel_arm64_NEON(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
){
    compute_spike_kernel<Context_arm64_NEON>(out, in, lengthI, kernel, lengthK);
}


void compute_spike_kernel_Default(float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);

class Test_arm64_NEON : public Test{
public:
    Test_arm64_NEON(size_t lengthK)
        : Test(
            "arm64_NEON",
            lengthK,
            compute_spike_kernel_Default,
            compute_spike_kernel_arm64_NEON
        )
    {}
};
void add_tests_arm64_NEON(UnitTestDatabase& database){
    for (size_t lengthK = 1; lengthK <= 32; lengthK++){
        database.add<Test_arm64_NEON>(lengthK);
    }
}



}
}
}
#endif
