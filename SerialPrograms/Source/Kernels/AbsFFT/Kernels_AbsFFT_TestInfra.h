/*  ABS FFT Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_AbsFFT_TestInfra_H
#define PokemonAutomation_Kernels_AbsFFT_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



//
//  Signature of fft_abs_Default(), fft_abs_x86_SSE41(), etc.
//
using FFTAbs = void (*)(int k, float* abs, float* real, const float* weights);



//
//  Runs the transform twice on identical random input data, once with
//  fft_abs_Default() and once with an ISA-specific implementation, and
//  compares the two outputs in ULP space. The Default path is the source of
//  truth: an ISA result may differ by a few ULPs since the implementations
//  order their floating-point operations differently.
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
        int k,
        bool use_weights,
        FFTAbs fft0,
        FFTAbs fft1
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

private:
    int m_k;
    bool m_use_weights;
    FFTAbs m_fft0;
    FFTAbs m_fft1;
};



}
}
}
#endif
