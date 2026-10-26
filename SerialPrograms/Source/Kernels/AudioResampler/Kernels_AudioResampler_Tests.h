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
        size_t sample_block,
        size_t tap_block,
        size_t out_index
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const KernelFn m_kernel_fn;
    const size_t m_in_rate;
    const size_t m_out_rate;
    const uint32_t m_taps;
    const size_t m_block;
    const size_t m_tap_block;
    const size_t m_out_index;
};



//
//  Runs one block of the kernel chosen by get_block_kernel() on
//  deterministic input and compares every output sample, in ULPs, to a
//  scalar reference. This validates that the dispatch result is a kernel
//  that is actually valid for the rate ratio and tap count.
//
class BlockDispatchTest : public UnitTest{
public:
    BlockDispatchTest(
        size_t in_sample_rate,
        size_t out_sample_rate,
        uint32_t taps
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const size_t m_in_rate;
    const size_t m_out_rate;
    const uint32_t m_taps;
};



//
//  Checks that WeightTable::time_shift() is the 1st-tap to median-tap
//  distance in input-sample seconds, and that the table actually
//  reconstructs at that shifted time: a delta at input sample p0 must
//  produce, at every output sample c, the windowed sinc centered on
//  input time p0 evaluated at c * in_rate / out_rate + (taps - 1) / 2
//  input samples.
//
class WeightTableTimeShiftTest : public UnitTest{
public:
    WeightTableTimeShiftTest(
        size_t in_sample_rate,
        size_t out_sample_rate,
        uint32_t taps
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    const size_t m_in_rate;
    const size_t m_out_rate;
    const uint32_t m_taps;
};



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
