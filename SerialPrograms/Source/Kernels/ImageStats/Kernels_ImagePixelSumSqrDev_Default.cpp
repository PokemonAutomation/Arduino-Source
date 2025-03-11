/*  Sum of Squares of Deviation (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "Kernels_ImagePixelSumSqrDev.h"

namespace PokemonAutomation{
namespace Kernels{


template <SumSquareMode mode>
PA_FORCE_INLINE void sum_sqr_deviation_Default(
    uint64_t& count, uint64_t& sumsqrs,
    uint16_t width,
    const uint32_t* ref, const uint32_t* img,
    uint32_t background
){
    uint32_t total = 0;
    for (size_t c = 0; c < width; c++){
        uint32_t r = ref[c];
        uint32_t i = img[c];

        uint32_t alphaR = (int32_t)r >> 31;

        if (mode == SumSquareMode::REFERENCE_ALPHA){
            r &= alphaR;
            i &= alphaR;
        }
        if (mode == SumSquareMode::USE_BACKGROUND){
            r = alphaR ? r : background;
        }
        if (mode == SumSquareMode::ARBITRATE_ALPHAS){
            uint32_t alphaI = (int32_t)i >> 31;
            r &= alphaR;
            i &= alphaI;
            alphaI ^= alphaR;
            r |= alphaI;
            i &= ~alphaI;
        }

        uint32_t r0 = r & 0x000000ff;
        uint32_t i0 = i & 0x000000ff;
        uint32_t r1 = (r >> 8) & 0x000000ff;
        uint32_t i1 = (i >> 8) & 0x000000ff;
        uint32_t r2 = (r >> 16) & 0x000000ff;
        uint32_t i2 = (i >> 16) & 0x000000ff;

        r0 -= i0;
        r1 -= i1;
        r2 -= i2;

        r0 *= r0;
        r1 *= r1;
        r2 *= r2;

        total -= alphaR;
        r0 += r1;
        r0 += r2;
        sumsqrs += r0;
    }
    count += total;
}

template <SumSquareMode mode>
void sum_sqr_deviation_Default(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
){
    if (width > 22017){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Width limit exceeded: " + std::to_string(width));
    }
    for (size_t r = 0; r < height; r++){
        sum_sqr_deviation_Default<mode>(
            count, sumsqrs,
            (uint16_t)width, ref, img, background
        );
        ref = (const uint32_t*)((const char*)ref + ref_bytes_per_line);
        img = (const uint32_t*)((const char*)img + img_bytes_per_line);
    }
}


template
void sum_sqr_deviation_Default<SumSquareMode::REFERENCE_ALPHA>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_Default<SumSquareMode::USE_BACKGROUND>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_Default<SumSquareMode::ARBITRATE_ALPHAS>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);




}
}
