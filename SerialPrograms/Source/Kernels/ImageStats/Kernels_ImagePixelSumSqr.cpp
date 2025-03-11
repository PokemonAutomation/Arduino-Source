/*  Pixel Sum + Sum of Squares
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_ImagePixelSumSqr.h"

namespace PokemonAutomation{
namespace Kernels{


void pixel_sum_sqr_Default(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);
void pixel_sum_sqr_x64_SSE41(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);
void pixel_sum_sqr_x64_AVX2(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);
void pixel_sum_sqr_x64_AVX512(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);



void pixel_sum_sqr(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        pixel_sum_sqr_x64_AVX512(
            sums,
            width, height,
            image, image_bytes_per_row,
            alpha, alpha_bytes_per_row
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        pixel_sum_sqr_x64_AVX2(
            sums,
            width, height,
            image, image_bytes_per_row,
            alpha, alpha_bytes_per_row
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        pixel_sum_sqr_x64_SSE41(
            sums,
            width, height,
            image, image_bytes_per_row,
            alpha, alpha_bytes_per_row
        );
        return;
    }
#endif
    pixel_sum_sqr_Default(
        sums,
        width, height,
        image, image_bytes_per_row,
        alpha, alpha_bytes_per_row
    );
}



}
}
