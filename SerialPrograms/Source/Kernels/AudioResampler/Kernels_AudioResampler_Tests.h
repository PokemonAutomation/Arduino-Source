/*  Audio Resampler Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_AudioResampler_Tests_H
#define PokemonAutomation_Kernels_AudioResampler_Tests_H

#include "Kernels_AudioResampler.h"

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioResampler{



void add_tests(UnitTestDatabase& database);



//
//  Pushes random input through an AudioResampler in chunks and compares
//  every produced output sample to a scalar reference computed from the
//  weight table.
//
class ResamplerStreamTest : public UnitTest{
public:
    ResamplerStreamTest(
        size_t in_sample_rate,
        size_t out_sample_rate,
        size_t channels,
        size_t samples,
        size_t push_size,
        uint32_t taps,
        double beta
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const size_t m_in_rate;
    const size_t m_out_rate;
    const size_t m_channels;
    const size_t m_samples;
    const size_t m_push;
    const uint32_t m_taps;
    const double m_beta;
};



}
}
}
#endif
