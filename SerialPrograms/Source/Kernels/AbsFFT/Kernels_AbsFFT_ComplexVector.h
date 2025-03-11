/*  ABS FFT Complex Transform (Vector)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_ComplexVector_H
#define PokemonAutomation_Kernels_AbsFFT_ComplexVector_H

#include "Kernels_AbsFFT_TwiddleTable.h"
#include "Kernels_AbsFFT_Butterflies.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


template <typename Context>
void base_transform(const TwiddleTable<Context>& table, typename Context::vtype* T);


template <typename Context>
void fft_complex_tk(const TwiddleTable<Context>& table, int k, typename Context::vtype* T){
    if (k == Context::BASE_COMPLEX_TRANSFORM_K){
        base_transform<Context>(table, T);
        return;
    }
    if (k == Context::BASE_COMPLEX_TRANSFORM_K + 1){
        Butterflies<Context>::reduce2(table, k, T);
        size_t stride = 2 << (k - 1 - Context::VECTOR_K);
        base_transform<Context>(table, T + 0*stride);
        base_transform<Context>(table, T + 1*stride);
        return;
    }

    Butterflies<Context>::reduce4(table, k, T);
    k -= 2;
    size_t stride = 2 << (k - Context::VECTOR_K);
    fft_complex_tk<Context>(table, k, T + 0*stride);
    fft_complex_tk<Context>(table, k, T + 1*stride);
    fft_complex_tk<Context>(table, k, T + 2*stride);
    fft_complex_tk<Context>(table, k, T + 3*stride);
}


}
}
}
#endif
