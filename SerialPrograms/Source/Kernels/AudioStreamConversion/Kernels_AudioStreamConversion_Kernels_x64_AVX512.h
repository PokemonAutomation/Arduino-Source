/*  Audio Stream Conversion Kernels (Generic Defaults)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioStreamConversion_Kernels_x64_AVX512_H
#define PokemonAutomation_Kernels_AudioStreamConversion_Kernels_x64_AVX512_H

#include <stddef.h>

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioStreamConversion{



template <typename Type, size_t channels>
void convert_from_x64_AVX512(
    size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type, size_t channels>
void convert_to_x64_AVX512(
    size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);



template <typename Type>
void convert_from_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type>
void convert_to_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);



void add_tests_x64_AVX512(UnitTestDatabase& database);



}
}
}
#endif
