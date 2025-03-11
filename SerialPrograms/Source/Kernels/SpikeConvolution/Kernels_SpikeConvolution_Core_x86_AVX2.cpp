/*  Scale Invariant Matrix Match (x86 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels_SpikeConvolution_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


struct Context_x86_AVX2{
    using vtype = __m256;

    static PA_FORCE_INLINE __m256 broadcast(float x){
        return _mm256_set1_ps(x);
    }
    static PA_FORCE_INLINE __m256 load_partial(const float* ptr, size_t length){
        __m256i mask = _mm256_cmpgt_epi32(
            _mm256_set1_epi32((uint32_t)length),
            _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)
        );
        return _mm256_maskload_ps((const float*)ptr, mask);
    }
    static PA_FORCE_INLINE void store_partial(float* ptr, __m256 x, size_t length){
        __m256i mask = _mm256_cmpgt_epi32(
            _mm256_set1_epi32((uint32_t)length),
            _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)
        );
        _mm256_maskstore_ps((float*)ptr, mask, x);
    }

    static PA_FORCE_INLINE __m256 multiply(__m256 k0, __m256 in){
        return _mm256_mul_ps(k0, in);
    }
    static PA_FORCE_INLINE void accumulate(__m256& out0, __m256 k0, __m256 in){
        out0 = _mm256_fmadd_ps(k0, in, out0);
    }

    static PA_FORCE_INLINE __m256 multiply(__m256 k0, const float* ptr){
        return multiply(k0, _mm256_loadu_ps(ptr));
    }
    static PA_FORCE_INLINE void accumulate(__m256& out0, __m256 k0, const float* ptr){
        accumulate(out0, k0, _mm256_loadu_ps(ptr));
    }

    static PA_FORCE_INLINE __m256 multiply_partial(__m256 k0, const float* ptr, size_t length){
        return multiply(k0, load_partial(ptr, length));
    }
    static PA_FORCE_INLINE void accumulate_partial(__m256& out0, __m256 k0, const float* ptr, size_t length){
        accumulate(out0, k0, load_partial(ptr, length));
    }
};


void compute_spike_kernel_x86_AVX2(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
){
    compute_spike_kernel<Context_x86_AVX2>(out, in, lengthI, kernel, lengthK);
}



}
}
}
#endif
