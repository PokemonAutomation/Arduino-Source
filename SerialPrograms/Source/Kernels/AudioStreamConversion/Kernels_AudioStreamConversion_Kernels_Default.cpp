/*  Audio Stream Conversion Kernels (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Kernels_AudioStreamConversion_TestInfra.h"
#include "Kernels_AudioStreamConversion_Kernels_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConversion{



template <typename Type>
void convert_from_GenericDefault(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type>
void convert_to_GenericDefault(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);


template <>
PA_FORCE_INLINE void convert_from_GenericDefault<uint8_t>(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const uint8_t* interleaved_stream
){
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            strided_stream[c * stride] = ((int)interleaved_stream[c] - 128) * (1.0f / 128.0f);
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}
template <>
PA_FORCE_INLINE void convert_to_GenericDefault<uint8_t>(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    uint8_t* interleaved_stream
){
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            float x = strided_stream[c * stride];
            x = std::clamp(x, -1.0f, 127.0f / 128.0f);
            x = x * 128.0f + 128.5f;
            interleaved_stream[c] = (uint8_t)x;
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}
template <>
PA_FORCE_INLINE void convert_from_GenericDefault<int16_t>(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const int16_t* interleaved_stream
){
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            float x = (float)interleaved_stream[c] * (1.0f / 32768.0f);
            strided_stream[c * stride] = x;
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}
template <>
PA_FORCE_INLINE void convert_to_GenericDefault<int16_t>(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    int16_t* interleaved_stream
){
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            float x = strided_stream[c * stride] * 32768.0f;
            x = std::min(x, 32767.0f);
            x = std::max(x, -32768.0f);
            x += x > 0 ? 0.5f : -0.5f;
            interleaved_stream[c] = (int16_t)x;
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}
template <>
PA_FORCE_INLINE void convert_from_GenericDefault<int32_t>(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const int32_t* interleaved_stream
){
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            float x = (float)interleaved_stream[c] * (1.0f / 2147483648.0f);
            strided_stream[c * stride] = x;
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}
template <>
PA_FORCE_INLINE void convert_to_GenericDefault<int32_t>(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    int32_t* interleaved_stream
){
    constexpr float SCALE = 2147483648.0f;
    constexpr float MAX_SAFE = 2147483520.0f; // 2^31 - 128
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            float x = strided_stream[c * stride] * SCALE;
            x = std::min(x, MAX_SAFE);
            x = std::max(x, -2147483648.0f);
            x += x > 0 ? 0.5f : -0.5f;
            interleaved_stream[c] = (int32_t)x;
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}
template <>
PA_FORCE_INLINE void convert_from_GenericDefault<float>(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const float* interleaved_stream
){
    if (channels == 1){
        memcpy(strided_stream, interleaved_stream, samples * sizeof(float));
        return;
    }
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            strided_stream[c * stride] = interleaved_stream[c];
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}
template <>
PA_FORCE_INLINE void convert_to_GenericDefault<float>(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    float* interleaved_stream
){
    if (channels == 1){
        memcpy(interleaved_stream, strided_stream, samples * sizeof(float));
        return;
    }
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            interleaved_stream[c] = strided_stream[c * stride];
        }
        interleaved_stream += channels;
        strided_stream++;
    }
}



template <typename Type, size_t channels>
PA_NO_INLINE void convert_from_Default(
    size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
){
    convert_from_GenericDefault<Type>(channels, samples, stride, strided_stream, interleaved_stream);
}
template <typename Type, size_t channels>
PA_NO_INLINE void convert_to_Default(
    size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
){
    convert_to_GenericDefault<Type>(channels, samples, stride, strided_stream, interleaved_stream);
}




template <typename Type>
void convert_from_Default(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
){
    //  Special case the lower channel counts so they can be force-inlined with
    //  the inner loop unrolled.
    switch (channels){
    case 1:
        convert_from_Default<Type, 1>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 2:
        convert_from_Default<Type, 2>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 3:
        convert_from_Default<Type, 3>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 4:
        convert_from_Default<Type, 4>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 5:
        convert_from_Default<Type, 5>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 6:
        convert_from_Default<Type, 6>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 7:
        convert_from_Default<Type, 7>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        convert_from_GenericDefault<Type>(channels, samples, stride, strided_stream, interleaved_stream);
    }
}
template <typename Type>
void convert_to_Default(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
){
    //  Special case the lower channel counts so they can be force-inlined with
    //  the inner loop unrolled.
    switch (channels){
    case 1:
        convert_to_Default<Type, 1>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 2:
        convert_to_Default<Type, 2>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 3:
        convert_to_Default<Type, 3>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 4:
        convert_to_Default<Type, 4>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 5:
        convert_to_Default<Type, 5>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 6:
        convert_to_Default<Type, 6>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 7:
        convert_to_Default<Type, 7>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        convert_to_GenericDefault<Type>(channels, samples, stride, strided_stream, interleaved_stream);
    }
}






template <typename Type>
class Test_Kernel_Default : public Test_Kernel<Type>{
public:
    Test_Kernel_Default(size_t channels, size_t samples)
        : Test_Kernel<Type>(
            "Default",
            channels, samples,
            convert_from_Default<Type>,
            convert_to_Default<Type>
        )
    {}
};



void add_tests_Default(UnitTestDatabase& database){
    for (size_t c = 1; c <= 8; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_Kernel_Default<uint8_t>>(c, samples);
            database.add<Test_Kernel_Default<int16_t>>(c, samples);
            database.add<Test_Kernel_Default<int32_t>>(c, samples);
            database.add<Test_Kernel_Default<float>>(c, samples);
        }
    }
}


}
}
}
