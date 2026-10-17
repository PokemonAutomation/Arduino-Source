/*  Audio Pipeline - Raw<->Working Round-Trip Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_AudioPipeline_RoundTrip_TestInfra_H
#define PokemonAutomation_Kernels_AudioPipeline_RoundTrip_TestInfra_H

#include "Kernels_AudioPipeline.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{




//
//  Pushes random samples through a RawAudio2WorkingAudio node and a
//  WorkingAudio2RawAudio node (with reciprocal multipliers, so the
//  round trip is lossless), then compares the resulting raw samples
//  to the input.
//
class RoundTripTest : public UnitTest{
public:
    RoundTripTest(
        SampleFormat sample_type,
        size_t channels,
        size_t samples,
        float from_multiplier = 1.0f
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const SampleFormat m_sample_type;
    const size_t m_channels;
    const size_t m_samples;
    const float m_from_multiplier;
};




}
}
}
#endif
