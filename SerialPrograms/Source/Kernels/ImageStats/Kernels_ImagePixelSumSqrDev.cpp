/*  Sum of Squares of Deviation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_ImagePixelSumSqrDev.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels_ImagePixelSumSqrDev_Default.h"
#ifdef PA_Arch_x64_SSE41
#include "Kernels_ImagePixelSumSqrDev_x64_SSE41.h"
#endif
#ifdef PA_Arch_x64_AVX2
#include "Kernels_ImagePixelSumSqrDev_x64_AVX2.h"
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels_ImagePixelSumSqrDev_x64_AVX512.h"
#endif

namespace PokemonAutomation{
namespace Kernels{


template <SumSquareMode mode>
void sum_sqr_deviation(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background = 0
){
#if 0
#elif defined PA_Arch_x64_AVX512
    sum_sqr_deviation_x64_AVX512<mode>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line,
        background
    );
#elif defined PA_Arch_x64_AVX2
    sum_sqr_deviation_x64_AVX2<mode>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line,
        background
    );
#elif defined PA_Arch_x64_SSE41
    sum_sqr_deviation_x64_SSE41<mode>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line,
        background
    );
#else
    sum_sqr_deviation_Default<mode>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line,
        background
    );
#endif
}


void sum_sqr_deviation(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line
){
    sum_sqr_deviation<SumSquareMode::REFERENCE_ALPHA>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line
    );
}
void sum_sqr_deviation(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
){
    sum_sqr_deviation<SumSquareMode::USE_BACKGROUND>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line,
        background
    );
}
void sum_sqr_deviation_masked(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line
){
    sum_sqr_deviation<SumSquareMode::ARBITRATE_ALPHAS>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line
    );
}



}
}
