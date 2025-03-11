/*  Scale Invariant Matrix Match (x86 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
#include "Kernels_SpikeConvolution_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


struct Context_x86_SSE41{
    using vtype = __m128;

    static PA_FORCE_INLINE __m128 broadcast(float x){
        return _mm_set1_ps(x);
    }
    static PA_FORCE_INLINE __m128 load_partial(const float* ptr, size_t length){
        PartialWordAccess_x64_SSE41 access(length * sizeof(float));
        return access.load_f32_no_read_past_end(ptr);
    }
    static PA_FORCE_INLINE void store_partial(float* ptr, __m128 x, size_t length){
        PartialWordAccess_x64_SSE41 access(length * sizeof(float));
        return access.store_f32_no_past_end(ptr, x);
    }

    static PA_FORCE_INLINE __m128 multiply(__m128 k0, __m128 in){
        return _mm_mul_ps(k0, in);
    }
    static PA_FORCE_INLINE void accumulate(__m128& out0, __m128 k0, __m128 in){
        out0 = _mm_add_ps(out0, _mm_mul_ps(k0, in));
    }

    static PA_FORCE_INLINE __m128 multiply(__m128 k0, const float* ptr){
        return multiply(k0, _mm_loadu_ps(ptr));
    }
    static PA_FORCE_INLINE void accumulate(__m128& out0, __m128 k0, const float* ptr){
        accumulate(out0, k0, _mm_loadu_ps(ptr));
    }

    static PA_FORCE_INLINE __m128 multiply_partial(__m128 k0, const float* ptr, size_t length){
        return multiply(k0, load_partial(ptr, length));
    }
    static PA_FORCE_INLINE void accumulate_partial(__m128& out0, __m128 k0, const float* ptr, size_t length){
        accumulate(out0, k0, load_partial(ptr, length));
    }
};


void compute_spike_kernel_x86_SSE41(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
){
    compute_spike_kernel<Context_x86_SSE41>(out, in, lengthI, kernel, lengthK);
}



}
}
}
#endif
