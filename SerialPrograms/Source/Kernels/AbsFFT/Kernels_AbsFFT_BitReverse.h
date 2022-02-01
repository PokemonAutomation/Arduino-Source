/*  ABS FFT Bit Reverse
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_BitReverse_H
#define PokemonAutomation_Kernels_AbsFFT_BitReverse_H

#include "Kernels_AbsFFT_Arch.h"
#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


//  32-bit Granular
void interleave_f32(int len_k, float* out, const float* in);


//  64-bit Granular
void bitreverse_u64_ip(int len_k, uint64_t* data, uint64_t* temp);  //  in-place
void bitreverse_u64_np(int len_k, uint64_t* out, uint64_t* in);     //  out-of-place

inline void bitreverse_f32v1_ip(int len_k, float* data, float* temp){
    bitreverse_u64_ip(len_k - 1, (uint64_t*)data, (uint64_t*)temp);
}


}
}
}
#endif
