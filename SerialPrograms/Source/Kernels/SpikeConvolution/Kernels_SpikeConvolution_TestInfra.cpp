/*  Spike Convolution Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cstdio>
#include <limits>
#include <random>
#include "Common/Compiler.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_SpikeConvolution_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{



//
//  Bounds for the random data.
//
//  All values are small integers so that the kernel's math is exact in
//  32-bit float: every product |weight|*|data| is at most 8*512 = 4096,
//  and every output (a sum of at most 32 such products) is at most
//  32*4096 = 131072, both far below 2^24. Every intermediate sum is
//  exact as well, so the Default and ISA implementations must agree
//  bit-exactly, regardless of operation order or FMA contraction.
//
static constexpr int MAX_DATA = 512;
static constexpr int MAX_WEIGHT = 8;

//
//  Input lengths to cover: from the smallest allowed (K) up to 1000.
//
static constexpr size_t MAX_LENGTH_I = 1000;

//
//  Extra room in the Default output buffer: the kernel may write up to
//  PA_ALIGNMENT/sizeof(float) - 1 = 15 floats past the valid outputs
//  (padding of the final vector).
//
static constexpr size_t OUTPUT_PAD = 16;

//
//  The documented size of "out": the valid outputs rounded up to 16 floats.
//
static size_t output_capacity(size_t lengthO){
    return (lengthO + 15) & ~(size_t)15;
}



Test::Test(
    const std::string& arch_str,
    size_t lengthK,
    SpikeConvolutionFn conv0,
    SpikeConvolutionFn conv1
)
    : UnitTest(
        "Kernels::SpikeConvolution - " + arch_str +
        "(K=" + std::to_string(lengthK) + ")"
    )
    , m_lengthK(lengthK)
    , m_conv0(conv0)
    , m_conv1(conv1)
{}

UnitTestResult Test::run(Logger& logger, CancellableScope& scope) const{
    //
    //  "in", "kernel" and the output of the implementation under test each
    //  end right before a guard page. A read past the end of "in" or
    //  "kernel", or a write past the documented end of "out", faults on
    //  every run. The buffers are mapped once for the longest input. Each
    //  input length puts its data at the end of them.
    //
    const size_t max_capacityO = output_capacity(MAX_LENGTH_I - m_lengthK + 1);
    GuardPageBuffer<float> in_buffer(MAX_LENGTH_I);
    GuardPageBuffer<float> kernel(m_lengthK);
    GuardPageBuffer<float> out1_buffer(max_capacityO, PA_ALIGNMENT);

    for (size_t lengthI = m_lengthK; lengthI <= MAX_LENGTH_I; lengthI++){
        scope.throw_if_cancelled();
        const size_t lengthO = lengthI - m_lengthK + 1;
        const size_t capacityO = output_capacity(lengthO);

        //
        //  Fresh random data for each (K, N), with a deterministic seed.
        //  "in" holds exactly lengthI floats. "out1" holds exactly
        //  capacityO floats and starts 64-byte aligned (both capacities
        //  are multiples of 16 floats). It starts as NaN, so an output the
        //  implementation never writes shows up as a mismatch.
        //
        float* in = in_buffer.data() + MAX_LENGTH_I - lengthI;
        float* out1 = out1_buffer.data() + max_capacityO - capacityO;
        std::fill(out1, out1 + capacityO, std::numeric_limits<float>::quiet_NaN());
        AlignedVector<float> out0(lengthO + OUTPUT_PAD);

        std::mt19937 generator((unsigned int)(lengthI * 1000003u + m_lengthK * 7919u));
        std::uniform_int_distribution<int> data_distribution(-MAX_DATA, MAX_DATA);
        std::uniform_int_distribution<int> weight_distribution(-MAX_WEIGHT, MAX_WEIGHT);
        for (size_t c = 0; c < lengthI; c++){
            in[c] = (float)data_distribution(generator);
        }
        for (size_t c = 0; c < m_lengthK; c++){
            kernel[c] = (float)weight_distribution(generator);
        }

        m_conv0(out0.data(), in, lengthI, kernel.data(), m_lengthK);
        m_conv1(out1, in, lengthI, kernel.data(), m_lengthK);

        //
        //  Both implementations compute identical integer sums, so the
        //  results must agree bit-exactly.
        //
        for (size_t c = 0; c < lengthO; c++){
            if (out0[c] != out1[c]){
                char value0[32];
                char value1[32];
                std::snprintf(value0, sizeof(value0), "%.9g", (double)out0[c]);
                std::snprintf(value1, sizeof(value1), "%.9g", (double)out1[c]);
                return "Mismatch at input length " + std::to_string(lengthI) + ", output " +
                    std::to_string(c) + " of " + std::to_string(lengthO) +
                    ": " + value0 + " != " + value1;
            }
        }
    }
    return true;
}



}
}
}
