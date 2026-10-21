/*  Pixel Sum + Sum of Squares Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_ImagePixelSumSqr_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{



Test_PixelSumSqr::Test_PixelSumSqr(
    const std::string& arch_str,
    size_t width, size_t height,
    PixelSumSqrConvert convert
)
    : UnitTest(
        "Kernels::ImageStats::pixel_sum_sqr - " + arch_str +
        "(" + std::to_string(width) + "x" + std::to_string(height) + ")"
    )
    , m_width(width)
    , m_height(height)
    , m_convert(convert)
{}

UnitTestResult Test_PixelSumSqr::run(Logger& logger, CancellableScope& scope) const{
    //  Both images end right before a guard page, so a kernel that reads past
    //  the last row faults on every run.
    GuardPageBuffer<uint32_t> image(m_width * m_height);
    GuardPageBuffer<uint32_t> alpha(m_width * m_height);

    std::mt19937 generator(42);
    std::uniform_int_distribution<uint32_t> distribution;
    for (size_t c = 0; c < m_width * m_height; c++){
        image[c] = distribution(generator);
        alpha[c] = distribution(generator);
    }

    //
    //  Known output, computed with a plain scalar loop.
    //  A pixel is active if the MSB of its alpha value is set.
    //  Everything is integer math, so this is exact.
    //
    PixelSums expected;
    for (size_t c = 0; c < m_width * m_height; c++){
        uint32_t p = image[c];
        int32_t m = (int32_t)alpha[c] >> 31;
        p &= (uint32_t)m;

        uint32_t b = p & 0x000000ff;
        uint32_t g = (p >>  8) & 0x000000ff;
        uint32_t r = (p >> 16) & 0x000000ff;

        expected.count += (m != 0) ? 1 : 0;
        expected.sumR += r;
        expected.sumG += g;
        expected.sumB += b;
        expected.sqrR += r * r;
        expected.sqrG += g * g;
        expected.sqrB += b * b;
    }

    PixelSums actual;
    m_convert(
        actual,
        m_width, m_height,
        image.data(), m_width * sizeof(uint32_t),
        alpha.data(), m_width * sizeof(uint32_t)
    );

    struct{
        const char* name;
        uint64_t expected;
        uint64_t actual;
    } fields[] = {
        { "count", expected.count, actual.count },
        { "sumR",  expected.sumR,  actual.sumR  },
        { "sumG",  expected.sumG,  actual.sumG  },
        { "sumB",  expected.sumB,  actual.sumB  },
        { "sqrR",  expected.sqrR,  actual.sqrR  },
        { "sqrG",  expected.sqrG,  actual.sqrG  },
        { "sqrB",  expected.sqrB,  actual.sqrB  },
    };
    for (auto& f : fields){
        if (f.expected != f.actual){
            return "Mismatch on " + std::string(f.name) + ": " +
                std::to_string(f.expected) + " != " + std::to_string(f.actual);
        }
    }

    return true;
}



}
}
