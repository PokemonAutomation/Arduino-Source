/*  Audio Stream Converter Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <type_traits>
#include <random>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/PrettyPrint.h"
#include "Kernels_AudioStreamConverter_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{




template <typename Type>
Test<Type>::Test(
    const std::string& arch_str,
    size_t samples,
    ConvertFrom<Type> convert_from,
    ConvertTo<Type> convert_to,
    float from_multiplier,
    float to_multiplier
)
    : UnitTest(make_name(arch_str, samples, from_multiplier, to_multiplier))
    , m_samples(samples)
    , m_convert_from(convert_from)
    , m_convert_to(convert_to)
    , m_from_multiplier(from_multiplier)
    , m_to_multiplier(to_multiplier)
{}
template <typename Type>
UnitTestResult Test<Type>::run(Logger& logger, CancellableScope& scope) const{
    if (m_samples < 2){
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Test size is too small.");
    }

    AlignedVector<Type> ext0(m_samples);
    AlignedVector<Type> ext1(m_samples);
    AlignedVector<float> native(m_samples);

    std::mt19937 generator(42);

    std::uniform_int_distribution<long long> distribution(
        std::numeric_limits<Type>::min(),
        std::numeric_limits<Type>::max()
    );
    ext0[0] = std::numeric_limits<Type>::min();
    ext0[1] = std::numeric_limits<Type>::max();
    if constexpr (std::is_same_v<Type, int32_t>){
        ext0[0] &= 0xfffff000;
        ext0[1] &= 0xfffff000;
    }
    for (size_t c = 2; c < m_samples; c++){
        ext0[c] = (Type)distribution(generator);
        //  Truncate some precision so the round-trip is lossless.
        if constexpr (std::is_same_v<Type, int32_t>){
            ext0[c] &= 0xfffff000;
        }
    }

    m_convert_from(native.data(), ext0.data(), m_samples, m_from_multiplier);
    m_convert_to(native.data(), ext1.data(), m_samples, m_to_multiplier);

    //  Check for errors.
    for (size_t c = 0; c < m_samples; c++){
        if (ext0[c] != ext1[c]){
            return "Mismatch at " + std::to_string(c) + ": " +
                std::to_string(ext0[c]) + " != " + std::to_string(ext1[c]);
        }
    }

    return true;
}
template <typename Type>
std::string Test<Type>::make_name(
    const std::string& arch_str,
    size_t samples,
    float from_multiplier,
    float to_multiplier
){
    std::string name = "Kernels::AudioStreamConverter - " + typestr() + "_" + arch_str +
        "(" + std::to_string(samples) + ")";
    if (from_multiplier != 1.0f || to_multiplier != 1.0f){
        name += " [from " + tostr_default(from_multiplier) +
            " / to " + tostr_default(to_multiplier) + "]";
    }
    return name;
}
template <typename Type>
std::string Test<Type>::typestr(){
    if constexpr (std::is_same_v<Type, uint8_t>){
        return "u8";
    }else if constexpr (std::is_same_v<Type, int16_t>){
        return "s16";
    }else if constexpr (std::is_same_v<Type, int32_t>){
        return "s32";
    }else{
        static_assert(false);
    }
}


template class Test<uint8_t>;
template class Test<int16_t>;
template class Test<int32_t>;




}
}
}
