/*  Pixel Sum + Sum of Squares
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_ImagePixelSumSqr.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels_ImagePixelSumSqr_Default.h"
#ifdef PA_Arch_x64_SSE41
#include "Kernels_ImagePixelSumSqr_x64_SSE41.h"
#endif
#ifdef PA_Arch_x64_AVX2
#include "Kernels_ImagePixelSumSqr_x64_AVX2.h"
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels_ImagePixelSumSqr_x64_AVX512.h"
#endif

namespace PokemonAutomation{
namespace Kernels{


void pixel_sum_sqr(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
){
#if 0
#elif defined PA_Arch_x64_AVX512
    pixel_sum_sqr_x64_AVX512(
        sums,
        width, height,
        image, image_bytes_per_row,
        alpha, alpha_bytes_per_row
    );
#elif defined PA_Arch_x64_AVX2
    pixel_sum_sqr_x64_AVX2(
        sums,
        width, height,
        image, image_bytes_per_row,
        alpha, alpha_bytes_per_row
    );
#elif defined PA_Arch_x64_SSE41
    pixel_sum_sqr_x64_SSE41(
        sums,
        width, height,
        image, image_bytes_per_row,
        alpha, alpha_bytes_per_row
    );
#else
    pixel_sum_sqr_Default(
        sums,
        width, height,
        image, image_bytes_per_row,
        alpha, alpha_bytes_per_row
    );
#endif
}



}
}
