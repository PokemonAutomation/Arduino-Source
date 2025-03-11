/*  Scale Invariant Matrix Match (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_SpikeConvolution_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


struct Context_Default{
    using vtype = float;

    static PA_FORCE_INLINE float broadcast(float x){
        return x;
    }
    static PA_FORCE_INLINE float load_partial(const float* ptr, size_t length){
        return ptr[0];
    }
    static PA_FORCE_INLINE void store_partial(float* ptr, float x, size_t length){
        ptr[0] = x;
    }

    static PA_FORCE_INLINE float multiply(float k0, float in){
        return k0 * in;
    }
    static PA_FORCE_INLINE void accumulate(float& out0, float k0, float in){
        out0 += k0 * in;
    }

    static PA_FORCE_INLINE float multiply(float k0, const float* ptr){
        return k0 * ptr[0];
    }
    static PA_FORCE_INLINE void accumulate(float& out0, float k0, const float* ptr){
        out0 += k0 * ptr[0];
    }

    static PA_FORCE_INLINE float multiply_partial(float k0, const float* ptr, size_t length){
        return k0 * ptr[0];
    }
    static PA_FORCE_INLINE void accumulate_partial(float& out0, float k0, const float* ptr, size_t length){
        out0 += k0 * ptr[0];
    }
};


void compute_spike_kernel_Default(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
){
    compute_spike_kernel<Context_Default>(out, in, lengthI, kernel, lengthK);
}



}
}
}
