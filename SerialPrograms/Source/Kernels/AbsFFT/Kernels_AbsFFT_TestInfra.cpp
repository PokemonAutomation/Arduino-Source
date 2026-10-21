/*  ABS FFT Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <bit>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include "Common/Compiler.h"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_AbsFFT_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



//
//  Tolerances for the ULP comparison.
//
//  Both implementations compute the same transform in 32-bit float, but the
//  order of the additions and multiplications differs. The accumulated
//  rounding error grows with the transform length, so for large transforms
//  the two results can drift apart by a few thousand ULPs. These bounds were
//  measured against an independent double-precision DFT at every supported
//  length (the Default path alone stays within ~350 ULPs of the reference at
//  k=16), with ample headroom for the sum of both implementations' drift.
//
//  Values that are close to zero are compared by absolute difference instead
//  of ULP distance: the ULP of a tiny value is far smaller than the
//  accumulated error of the transform, so a pure ULP bound would be
//  unreasonably strict there. The absolute bound is scaled to the magnitude
//  of the input data.
//
static constexpr uint32_t MAX_ULP_DIFF = 4096;
static constexpr double ABS_TOLERANCE_ULPS = 4096.0;



//
//  Map the float bit pattern to a monotonically increasing uint32_t, so that
//  the difference of two mapped values is the number of ULPs between the two
//  floats. +0.0 and -0.0 map to the same coordinate.
//
static uint32_t ulp_coordinate(float value){
    uint32_t bits = std::bit_cast<uint32_t>(value);
    if (bits & 0x80000000u){
        return 0u - bits;
    }
    return bits + 0x80000000u;
}
static uint32_t ulp_distance(float a, float b){
    uint32_t ia = ulp_coordinate(a);
    uint32_t ib = ulp_coordinate(b);
    return ia >= ib ? ia - ib : ib - ia;
}
static bool outputs_match(float a, float b, double abs_tolerance){
    if (!std::isfinite(a) || !std::isfinite(b)){
        return false;
    }
    if (ulp_distance(a, b) <= MAX_ULP_DIFF){
        return true;
    }
    return std::fabs((double)a - (double)b) <= abs_tolerance;
}



Test::Test(
    const std::string& arch_str,
    int k,
    bool use_weights,
    FFTAbs fft0,
    FFTAbs fft1
)
    : UnitTest(
        "Kernels::AbsFFT - " + arch_str +
        "(k=" + std::to_string(k) + (use_weights ? ", weights)" : ")")
    )
    , m_k(k)
    , m_use_weights(use_weights)
    , m_fft0(fft0)
    , m_fft1(fft1)
{}

UnitTestResult Test::run(Logger& logger, CancellableScope& scope) const{
    size_t length = (size_t)1 << m_k;
    size_t half_length = length >> 1;

    //
    //  Random input with a fixed seed. fft_abs() is destructive on "real",
    //  so each run gets its own copy of the same data.
    //
    //  Every buffer ends right before a guard page, so a read or write past
    //  the end faults on every run. "real" and "abs" must be aligned to 64
    //  bytes. Their sizes are powers of two, so for all but the smallest
    //  transforms they still end exactly at the guard page.
    //
    GuardPageBuffer<float> real0(length, PA_ALIGNMENT);
    GuardPageBuffer<float> real1(length, PA_ALIGNMENT);
    GuardPageBuffer<float> abs0(half_length, PA_ALIGNMENT);
    GuardPageBuffer<float> abs1(half_length, PA_ALIGNMENT);
    GuardPageBuffer<float> weights(length);

    std::mt19937 generator(42);
    std::uniform_int_distribution<int> sample_distribution(-10000, 10000);
    std::uniform_int_distribution<int> weight_distribution(500, 1500);
    double input_scale = 0.0;
    for (size_t c = 0; c < length; c++){
        float sample = (float)sample_distribution(generator) / 10000.0f;
        float weight = (float)weight_distribution(generator) / 1000.0f;
        real0[c] = sample;
        real1[c] = sample;
        weights[c] = weight;
        input_scale = std::max(
            input_scale,
            std::fabs((double)sample * (m_use_weights ? (double)weight : 1.0))
        );
    }

    const float* weights_ptr = m_use_weights ? weights.data() : nullptr;

    m_fft0(m_k, abs0.data(), real0.data(), weights_ptr);
    m_fft1(m_k, abs1.data(), real1.data(), weights_ptr);

    //
    //  The Default path is the source of truth; the ISA result is allowed to
    //  drift by a small ULP error.
    //
    const double abs_tolerance = ABS_TOLERANCE_ULPS * FLT_EPSILON * input_scale;
    for (size_t c = 0; c < half_length; c++){
        if (!outputs_match(abs0[c], abs1[c], abs_tolerance)){
            char value0[32];
            char value1[32];
            std::snprintf(value0, sizeof(value0), "%.9g", (double)abs0[c]);
            std::snprintf(value1, sizeof(value1), "%.9g", (double)abs1[c]);
            return "Mismatch at " + std::to_string(c) + " of " + std::to_string(half_length) +
                ": " + value0 + " != " + value1 +
                " (" + std::to_string(ulp_distance(abs0[c], abs1[c])) + " ULPs)";
        }
    }

    return true;
}



}
}
}
