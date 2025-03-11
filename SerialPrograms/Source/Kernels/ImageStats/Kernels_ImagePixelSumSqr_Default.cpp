/*  Pixel Sum + Sum of Squares (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels_ImagePixelSumSqr.h"

namespace PokemonAutomation{
namespace Kernels{


PA_FORCE_INLINE void pixel_sum_sqr_Default(
    PixelSums& sums,
    uint16_t width,
    const uint32_t* image,
    const uint32_t* alpha
){
    uint32_t sumB = 0;
    uint32_t sumG = 0;
    uint32_t sumR = 0;
    uint32_t sumA = 0;
    uint32_t sqrB = 0;
    uint32_t sqrG = 0;
    uint32_t sqrR = 0;

    for (size_t c = 0; c < width; c++){
        uint32_t p = image[c];
        int32_t m = alpha[c];

        m = m >> 31;
        p &= (uint32_t)m;

        uint32_t r0 = p & 0x000000ff;
        uint32_t r1 = (p >>  8) & 0x000000ff;
        uint32_t r2 = (p >> 16) & 0x000000ff;

        sumB += r0;
        sumG += r1;
        sumR += r2;
        sumA -= m;

        r0 *= r0;
        r1 *= r1;
        r2 *= r2;

        sqrB += r0;
        sqrG += r1;
        sqrR += r2;
    }

    sums.count += sumA;
    sums.sumR += sumR;
    sums.sumG += sumG;
    sums.sumB += sumB;
    sums.sqrR += sqrR;
    sums.sqrG += sqrG;
    sums.sqrB += sqrB;
}
void pixel_sum_sqr_Default(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
){
    if (width == 0 || height == 0){
        return;
    }
    if (width > 65535){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Width limit exceeded: " + std::to_string(width));
    }
    for (size_t r = 0; r < height; r++){
        pixel_sum_sqr_Default(sums, (uint16_t)width, image, alpha);
        image = (const uint32_t*)((const char*)image + image_bytes_per_row);
        alpha = (const uint32_t*)((const char*)alpha + alpha_bytes_per_row);
    }
}


}
}
