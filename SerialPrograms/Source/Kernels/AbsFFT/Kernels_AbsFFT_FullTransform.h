/*  ABS FFT Full Transform
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_FullTransform_H
#define PokemonAutomation_Kernels_AbsFFT_FullTransform_H

#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


template <typename Context>
void fft_abs_scalar(const TwiddleTable<Context>& table, int k, float* abs, float* real);

template <typename Context>
void fft_abs(const TwiddleTable<Context>& table, int k, float* abs, float* real);



}
}
}
#endif
