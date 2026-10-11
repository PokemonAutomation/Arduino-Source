/*  Audio Channel Interleave Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioChannelInterleave_TestInfra_H
#define PokemonAutomation_Kernels_AudioChannelInterleave_TestInfra_H

#include <cstddef>
#include <random>
#include "Common/Compiler.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/TestRunners/UnitTest.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelInterleave{





using ConvertFrom = void (*)(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
using ConvertTo = void (*)(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);






class Test : public UnitTest{
public:
    Test(
        const std::string& arch_str,
        size_t channels, size_t samples,
        ConvertFrom convert_from,
        ConvertTo convert_to
    )
        : UnitTest(
            "Kernels::AudioChannelInterleave - " + arch_str +
            "(ch = " + std::to_string(channels) + ", samples=" + std::to_string(samples) + ")"
        )
        , m_channels(channels)
        , m_samples(samples)
        , m_convert_from(convert_from)
        , m_convert_to(convert_to)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        size_t total = m_channels * m_samples;

        AlignedVector<float> interleaved0(total);
        AlignedVector<float> interleaved1(total);
        AlignedVector<float> strided(total);

        std::mt19937 generator(42);

        std::uniform_real_distribution<float> distribution(-1.0, +1.0);
        for (size_t c = 0; c < total; c++){
            interleaved0[c] = distribution(generator);
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

private:
    size_t m_channels;
    size_t m_samples;
    ConvertFrom m_convert_from;
    ConvertTo m_convert_to;
};




}
}
}
#endif
