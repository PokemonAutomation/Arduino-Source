/*  ABS FFT Complex Scalar
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_ComplexScalar_H
#define PokemonAutomation_Kernels_AbsFFT_ComplexScalar_H

#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


void reduce4_scalar(const TwiddleTable& table, int k, float* T);

void fft_complex_tk_scalar(const TwiddleTable& table, int k, float* T);



}
}
}
#endif
