/*  Audio Channel Interleave Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioChannelInterleave_TestInfra_H
#define PokemonAutomation_Kernels_AudioChannelInterleave_TestInfra_H

#include <cstddef>
#include "Common/Compiler.h"
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
        size_t channels, size_t samples,
        ConvertFrom convert_from,
        ConvertTo convert_to
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

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
