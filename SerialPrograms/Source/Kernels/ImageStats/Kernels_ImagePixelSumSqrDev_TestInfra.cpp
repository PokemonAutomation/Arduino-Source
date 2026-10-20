/*  Sum of Squares of Deviation Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_ImagePixelSumSqrDev_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{



static const char* mode_str(SumSquareMode mode){
    switch (mode){
    case SumSquareMode::REFERENCE_ALPHA:
        return "REFERENCE_ALPHA";
    case SumSquareMode::USE_BACKGROUND:
        return "USE_BACKGROUND";
    case SumSquareMode::ARBITRATE_ALPHAS:
        return "ARBITRATE_ALPHAS";
    }
    return "UNKNOWN";
}



Test_SumSqrDeviation::Test_SumSqrDeviation(
    const std::string& arch_str,
    SumSquareMode mode,
    size_t width, size_t height,
    SumSqrDeviationConvert convert
)
    : UnitTest(
        "Kernels::ImageStats::sum_sqr_deviation - " + arch_str + "/" + mode_str(mode) +
        "(" + std::to_string(width) + "x" + std::to_string(height) + ")"
    )
    , m_mode(mode)
    , m_width(width)
    , m_height(height)
    , m_convert(convert)
{}

UnitTestResult Test_SumSqrDeviation::run(Logger& logger, CancellableScope& scope) const{
    //  Both images end right before a guard page, so a kernel that reads past
    //  the last row faults on every run.
    GuardPageBuffer<uint32_t> ref(m_width * m_height);
    GuardPageBuffer<uint32_t> img(m_width * m_height);

    std::mt19937 generator(42);
    std::uniform_int_distribution<uint32_t> distribution;
    for (size_t c = 0; c < m_width * m_height; c++){
        ref[c] = distribution(generator);
        img[c] = distribution(generator);
    }
    uint32_t background = distribution(generator);

    //
    //  Known output, computed with a plain scalar loop.
    //  A pixel is active if the MSB of its value is set.
    //  Everything is integer math, so this is exact.
    //
    uint64_t expected_count = 0;
    uint64_t expected_sumsqrs = 0;
    for (size_t c = 0; c < m_width * m_height; c++){
        uint32_t r = ref[c];
        uint32_t i = img[c];

        int32_t alphaR = (int32_t)r >> 31;

        if (m_mode == SumSquareMode::REFERENCE_ALPHA){
            r &= (uint32_t)alphaR;
            i &= (uint32_t)alphaR;
        }
        else if (m_mode == SumSquareMode::USE_BACKGROUND){
            r = (alphaR != 0) ? r : background;
        }
        else{
            uint32_t alphaI = (uint32_t)((int32_t)i >> 31);
            r &= (uint32_t)alphaR;
            i &= alphaI;
            uint32_t disagree = alphaI ^ (uint32_t)alphaR;
            r |= disagree;
            i &= ~disagree;
        }

        expected_count += (alphaR != 0) ? 1 : 0;

        int32_t d0 = (int32_t)(r & 0x000000ff) - (int32_t)(i & 0x000000ff);
        int32_t d1 = (int32_t)((r >>  8) & 0x000000ff) - (int32_t)((i >>  8) & 0x000000ff);
        int32_t d2 = (int32_t)((r >> 16) & 0x000000ff) - (int32_t)((i >> 16) & 0x000000ff);
        expected_sumsqrs += (uint64_t)(d0 * d0) + (uint64_t)(d1 * d1) + (uint64_t)(d2 * d2);
    }

    uint64_t count = 0;
    uint64_t sumsqrs = 0;
    m_convert(
        count, sumsqrs,
        m_width, m_height,
        ref.data(), m_width * sizeof(uint32_t),
        img.data(), m_width * sizeof(uint32_t),
        background
    );

    if (expected_count != count){
        return "Mismatch on count: " +
            std::to_string(expected_count) + " != " + std::to_string(count);
    }
    if (expected_sumsqrs != sumsqrs){
        return "Mismatch on sumsqrs: " +
            std::to_string(expected_sumsqrs) + " != " + std::to_string(sumsqrs);
    }

    return true;
}



}
}
