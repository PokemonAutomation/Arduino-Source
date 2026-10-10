/*  Audio Stream Converter Kernels (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioStreamConverter_Kernels_x64_AVX2_H
#define PokemonAutomation_Kernels_AudioStreamConverter_Kernels_x64_AVX2_H

#include <stddef.h>

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioStreamConverter{



template <typename Type, size_t channels>
void convert_from_x64_AVX2(
    size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type, size_t channels>
void convert_to_x64_AVX2(
    size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);



template <typename Type>
void convert_from_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type>
void convert_to_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);



void add_tests_x64_AVX2(UnitTestDatabase& database);



}
}
}
#endif
