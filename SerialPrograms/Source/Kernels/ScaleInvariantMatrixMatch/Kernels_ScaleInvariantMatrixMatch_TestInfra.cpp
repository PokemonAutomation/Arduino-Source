/*  Scale Invariant Matrix Match Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <bit>
#include <cmath>
#include <cstdio>
#include <random>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_ScaleInvariantMatrixMatch_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



//
//  Head padding (in floats) before the first row of the matrices. The
//  current ISAs do not read before a row start, but the padding keeps the
//  first row away from the very start of the allocation in case a future
//  ISA does (e.g. an SSE4.1-style back-read of up to VL - 1 floats).
//
static constexpr size_t HEAD_PAD = 16;
//
//  There is no tail padding: the last row ends right before a guard page,
//  so a read past the end of the matrix faults on every run.
//

//
//  A result passes if it is within 4096 ULPs of the Default result, or
//  within 1e-4 of the natural magnitude of the accumulated sum. The
//  absolute part covers near-zero results, where ULPs are far smaller
//  than the accumulated rounding of a multi-million-term sum.
//
static constexpr uint64_t MAX_ULP_DIFF = 4096;
static constexpr double ABS_TOLERANCE = 1e-4;

static uint64_t ulp_distance(float a, float b){
    //  Monotonic mapping of the float bits to a signed order.
    auto U = [](float f) -> uint64_t{
        int32_t i = std::bit_cast<int32_t>(f);
        if (i >= 0)
            return (uint64_t)(uint32_t)i + 0x80000000ull;
        return (uint64_t)(uint32_t)(-i);
    };
    uint64_t x = U(a);
    uint64_t y = U(b);
    return x > y ? x - y : y - x;
}
static bool within_tolerance(float x, float y, double magnitude){
    if (x == y)
        return true;
    if (ulp_distance(x, y) <= MAX_ULP_DIFF)
        return true;
    return std::fabs((double)x - (double)y) <= ABS_TOLERANCE * std::fmax(1.0, magnitude);
}



Test::Test(
    const std::string& arch_str,
    size_t width, size_t height,
    bool use_weights,
    SimmFnScale scale0, SimmFnScale scale1,
    SimmFnScaleW scaleW0, SimmFnScaleW scaleW1,
    SimmFnError error0, SimmFnError error1,
    SimmFnErrorW errorW0, SimmFnErrorW errorW1
)
    : UnitTest(
        "Kernels::ScaleInvariantMatrixMatch - " + arch_str +
        "(w=" + std::to_string(width) + ", h=" + std::to_string(height) +
        (use_weights ? ", weights)" : ")")
    )
    , m_width(width)
    , m_height(height)
    , m_use_weights(use_weights)
    , m_scale0(scale0)
    , m_scale1(scale1)
    , m_scaleW0(scaleW0)
    , m_scaleW1(scaleW1)
    , m_error0(error0)
    , m_error1(error1)
    , m_errorW0(errorW0)
    , m_errorW1(errorW1)
{}

UnitTestResult Test::run(Logger& logger, CancellableScope& scope) const{
    scope.throw_if_cancelled();
    const size_t n = m_width * m_height;

    //
    //  Random data with a deterministic seed: A and T in [-1, 1], W in
    //  [0, 1]. TW = T * W is precomputed with the same float rounding
    //  that production uses.
    //
    //  Each matrix ends right before a guard page. The kernels align their
    //  loads to T and step A and W by the same offset, so every matrix must
    //  have the same alignment. Equal sizes give them that.
    //
    const size_t weighted_size = m_use_weights ? n + HEAD_PAD : 0;
    GuardPageBuffer<float> A(n + HEAD_PAD);
    GuardPageBuffer<float> T(n + HEAD_PAD);
    AlignedVector<const float*> rowsA(m_height);
    AlignedVector<const float*> rowsT(m_height);
    GuardPageBuffer<float> W(weighted_size);
    GuardPageBuffer<float> TW(weighted_size);
    AlignedVector<const float*> rowsW;
    AlignedVector<const float*> rowsTW;
    if (m_use_weights){
        rowsW = AlignedVector<const float*>(m_height);
        rowsTW = AlignedVector<const float*>(m_height);
    }

    std::mt19937 generator((unsigned int)(m_width * 1000003u + m_height * 7919u + (m_use_weights ? 2654435761u : 0u)));
    std::uniform_real_distribution<float> data_distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> weight_distribution(0.0f, 1.0f);
    for (size_t c = HEAD_PAD; c < HEAD_PAD + n; c++){
        A[c] = data_distribution(generator);
        T[c] = data_distribution(generator);
        if (m_use_weights){
            W[c] = weight_distribution(generator);
            TW[c] = T[c] * W[c];
        }
    }
    for (size_t r = 0; r < m_height; r++){
        rowsA[r] = A.data() + HEAD_PAD + r * m_width;
        rowsT[r] = T.data() + HEAD_PAD + r * m_width;
        if (m_use_weights){
            rowsW[r] = W.data() + HEAD_PAD + r * m_width;
            rowsTW[r] = TW.data() + HEAD_PAD + r * m_width;
        }
    }

    char value0[32];
    char value1[32];
    auto mismatch = [&](const char* fn, float r0, float r1, double magnitude) -> UnitTestResult{
        std::snprintf(value0, sizeof(value0), "%.9g", (double)r0);
        std::snprintf(value1, sizeof(value1), "%.9g", (double)r1);
        return std::string(fn) + ": default=" + value0 + " isa=" + value1 +
            " (" + std::to_string(ulp_distance(r0, r1)) + " ulps, abs " +
            std::to_string(std::fabs((double)r0 - (double)r1)) + " vs tolerance " +
            std::to_string(ABS_TOLERANCE * std::fmax(1.0, magnitude)) + ")";
    };

    //
    //  compute_scale: s = sum(A * T) / sum(A * A).
    //
    float s0 = m_scale0(m_width, m_height, rowsA.data(), rowsT.data());
    float s1 = m_scale1(m_width, m_height, rowsA.data(), rowsT.data());
    double mag = std::fmax(std::fabs((double)s0), std::fabs((double)s1));
    if (!within_tolerance(s0, s1, mag))
        return mismatch("compute_scale", s0, s1, mag);

    //
    //  compute_error at the Default scale: sum((s * A - T)^2).
    //
    double magE = 0.0;
    for (size_t c = HEAD_PAD; c < HEAD_PAD + n; c++){
        double m = std::fabs((double)s0 * (double)A[c]) + std::fabs((double)T[c]);
        magE += m * m;
    }
    float e0 = m_error0(m_width, m_height, s0, rowsA.data(), rowsT.data());
    float e1 = m_error1(m_width, m_height, s0, rowsA.data(), rowsT.data());
    if (!within_tolerance(e0, e1, magE))
        return mismatch("compute_error", e0, e1, magE);

    if (m_use_weights){
        //
        //  Weighted compute_scale: s = sum((A * W) * (T * W)) / sum((A * W)^2).
        //
        float ws0 = m_scaleW0(m_width, m_height, rowsA.data(), rowsTW.data(), rowsW.data());
        float ws1 = m_scaleW1(m_width, m_height, rowsA.data(), rowsTW.data(), rowsW.data());
        double magW = std::fmax(std::fabs((double)ws0), std::fabs((double)ws1));
        if (!within_tolerance(ws0, ws1, magW))
            return mismatch("compute_scale (weighted)", ws0, ws1, magW);

        //
        //  Weighted compute_error at the Default weighted scale:
        //  sum(((s * A) * W - T * W)^2).
        //
        double magWE = 0.0;
        for (size_t c = HEAD_PAD; c < HEAD_PAD + n; c++){
            double m = std::fabs((double)ws0 * (double)A[c]) + std::fabs((double)T[c]);
            magWE += (double)W[c] * (double)W[c] * m * m;
        }
        float we0 = m_errorW0(m_width, m_height, ws0, rowsA.data(), rowsTW.data(), rowsW.data());
        float we1 = m_errorW1(m_width, m_height, ws0, rowsA.data(), rowsTW.data(), rowsW.data());
        if (!within_tolerance(we0, we1, magWE))
            return mismatch("compute_error (weighted)", we0, we1, magWE);
    }

    return true;
}



}
}
}
