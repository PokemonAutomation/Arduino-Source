/*  Audio Channel Mixer (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioChannelMixer_Default_H
#define PokemonAutomation_Kernels_AudioChannelMixer_Default_H

#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



void mix_channels_Default(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
);



PA_FORCE_INLINE void mix_channels_GenericDefault(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    for (size_t s = 0; s < samples; s++){
        for (size_t o = 0; o < out_channels; o++){
            float x = 0;
            for (size_t i = 0; i < in_channels; i++){
                x += in[in_stride * i] * matrix[i + in_channels * o];
            }
            out[out_stride * o] = x;
        }
        in++;
        out++;
    }
}


template <size_t in_channels, size_t out_channels>
inline void mix_channels_Default(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
);
template <>
inline void mix_channels_Default<1, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float i0o0 = matrix[0];
    for (size_t s = 0; s < samples; s++){
        float in0 = in[0];
        out[0] = in0 * i0o0;
        in++;
        out++;
    }
}
template <>
inline void mix_channels_Default<1, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float i0o0 = matrix[0];
    float i0o1 = matrix[1];
    for (size_t s = 0; s < samples; s++){
        float in0 = in[0];
        out[out_stride * 0] = in0 * i0o0;
        out[out_stride * 1] = in0 * i0o1;
        in++;
        out++;
    }
}
template <>
inline void mix_channels_Default<2, 1>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float i0o0 = matrix[0];
    float i1o0 = matrix[1];
    for (size_t s = 0; s < samples; s++){
        out[out_stride * 0] = in[0] * i0o0 + in[1] * i1o0;
        in++;
        out++;
    }
}
template <>
inline void mix_channels_Default<2, 2>(
    c_ptr<float> matrix, size_t samples,
    size_t out_stride, r_ptr<float> out,
    size_t in_stride, c_ptr<float> in
){
    float i0o0 = matrix[0];
    float i1o0 = matrix[1];
    float i0o1 = matrix[2];
    float i1o1 = matrix[3];
    for (size_t s = 0; s < samples; s++){
        float in0 = in[0];
        float in1 = in[1];
        out[out_stride * 0] = in0 * i0o0 + in1 * i1o0;
        out[out_stride * 1] = in0 * i0o1 + in1 * i1o1;
        in++;
        out++;
    }
}



}
}
}
#endif
