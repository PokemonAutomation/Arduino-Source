/*  Audio Resampler Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioResampler_Tests_H
#define PokemonAutomation_Kernels_AudioResampler_Tests_H

#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioResampler{



void add_tests(UnitTestDatabase& database);



//
//  Warning, this file is compiled in multiple ISA contexts.
//  We cannot define the functions here since they will be compiled differently.
//  If a higher (AVX512) version wins out in the linker, all versions will use
//  it and crash on older CPUs.
//
//
//  Runs one block of a block kernel on deterministic input and compares
//  every output sample, in ULPs, to a scalar reference computed straight
//  from the weight table. The coefficients are shared with the reference,
//  so only floating-point rounding differences (FMA contraction,
//  accumulation order) are allowed, bounded by a small ULP tolerance.
//
class BlockKernelTest : public UnitTest{
public:
    using KernelFn = void(*)(const WeightTable&, size_t, const float*, size_t, float*);

    BlockKernelTest(
        const char* p_kernel,
        KernelFn p_kernel_fn,
        size_t in_sample_rate,
        size_t out_sample_rate,
        uint32_t taps,
        size_t block,
        size_t out_index
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const KernelFn m_kernel_fn;
    const size_t m_in_rate;
    const size_t m_out_rate;
    const uint32_t m_taps;
    const size_t m_block;
    const size_t m_out_index;
};



}
}
}
#endif
