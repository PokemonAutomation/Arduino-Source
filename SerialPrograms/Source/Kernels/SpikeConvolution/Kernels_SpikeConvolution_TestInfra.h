/*  Spike Convolution Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_SpikeConvolution_TestInfra_H
#define PokemonAutomation_Kernels_SpikeConvolution_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{



//
//  Signature of compute_spike_kernel_Default(), compute_spike_kernel_x86_SSE41(), etc.
//
using SpikeConvolutionFn = void (*)(float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);



//
//  Runs the kernel twice on identical random input data, once with
//  compute_spike_kernel_Default() and once with the ISA-specific
//  implementation, and requires the two outputs to agree bit-exactly. The
//  Default path is the source of truth.
//
//  The input data and kernel weights are small integers, so every product
//  and every accumulated sum in the kernel is exactly representable in
//  32-bit float. The two implementations therefore must agree exactly,
//  regardless of operation order or FMA contraction.
//
//  For a given kernel length K, the test runs every input length
//  N = K..1000, with fresh random data for each N.
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
        size_t lengthK,
        SpikeConvolutionFn conv0,
        SpikeConvolutionFn conv1
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

private:
    size_t m_lengthK;
    SpikeConvolutionFn m_conv0;
    SpikeConvolutionFn m_conv1;
};



}
}
}
#endif
