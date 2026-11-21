/*  YUV Conversion (scalar core)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The SIMD paths use this for row tails and must match it exactly.
 *
 */

#ifndef PokemonAutomation_Kernels_YUVConversion_Core_H
#define PokemonAutomation_Kernels_YUVConversion_Core_H

#include <algorithm>
#include "Kernels_YUVConversion.h"

namespace PokemonAutomation{
namespace Kernels{


//  Saturating, to match the SIMD paths.
inline int32_t yuv_adds16(int32_t a, int32_t b){
    return std::min(std::max(a + b, -32768), 32767);
}
inline uint8_t yuv_clamp_shift(int32_t value){
    return (uint8_t)std::min(std::max(yuv_adds16(value, 32) >> 6, 0), 255);
}

inline uint32_t yuv_to_rgb32(
    int32_t Y, int32_t Cb, int32_t Cr, const YUVCoefficients& c
){
    const int32_t luma = (Y - c.y_offset) * c.y_mult;
    const int32_t cb = Cb - 128;
    const int32_t cr = Cr - 128;

    const int32_t r = yuv_adds16(luma, cr * c.cr_r);
    const int32_t g = yuv_adds16(yuv_adds16(luma, -(cb * c.cb_g)), -(cr * c.cr_g));
    const int32_t b = yuv_adds16(luma, cb * c.cb_b);

    return 0xff000000
         | ((uint32_t)yuv_clamp_shift(r) << 16)
         | ((uint32_t)yuv_clamp_shift(g) << 8)
         |  (uint32_t)yuv_clamp_shift(b);
}



}
}
#endif
