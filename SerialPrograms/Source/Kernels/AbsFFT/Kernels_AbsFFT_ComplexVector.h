/*  ABS FFT Complex Transform (Vector)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_ComplexVector_H
#define PokemonAutomation_Kernels_AbsFFT_ComplexVector_H

#include "Kernels_AbsFFT_Arch.h"
#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


void base_transform(const TwiddleTable& table, vtype* T);
void fft_complex_tk(const TwiddleTable& table, int k, vtype* T);


}
}
}
#endif
