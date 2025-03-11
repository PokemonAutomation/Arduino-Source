/*  Sum of Squares of Deviation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_ImagePixelSumSqrDev.h"

namespace PokemonAutomation{
namespace Kernels{


template <SumSquareMode mode>
void sum_sqr_deviation_Default(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template <SumSquareMode mode>
void sum_sqr_deviation_x64_SSE41(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template <SumSquareMode mode>
void sum_sqr_deviation_x64_AVX2(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template <SumSquareMode mode>
void sum_sqr_deviation_x64_AVX512(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);



template <SumSquareMode mode>
void sum_sqr_deviation(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background = 0
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        sum_sqr_deviation_x64_AVX512<mode>(
            count, sumsqrs,
            width, height,
            ref, ref_bytes_per_line,
            img, img_bytes_per_line,
            background
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        sum_sqr_deviation_x64_AVX2<mode>(
            count, sumsqrs,
            width, height,
            ref, ref_bytes_per_line,
            img, img_bytes_per_line,
            background
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        sum_sqr_deviation_x64_SSE41<mode>(
            count, sumsqrs,
            width, height,
            ref, ref_bytes_per_line,
            img, img_bytes_per_line,
            background
        );
        return;
    }
#endif
    sum_sqr_deviation_Default<mode>(
        count, sumsqrs,
        width, height,
        ref, ref_bytes_per_line,
        img, img_bytes_per_line,
        background
    );
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
