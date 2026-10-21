/*  Audio Channel Mixer Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_AudioChannelMixer_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



Test::Test(
    const std::string& arch_str,
    size_t in_channels,
    size_t out_channels,
    size_t samples,
    Convert convert0,
    Convert convert1
)
    : UnitTest(
        "Kernels::AudioChannelMixer - " + arch_str +
        "(" + std::to_string(in_channels) + " -> " + std::to_string(out_channels) + " x " + std::to_string(samples) + ")"
    )
    , m_in_channels(in_channels)
    , m_out_channels(out_channels)
    , m_samples(samples)
    , m_convert0(convert0)
    , m_convert1(convert1)
{}

UnitTestResult Test::run(Logger& logger, CancellableScope& scope) const{
    std::vector<float> matrix(m_in_channels * m_out_channels);

    //  The input and both outputs end right before a guard page. A tail that
    //  reads or writes past the end then faults on every run, not just when
    //  the heap happens to end there.
    GuardPageBuffer<float> in(m_samples * m_in_channels);
    GuardPageBuffer<float> out0(m_samples * m_out_channels);
    GuardPageBuffer<float> out1(m_samples * m_out_channels);

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
    for (size_t c = 0; c < m_samples * m_out_channels; c++){
        if (out0[c] != out1[c]){
            return "Mismatch at " + std::to_string(c) + ": " +
                std::to_string(out0[c]) + " != " + std::to_string(out1[c]);
        }
    }

    return true;
}




}
}
}
