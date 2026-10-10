/*  Audio Stream Converter Kernels (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Kernels_AudioStreamConverter_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{



template <typename ExternalType>
void convert_from_Default(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
void convert_to_Default(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);
void add_tests_Default(UnitTestDatabase& database);



template <>
void convert_from_Default<uint8_t>(r_ptr<float> native, c_ptr<uint8_t> ext, size_t samples){
    for (size_t c = 0; c < samples; c++){
        native[c] = ((int)ext[c] - 128) * (1.0f / 128.0f);
    }
}
template <>
void convert_to_Default<uint8_t>(c_ptr<float> native, r_ptr<uint8_t> ext, size_t samples, float multiplier){
    for (size_t c = 0; c < samples; c++){
        float x = native[c] * multiplier;
        x = std::clamp(x, -1.0f, 127.0f / 128.0f);
        x = x * 128.0f + 128.5f;
        ext[c] = (uint8_t)x;
    }
}
template <>
void convert_from_Default<int16_t>(r_ptr<float> native, c_ptr<int16_t> ext, size_t samples){
    for (size_t c = 0; c < samples; c++){
        native[c] = (float)ext[c] * (1.0f / 32768.0f);
    }
}
template <>
void convert_to_Default<int16_t>(c_ptr<float> native, r_ptr<int16_t> ext, size_t samples, float multiplier){
    multiplier *= 32768.0f;
    for (size_t c = 0; c < samples; c++){
        float x = native[c] * multiplier;
        x = std::min(x, 32767.0f);
        x = std::max(x, -32768.0f);
        x += x > 0 ? 0.5f : -0.5f;
        ext[c] = (int16_t)x;
    }
}
template <>
void convert_from_Default<int32_t>(r_ptr<float> native, c_ptr<int32_t> ext, size_t samples){
    for (size_t c = 0; c < samples; c++){
        native[c] = (float)ext[c] * (1.0f / 2147483648.0f);
    }
}
template <>
void convert_to_Default<int32_t>(c_ptr<float> native, r_ptr<int32_t> ext, size_t samples, float multiplier){
    constexpr float SCALE = 2147483648.0f;
    constexpr float MAX_SAFE = 2147483520.0f; // 2^31 - 128
    multiplier *= SCALE;
    for (size_t c = 0; c < samples; c++){
        float x = native[c] * multiplier;
        x = std::min(x, MAX_SAFE);
        x = std::max(x, -2147483648.0f);
        x += x > 0 ? 0.5f : -0.5f;
        ext[c] = (int32_t)x;
    }
}






template <typename Type>
class Test_Default : public Test<Type>{
public:
    Test_Default(size_t samples)
        : Test<Type>(
            "Default",
            samples,
            convert_from_Default<Type>,
            convert_to_Default<Type>
        )
    {}
};
void add_tests_Default(UnitTestDatabase& database){
    for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
        database.add<Test_Default<uint8_t>>(samples);
        database.add<Test_Default<int16_t>>(samples);
        database.add<Test_Default<int32_t>>(samples);
    }
}


}
}
}
