/*  Audio Channel Interleave Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_AudioChannelInterleave_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelInterleave{



Test::Test(
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
UnitTestResult Test::run(Logger& logger, CancellableScope& scope) const{
    size_t total = m_channels * m_samples;

    //  Every buffer ends right before a guard page, so a kernel that reads or
    //  writes past the end faults on every run.
    GuardPageBuffer<float> interleaved0(total);
    GuardPageBuffer<float> interleaved1(total);
    GuardPageBuffer<float> strided(total);

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


}
}
}
