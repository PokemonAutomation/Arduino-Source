/*  Scale Invariant Matrix Match (x86 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_SpikeConvolution_Routines.h"
#include "Kernels_SpikeConvolution_TestInfra.h"

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
        return _mm512_maskz_loadu_ps(mask, ptr);
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



void compute_spike_kernel_Default(float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);

class Test_x86_AVX512 : public Test{
public:
    Test_x86_AVX512(size_t lengthK)
        : Test(
            "x86_AVX512",
            lengthK,
            compute_spike_kernel_Default,
            compute_spike_kernel_x86_AVX512
        )
    {}
};
void add_tests_x86_AVX512(UnitTestDatabase& database){
    for (size_t lengthK = 1; lengthK <= 32; lengthK++){
        database.add<Test_x86_AVX512>(lengthK);
    }
}



}
}
}
#endif
