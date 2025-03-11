/*  Sum of Squares of Deviation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_ImagePixelSumSqrDeviation_H
#define PokemonAutomation_Kernels_ImagePixelSumSqrDeviation_H

#include <stdint.h>
#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{



enum class SumSquareMode{
    REFERENCE_ALPHA,
    USE_BACKGROUND,
    ARBITRATE_ALPHAS,
};


//
//  count   = # of non-zero alpha pixels in "ref".
//  sumsqrs = Sum of squares of differences between "ref" and "img".
//
void sum_sqr_deviation(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line
);


//
//  Replace all zero-alpha pixels in "ref" with "background".
//
//  count   = # of non-zero alpha pixels in "ref" (before background replacement.)
//  sumsqrs = Sum of squares of differences between "ref" and "img".
//
void sum_sqr_deviation(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);


//
//  count   = # of non-zero alpha pixels in "ref".
//  sumsqrs = Sum of squares of differences between "ref" and "img".
//
//  Pixels where the two images disagree on alpha status are treated as maximum
//  possible difference.
//
void sum_sqr_deviation_masked(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line
);


}
}
#endif
