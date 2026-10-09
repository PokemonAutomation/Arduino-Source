/*  Kaiser Window
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_KaiserWindow_H
#define PokemonAutomation_Kernels_KaiserWindow_H

#include "Common/Cpp/Containers/AlignedVector.h"

namespace PokemonAutomation{
namespace Kernels{
namespace KaiserWindow{


double bessel_i0(double x);
void make_kaiser_window(float* weights, size_t window, double beta = 9.0);


//  Get the Kaiser window weights for a window of 2^window_k.
const float* get_kaiser_window_k(int window_k);



}
}
}
#endif
