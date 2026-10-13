/*  Audio Pipeline - ChannelMixer Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_AudioPipeline_ChannelMixer_TestInfra_H
#define PokemonAutomation_Kernels_AudioPipeline_ChannelMixer_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{




//
//  Pushes random integer samples through a ChannelMixer with an identity
//  weight matrix and verifies the output is unchanged.
//
class MixerIdentityTest : public UnitTest{
public:
    MixerIdentityTest(size_t channels, size_t samples);

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const size_t m_channels;
    const size_t m_samples;
};




//
//  Pushes random integer samples through a 1->2 ChannelMixer, then through
//  a 2->1 averaging ChannelMixer, and verifies the round-trip is exact.
//
class MixerRoundTripTest : public UnitTest{
public:
    MixerRoundTripTest(size_t samples);

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const size_t m_samples;
};




}
}
}
#endif
