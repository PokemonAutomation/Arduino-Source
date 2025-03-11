/*  Audio Stream Conversion (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "AudioStreamConversion.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConversion{



void convert_audio_uint8_to_float_Default(float* f, const uint8_t* i, size_t length, float output_multiplier){
    const float SCALE = output_multiplier / 127.f;
    for (size_t c = 0; c < length; c++){
        float x = (float)i[c] * SCALE - output_multiplier;
        x = std::max(x, -1.0f);
        x = std::min(x, 1.0f);
        f[c] = x;
    }
}
void convert_audio_float_to_uint8_Default(uint8_t* i, const float* f, size_t length){
    for (size_t c = 0; c < length; c++){
        float r = (f[c] + 1.0f) * 127.f;
        r = std::min(r, 255.f);
        r = std::max(r, 0.f);
        r += 0.5;
        i[c] = (uint8_t)r;
    }
}

void convert_audio_sint16_to_float_Default(float* f, const int16_t* i, size_t length, float output_multiplier){
    const float SCALE = output_multiplier / 32767.f;
    for (size_t c = 0; c < length; c++){
        float x = (float)i[c] * SCALE;
        x = std::max(x, -1.0f);
        x = std::min(x, 1.0f);
        f[c] = x;
    }
}
void convert_audio_float_to_sint16_Default(int16_t* i, const float* f, size_t length){
    for (size_t c = 0; c < length; c++){
        float r = f[c] * 32767.f;
        r = std::min(r, 32767.f);
        r = std::max(r, -32768.f);
        r += r > 0 ? 0.5f : -0.5f;
        i[c] = (int16_t)r;
    }
}

void convert_audio_sint32_to_float_Default(float* f, const int32_t* i, size_t length, float output_multiplier){
    const float SCALE = output_multiplier / 2147483647.f;
    for (size_t c = 0; c < length; c++){
        float x = (float)i[c] * SCALE;
        x = std::max(x, -1.0f);
        x = std::min(x, 1.0f);
        f[c] = x;
    }
}
void convert_audio_float_to_sint32_Default(int32_t* i, const float* f, size_t length){
    for (size_t c = 0; c < length; c++){
        double r = f[c] * 2147483647.;
        r = std::min(r, 2147483647.);
        r = std::max(r, -2147483648.);
        r += r > 0 ? 0.5 : -0.5;
        i[c] = (int32_t)r;
    }
}




}
}
}
