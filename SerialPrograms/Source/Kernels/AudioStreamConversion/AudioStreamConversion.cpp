/*  Audio Stream Conversion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "AudioStreamConversion.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConversion{



void convert_audio_uint8_to_float_Default(float* f, const uint8_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_uint8_Default(uint8_t* i, const float* f, size_t length);
void convert_audio_sint16_to_float_Default(float* f, const int16_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_sint16_Default(int16_t* i, const float* f, size_t length);
void convert_audio_sint32_to_float_Default(float* f, const int32_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_sint32_Default(int32_t* i, const float* f, size_t length);

void convert_audio_uint8_to_float_x86_SSE41(float* f, const uint8_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_uint8_x86_SSE41(uint8_t* i, const float* f, size_t length);
void convert_audio_sint16_to_float_x86_SSE41(float* f, const int16_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_sint16_x86_SSE41(int16_t* i, const float* f, size_t length);
void convert_audio_sint32_to_float_x86_SSE2(float* f, const int32_t* i, size_t length, float output_multiplier);
void convert_audio_float_to_sint32_x86_SSE2(int32_t* i, const float* f, size_t length);




void convert_audio_uint8_to_float(float* f, const uint8_t* i, size_t length, float output_multiplier){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_audio_uint8_to_float_x86_SSE41(f, i, length, output_multiplier);
        return;
    }
#endif
    convert_audio_uint8_to_float_Default(f, i, length, output_multiplier);
}
void convert_audio_float_to_uint8(uint8_t* i, const float* f, size_t length){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_audio_float_to_uint8_x86_SSE41(i, f, length);
        return;
    }
#endif
    convert_audio_float_to_uint8_Default(i, f, length);
}
void convert_audio_sint16_to_float(float* f, const int16_t* i, size_t length, float output_multiplier){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_audio_sint16_to_float_x86_SSE41(f, i, length, output_multiplier);
        return;
    }
#endif
    convert_audio_sint16_to_float_Default(f, i, length, output_multiplier);
}
void convert_audio_float_to_sint16(int16_t* i, const float* f, size_t length){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_audio_float_to_sint16_x86_SSE41(i, f, length);
        return;
    }
#endif
    convert_audio_float_to_sint16_Default(i, f, length);
}
void convert_audio_sint32_to_float(float* f, const int32_t* i, size_t length, float output_multiplier){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_audio_sint32_to_float_x86_SSE2(f, i, length, output_multiplier);
        return;
    }
#endif
    convert_audio_sint32_to_float_Default(f, i, length, output_multiplier);
}
void convert_audio_float_to_sint32(int32_t* i, const float* f, size_t length){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_audio_float_to_sint32_x86_SSE2(i, f, length);
        return;
    }
#endif
    convert_audio_float_to_sint32_Default(i, f, length);
}



}
}
}
