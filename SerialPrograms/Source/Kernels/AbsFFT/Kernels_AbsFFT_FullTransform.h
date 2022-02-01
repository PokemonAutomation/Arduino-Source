/*  ABS FFT Full Transform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_FullTransform_H
#define PokemonAutomation_Kernels_AbsFFT_FullTransform_H

#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


void fft_abs_scalar(const TwiddleTable& table, int k, float* abs, float* real);
void fft_abs(const TwiddleTable& table, int k, float* abs, float* real);



}
}
}
#endif
