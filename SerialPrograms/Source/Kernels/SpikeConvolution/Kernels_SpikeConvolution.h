/*  Spike Convolution
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_SpikeConvolution_H
#define PokemonAutomation_Kernels_SpikeConvolution_H

#include <stddef.h>

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace SpikeConvolution{


//
//  Compute the "spike kernel": the valid cross-correlation of "in" with
//  "kernel",
//
//      out[j] = sum over m of kernel[m] * in[j + m],
//              j = 0 .. lengthI - lengthK
//
//  The kernel used in production (buildSpikeKernel in SpectrogramMatcher.cpp)
//  is an antisymmetric triangular tent spanning about 199 Hz (at 48 kHz with
//  2048 frequency bins, 18 taps: -4, -3, -2, -1, 0, 1, 2, 3, 4, 4, 3, 2, 1,
//  0, -1, -2, -3, -4; it sums to zero), so out[j] measures the local slope
//  of "in" along the frequency axis: zero where "in" is flat, large at sharp
//  rises and falls. SpectrogramMatcher's SPIKE_CONV mode uses this to match
//  audio templates (shiny cries, item pickups, ...) against a recording.
//
//      - "out" must be aligned to PA_ALIGNMENT bytes.
//      - lengthI >= lengthK.
//      - "in" is valid for lengthI floats.
//      - "out" is written for lengthI - lengthK + 1 values, but the SIMD
//        paths write in 4-lane vectors, so the last store may also write up
//        to 3 padding values past that. "out" must be valid for
//        lengthI - lengthK + 1 rounded up to 16 floats.
//
void compute_spike_kernel(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
);



void add_tests(UnitTestDatabase& database);



}
}
}
#endif
