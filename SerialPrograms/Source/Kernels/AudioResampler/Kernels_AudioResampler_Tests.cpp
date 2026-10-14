/*  Audio Resampler Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioResampler_BlockKernels.h"
#include "Kernels_AudioResampler_Tests.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



namespace{



//
//  Scalar reference for one output sample:
//      out[out_index + c] = sum_t weight[t][out_index + c] x
//          input[index[out_index + c] + t]
//
float block_reference(
    const WeightTable& table, const float* input, size_t out_index, size_t c
){
    const uint32_t* index = table.in_sample_index();
    const uint32_t base = index[out_index + c];
    float acc = 0;
    for (size_t t = 0; t < table.taps(); t++){
        acc += table.tap_start(t)[out_index + c] * input[base + t];
    }
    return acc;
}
AlignedVector<float> reference_lanes(
    const WeightTable& table, const float* input, size_t out_index, size_t block
){
    AlignedVector<float> out(block);
    for (size_t c = 0; c < block; c++){
        out[c] = block_reference(table, input, out_index, c);
    }
    return out;
}
std::string format_lanes(const float* v, size_t n){
    std::string s;
    const size_t count = n < 4 ? n : 4;
    for (size_t i = 0; i < count; i++){
        if (i > 0){
            s += ", ";
        }
        s += std::to_string(v[i]);
    }
    return s;
}


//
//  Number of ULPs between two floats, in a monotonic bit ordering.
//  Returns a distance larger than any real one if either value is NaN.
//
int64_t ulp_distance(float a, float b){
    if (a == b){
        return 0;
    }
    if (a != a || b != b){
        return INT64_MAX;
    }
    const int32_t ia = std::bit_cast<int32_t>(a);
    const int32_t ib = std::bit_cast<int32_t>(b);
    const int64_t ma = ia < 0 ? -(int64_t)(uint32_t)ia : ia;
    const int64_t mb = ib < 0 ? -(int64_t)(uint32_t)ib : ib;
    return ma > mb ? ma - mb : mb - ma;
}
//
//  The SIMD kernels use the same coefficients as the reference; only the
//  floating-point rounding can differ (FMA contraction, accumulation
//  order). The tolerance is 32 ULPs of the result magnitude, anchored at
//  1.0 so near-zero results keep an absolute floor. Any indexing or
//  coefficient bug shows up as an O(0.01) difference, far outside this.
//
bool within_ulp_tolerance(float got, float want, int64_t& ulp_diff){
    ulp_diff = ulp_distance(got, want);
    if (std::isinf(got) || std::isinf(want)){
        return got == want;
    }
    const float scale = std::max(std::max(std::fabs(got), std::fabs(want)), 1.0f);
    const float ulp = std::nextafterf(scale, INFINITY) - scale;
    return std::fabs(got - want) <= 32.0f * ulp;
}



}



//  Per-ISA test registration, defined in the matching block kernel
//  translation unit next to the kernel it tests.
void add_tests_x64_SSE2(UnitTestDatabase& database);
void add_tests_x64_FMA3(UnitTestDatabase& database);
void add_tests_x64_AVX512(UnitTestDatabase& database);
void add_tests_ARM64_NEON(UnitTestDatabase& database);




BlockKernelTest::BlockKernelTest(
    const char* p_kernel,
    KernelFn p_kernel_fn,
    size_t in_sample_rate,
    size_t out_sample_rate,
    uint32_t taps,
    size_t block,
    size_t out_index
)
    : UnitTest(
        "Kernels::AudioResampler - Block(" + std::string(p_kernel) +
        ", " + std::to_string(in_sample_rate) + "->" + std::to_string(out_sample_rate) +
        ", " + std::to_string(taps) + " taps) @" + std::to_string(out_index)
      )
    , m_kernel_fn(p_kernel_fn)
    , m_in_rate(in_sample_rate)
    , m_out_rate(out_sample_rate)
    , m_taps(taps)
    , m_block(block)
    , m_out_index(out_index)
{}
UnitTestResult BlockKernelTest::run(Logger& logger, CancellableScope& scope) const{
    (void)logger;
    (void)scope;

    WeightTable table(
        m_in_rate, m_out_rate, m_taps, 9.0,
        m_out_index + m_block + 64,
        m_taps
    );
    if (m_out_index >= table.period()){
        return "out_index must wrap to the period.";
    }

    //  Deterministic pseudo-random input. input[i] is absolute stream
    //  sample i, so the kernel's in_index is 0 (k = 0 periods in).
    //  The index columns keep counting past the period, so a block that
    //  wraps the period reads samples up to about period_in + block + taps.
    const size_t period_in = m_in_rate / std::gcd(m_in_rate, m_out_rate);
    AlignedVector<float> input(period_in + m_block + m_taps + 128);
    std::mt19937 generator(42);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    for (size_t i = 0; i < input.size(); i++){
        input[i] = distribution(generator);
    }

    AlignedVector<float> got(m_block);
    m_kernel_fn(table, 0, input.data(), m_out_index, got.data());

    AlignedVector<float> want = reference_lanes(table, input.data(), m_out_index, m_block);

    for (size_t c = 0; c < m_block; c++){
        int64_t ulp = 0;
        if (!within_ulp_tolerance(got[c], want[c], ulp)){
            return "Kernels disagree; max diff " + std::to_string(std::fabs(got[c] - want[c])) +
                " at output " + std::to_string(c) +
                "; got[" + std::to_string(c) + "] = " + std::to_string(got[c]) +
                ", want[" + std::to_string(c) + "] = " + std::to_string(want[c]) +
                " (" + std::to_string(ulp) + " ULP); got0..3 = [" + format_lanes(got.data(), m_block) +
                "], want0..3 = [" + format_lanes(want.data(), m_block) + "]";
        }
    }

    return true;
}



void add_tests(UnitTestDatabase& database){
    //  The Default kernel doubles as the reference for the SIMD variants.
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        48000, 44100, 16, 8, 0
    );
    //  The block straddles the table period (147) into the continuing
    //  index columns.
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        48000, 44100, 16, 8, 140
    );
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        96000, 44100, 16, 8, 0
    );

#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        add_tests_x64_SSE2(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        add_tests_x64_FMA3(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        add_tests_x64_AVX512(database);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        add_tests_ARM64_NEON(database);
    }
#endif
}



}
}
}
