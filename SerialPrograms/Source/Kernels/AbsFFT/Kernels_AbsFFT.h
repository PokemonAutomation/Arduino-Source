/*  ABS FFT
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_H
#define PokemonAutomation_Kernels_AbsFFT_H


namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{

//
//  Compute the FFT of "real" and store the absolute values of the
//  lower half into "abs".
//
//    - 2^k is the transform length.
//    - "abs" is the absolute values of the frequency domain. It has length 2^(k-1).
//    - "real" is the input time domain. It has length 2^k.
//    - "weights" is multiplied into "real" before the FFT. It has length 2^k.
//    - Both "real" and "abs" must be aligned to 64 bytes.
//
//  This operation is destructive on "real" even though it is an input.
//
void fft_abs(int k, float* abs, float* real, const float* weights = nullptr);



}
}
}
#endif
