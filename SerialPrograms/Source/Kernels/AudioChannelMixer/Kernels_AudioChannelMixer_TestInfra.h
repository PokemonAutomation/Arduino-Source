/*  Audio Channel Mixer Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
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





//
//  Warning, this file is compiled in multiple ISA contexts.
//  We cannot define the functions here since they will be compiled differently.
//  If a higher (AVX512) version wins out in the linker, all versions will use
//  it and crash on older CPUs.
//
class Test : public UnitTest{
public:
    Test(
        const std::string& arch_str,
        size_t in_channels,
        size_t out_channels,
        size_t samples,
        Convert convert0,
        Convert convert1
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

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
