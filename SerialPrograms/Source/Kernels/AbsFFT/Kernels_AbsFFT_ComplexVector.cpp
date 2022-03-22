/*  ABS FFT Complex Transform (Vector)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_AbsFFT_Butterflies.h"
#include "Kernels_AbsFFT_ComplexVector.h"


#if 0
#elif defined PA_Kernels_AbsFFT_Arch_Default
#include "Kernels_AbsFFT_ComplexScalar.h"
namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
    void base_transform(const TwiddleTable& table, vtype* T){
        fft_complex_tk_scalar(table, Intrinsics::BASE_COMPLEX_TRANSFORM_K, T);
    }
    void fft_complex_tk(const TwiddleTable& table, int k, vtype* T){
        fft_complex_tk_scalar(table, k, T);
    }
}
}
}
#elif defined PA_Kernels_AbsFFT_Arch_x86_SSE41
#include "Kernels_AbsFFT_BaseTransform_x86_SSE41.h"
#elif defined PA_Kernels_AbsFFT_Arch_x86_AVX2
#include "Kernels_AbsFFT_BaseTransform_x86_AVX2.h"
#endif


#ifndef PA_Kernels_AbsFFT_Arch_Default
namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



void fft_complex_tk(const TwiddleTable& table, int k, vtype* T){
    if (k == Context::BASE_COMPLEX_TRANSFORM_K){
        base_transform(table, T);
        return;
    }
    if (k == Context::BASE_COMPLEX_TRANSFORM_K + 1){
        Butterflies<Context>::reduce2(table, k, T);
        size_t stride = 2 << (k - 1 - VECTOR_K);
        base_transform(table, T + 0*stride);
        base_transform(table, T + 1*stride);
        return;
    }

    Butterflies<Context>::reduce4(table, k, T);
    k -= 2;
    size_t stride = 2 << (k - VECTOR_K);
    fft_complex_tk(table, k, T + 0*stride);
    fft_complex_tk(table, k, T + 1*stride);
    fft_complex_tk(table, k, T + 2*stride);
    fft_complex_tk(table, k, T + 3*stride);
}



}
}
}
#endif
