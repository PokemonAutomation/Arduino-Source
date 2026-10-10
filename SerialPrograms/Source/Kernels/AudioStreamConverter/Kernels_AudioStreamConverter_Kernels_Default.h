/*  Audio Stream Converter Kernels (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioStreamConverter_Kernels_Default_H
#define PokemonAutomation_Kernels_AudioStreamConverter_Kernels_Default_H

#include "Common/Compiler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioStreamConverter{


template <typename Type, size_t channels>
void convert_from_Default(
    size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type, size_t channels>
void convert_to_Default(
    size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);



template <typename Type>
void convert_from_Default(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type>
void convert_to_Default(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);







PA_FORCE_INLINE void convolve_channels_Default(
    const float* matrix, size_t samples,
    size_t out_channels, size_t out_stride, float* out,
    size_t in_channels, size_t in_stride, const float* in
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




void add_tests_Default(UnitTestDatabase& database);




}
}
}
#endif
