/*  Sum of Squares of Deviation (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImagePixelSumSqrDeviation_x64_AVX2_H
#define PokemonAutomation_Kernels_ImagePixelSumSqrDeviation_x64_AVX2_H

#include "Kernels_ImagePixelSumSqrDev.h"

namespace PokemonAutomation{
namespace Kernels{


template <SumSquareMode mode>
void sum_sqr_deviation_x64_AVX2(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background = 0
);



}
}
#endif
