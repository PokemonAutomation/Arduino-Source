/*  Audio Resampler Block Kernel (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BlockKernel_Default_H
#define PokemonAutomation_Kernels_BlockKernel_Default_H

#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



inline void run_bidirectional_block8_Default(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
){
    in_samples -= in_index;

    const uint32_t* index = table.in_sample_index() + out_index;
    const float* in0 = in_samples + index[0];
    const float* in1 = in_samples + index[1];
    const float* in2 = in_samples + index[2];
    const float* in3 = in_samples + index[3];
    const float* in4 = in_samples + index[4];
    const float* in5 = in_samples + index[5];
    const float* in6 = in_samples + index[6];
    const float* in7 = in_samples + index[7];

    float r0 = 0;
    float r1 = 0;
    float r2 = 0;
    float r3 = 0;
    float r4 = 0;
    float r5 = 0;
    float r6 = 0;
    float r7 = 0;

    size_t tap = 0;
    size_t stop = table.taps();
    do{
        const float* tap_weights = table.tap_start(tap) + out_index;

        r0 += in0[tap] * tap_weights[0];
        r1 += in1[tap] * tap_weights[1];
        r2 += in2[tap] * tap_weights[2];
        r3 += in3[tap] * tap_weights[3];
        r4 += in4[tap] * tap_weights[4];
        r5 += in5[tap] * tap_weights[5];
        r6 += in6[tap] * tap_weights[6];
        r7 += in7[tap] * tap_weights[7];

        tap++;
    }while (tap < stop);

    out_samples[0] = r0;
    out_samples[1] = r1;
    out_samples[2] = r2;
    out_samples[3] = r3;
    out_samples[4] = r4;
    out_samples[5] = r5;
    out_samples[6] = r6;
    out_samples[7] = r7;
}



}
}
}
#endif
