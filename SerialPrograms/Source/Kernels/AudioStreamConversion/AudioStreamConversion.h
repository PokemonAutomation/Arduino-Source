/*  Audio Stream Conversion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioStreamConversion_H
#define PokemonAutomation_Kernels_AudioStreamConversion_H

#include <stddef.h>
#include <stdint.h>

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConversion{


void convert_audio_uint8_to_float(float* f, const uint8_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_uint8(uint8_t* i, const float* f, size_t length);

void convert_audio_sint16_to_float(float* f, const int16_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_sint16(int16_t* i, const float* f, size_t length);

void convert_audio_sint32_to_float(float* f, const int32_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_sint32(int32_t* i, const float* f, size_t length);




}
}
}
#endif
