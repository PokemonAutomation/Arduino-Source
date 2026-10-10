/*  Audio Stream Converter Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <type_traits>
#include <random>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"

#ifndef PokemonAutomation_Kernels_AudioStreamConverter_TestInfra_H
#define PokemonAutomation_Kernels_AudioStreamConverter_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{





template <typename Type>
using ConvertFrom = void (*)(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
);
template <typename Type>
using ConvertTo = void (*)(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
);






template <typename Type>
class Test_Kernel : public UnitTest{
public:
    Test_Kernel(
        const std::string& arch_str,
        size_t channels, size_t samples,
        ConvertFrom<Type> convert_from,
        ConvertTo<Type> convert_to
    )
        : UnitTest(
            "Kernels::AudioStreamConverter - convert_to/from_" + typestr() + "_" + arch_str +
            "(ch = " + std::to_string(channels) + ", samples=" + std::to_string(samples) + ")"
        )
        , m_channels(channels)
        , m_samples(samples)
        , m_convert_from(convert_from)
        , m_convert_to(convert_to)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        size_t total = m_channels * m_samples;
        if (total < 2){
            throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Test size is too small.");
        }

        AlignedVector<Type> interleaved0(total);
        AlignedVector<Type> interleaved1(total);
        AlignedVector<float> strided(total);

        std::mt19937 generator(42);

        if constexpr (std::is_floating_point_v<Type>){
            std::uniform_real_distribution<Type> distribution(-1.0, +1.0);
            for (size_t c = 0; c < total; c++){
                interleaved0[c] = distribution(generator);
            }
        }else{
            std::uniform_int_distribution<long long> distribution(
                std::numeric_limits<Type>::min(),
                std::numeric_limits<Type>::max()
            );
            interleaved0[0] = std::numeric_limits<Type>::min();
            interleaved0[1] = std::numeric_limits<Type>::max();
            if constexpr (std::is_same_v<Type, int32_t>){
                interleaved0[0] &= 0xfffff000;
                interleaved0[1] &= 0xfffff000;
            }
            for (size_t c = 2; c < total; c++){
                interleaved0[c] = (Type)distribution(generator);
                //  Truncate some precision so the round-trip is lossless.
                if constexpr (std::is_same_v<Type, int32_t>){
                    interleaved0[c] &= 0xfffff000;
                }
            }
        }

        m_convert_from(m_channels, m_samples, m_samples, strided.data(), interleaved0.data());
        m_convert_to(m_channels, m_samples, m_samples, strided.data(), interleaved1.data());

        //  Check for errors.
        for (size_t c = 0; c < total; c++){
            if (interleaved0[c] != interleaved1[c]){
                return "Mismatch at " + std::to_string(c) + ": " +
                    std::to_string(interleaved0[c]) + " != " + std::to_string(interleaved1[c]);
            }
        }

        return true;
    }

    static std::string typestr(){
        if constexpr (std::is_same_v<Type, uint8_t>){
            return "u8";
        }else if constexpr (std::is_same_v<Type, int16_t>){
            return "s16";
        }else if constexpr (std::is_same_v<Type, int32_t>){
            return "s32";
        }else if constexpr (std::is_same_v<Type, float>){
            return "f32";
        }else{
            static_assert(false);
        }
    }

private:
    size_t m_channels;
    size_t m_samples;
    ConvertFrom<Type> m_convert_from;
    ConvertTo<Type> m_convert_to;
};




}
}
}
#endif
