/*  Spike Convolution
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_SpikeConvolution_H
#define PokemonAutomation_Kernels_SpikeConvolution_H

#include <stddef.h>

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


//  Compute the Spike Kernel
//      "out" is aligned to "PA_ALIGNMENT" bytes.
//      lengthI >= lengthK
//      "in"  is valid for lengthI
//      "out" is valid for (lengthI - lengthK + 1) rounded up to PA_ALIGNMENT/sizeof(float)
void compute_spike_kernel(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
);



}
}
}
#endif
