/*  Audio Channel Mixer Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_AudioChannelMixer_TestInfra_H
#define PokemonAutomation_Kernels_AudioChannelMixer_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{





using Convert = void (*)(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
);





class Test : public UnitTest{
public:
    Test(
        const std::string& arch_str,
        size_t in_channels,
        size_t out_channels,
        size_t samples,
        Convert convert0,
        Convert convert1
    )
        : UnitTest(
            "Kernels::AudioStreamMixer - " + arch_str +
            "(" + std::to_string(in_channels) + " -> " + std::to_string(out_channels) + " x " + std::to_string(samples) + ")"
        )
        , m_in_channels(in_channels)
        , m_out_channels(out_channels)
        , m_samples(samples)
        , m_convert0(convert1)
        , m_convert1(convert1)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        std::vector<float> matrix(m_in_channels * m_out_channels);
        AlignedVector<float> in(m_samples * m_in_channels);
        AlignedVector<float> out0(m_samples * m_out_channels);
        AlignedVector<float> out1(m_samples * m_out_channels);

        std::mt19937 generator(42);

        std::uniform_int_distribution<long long> distribution(-128, 128);
        for (size_t c = 0; c < m_in_channels * m_out_channels; c++){
            matrix[c] = (float)distribution(generator);
        }
        for (size_t c = 0; c < m_samples * m_in_channels; c++){
            in[c] = (float)distribution(generator);
        }

        m_convert0(
            matrix.data(),
            m_samples,
            m_out_channels, m_samples, out0.data(),
            m_in_channels, m_samples, in.data()
        );
        m_convert1(
            matrix.data(),
            m_samples,
            m_out_channels, m_samples, out1.data(),
            m_in_channels, m_samples, in.data()
        );

        //  Check for errors.
        for (size_t c = 0; c < m_samples; c++){
            if (out0[c] != out1[c]){
                return "Mismatch at " + std::to_string(c) + ": " +
                    std::to_string(out0[c]) + " != " + std::to_string(out1[c]);
            }
        }

        return true;
    }

private:
    size_t m_in_channels;
    size_t m_out_channels;
    size_t m_samples;
    Convert m_convert0;
    Convert m_convert1;
};




}
}
}
#endif
