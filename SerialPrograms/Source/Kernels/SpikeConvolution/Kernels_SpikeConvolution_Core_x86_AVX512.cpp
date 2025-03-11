/*  Scale Invariant Matrix Match (x86 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels_SpikeConvolution_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


struct Context_x86_AVX512{
    using vtype = __m512;

    static PA_FORCE_INLINE __m512 broadcast(float x){
        return _mm512_set1_ps(x);
    }
    static PA_FORCE_INLINE __m512 load_partial(const float* ptr, size_t length){
        __mmask16 mask = ((uint16_t)1 << length) - 1;
        return _mm512_maskz_load_ps(mask, ptr);
    }
    static PA_FORCE_INLINE void store_partial(float* ptr, __m512 x, size_t length){
        __mmask16 mask = ((uint16_t)1 << length) - 1;
        _mm512_mask_store_ps(ptr, mask, x);
    }

    static PA_FORCE_INLINE __m512 multiply(__m512 k0, __m512 in){
        return _mm512_mul_ps(k0, in);
    }
    static PA_FORCE_INLINE void accumulate(__m512& out0, __m512 k0, __m512 in){
        out0 = _mm512_fmadd_ps(k0, in, out0);
    }

    static PA_FORCE_INLINE __m512 multiply(__m512 k0, const float* ptr){
        return multiply(k0, _mm512_loadu_ps(ptr));
    }
    static PA_FORCE_INLINE void accumulate(__m512& out0, __m512 k0, const float* ptr){
        accumulate(out0, k0, _mm512_loadu_ps(ptr));
    }

    static PA_FORCE_INLINE __m512 multiply_partial(__m512 k0, const float* ptr, size_t length){
        return multiply(k0, load_partial(ptr, length));
    }
    static PA_FORCE_INLINE void accumulate_partial(__m512& out0, __m512 k0, const float* ptr, size_t length){
        accumulate(out0, k0, load_partial(ptr, length));
    }
};


void compute_spike_kernel_x86_AVX512(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
){
    compute_spike_kernel<Context_x86_AVX512>(out, in, lengthI, kernel, lengthK);
}



}
}
}
#endif
