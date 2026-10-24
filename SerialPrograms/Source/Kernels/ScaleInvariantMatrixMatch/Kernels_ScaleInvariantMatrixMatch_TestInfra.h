/*  Scale Invariant Matrix Match Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_TestInfra_H
#define PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



//
//  Signatures of compute_scale_Default(), compute_scale_min4_x86_SSE(), etc.
//
using SimmFnScale     = float (*)(size_t width, size_t height, float const* const* A, float const* const* T);
using SimmFnScaleW    = float (*)(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
using SimmFnError     = float (*)(size_t width, size_t height, float scale, float const* const* A, float const* const* T);
using SimmFnErrorW    = float (*)(size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);



//
//  The width/height combinations the tests cover: 1x1 up to 1000x1000,
//  divisible and undivisible by the vector lengths (4, 8, 16), including
//  the short-row cases that exercise the unaligned prefix and the partial
//  tail of the vector loops.
//
static constexpr size_t kTestSizes[][2] = {
    {1, 1},   {2, 1},   {3, 1},   {4, 1},   {5, 1},   {7, 1},
    {8, 1},   {15, 1},  {16, 1},  {17, 1},
    {1, 2},   {2, 3},   {3, 5},   {5, 7},
    {7, 16},  {8, 33},
    {31, 32}, {32, 33},
    {63, 64}, {64, 65},
    {127, 128},{128, 129},
    {255, 256},{256, 257},
    {511, 512},{512, 513},
    {999, 1000},{1000, 1000},
    {1000, 1},{999, 1},
    {1, 1000},{2, 1000},{3, 1000},
    {16, 1000},{17, 1000},
    {256, 1000},{257, 1000},
    {512, 1000},{513, 1000},
    {7, 1000},{8, 1000},
};



//
//  Runs all four functions (compute_scale / compute_error, plain and
//  weighted) twice on identical random data, once with the Default
//  implementation and once with the ISA-specific implementation, and
//  requires the two results to agree within rounding. The Default path
//  is the source of truth.
//
//  The results are float sums over up to a million terms accumulated in
//  different orders, so an exact comparison is not possible; a result
//  passes if it is within 4096 ULPs of the Default result, or within
//  1e-4 of the natural magnitude of the accumulated sum (the latter
//  covers near-zero results, where ULPs are far smaller than the
//  accumulated rounding).
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
        size_t width, size_t height,
        bool use_weights,
        SimmFnScale scale0, SimmFnScale scale1,
        SimmFnScaleW scaleW0, SimmFnScaleW scaleW1,
        SimmFnError error0, SimmFnError error1,
        SimmFnErrorW errorW0, SimmFnErrorW errorW1
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

private:
    size_t m_width;
    size_t m_height;
    bool m_use_weights;
    SimmFnScale m_scale0;
    SimmFnScale m_scale1;
    SimmFnScaleW m_scaleW0;
    SimmFnScaleW m_scaleW1;
    SimmFnError m_error0;
    SimmFnError m_error1;
    SimmFnErrorW m_errorW0;
    SimmFnErrorW m_errorW1;
};



}
}
}
#endif
