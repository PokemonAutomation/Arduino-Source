/*  YUV Conversion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_YUVConversion_H
#define PokemonAutomation_Kernels_YUVConversion_H

#include <stdint.h>
#include <stddef.h>

namespace PokemonAutomation{
namespace Kernels{


enum class YUVColorSpace{
    BT601,
    BT709,
};
enum class YUVColorRange{
    Limited,    //  Y in [16, 235], chroma in [16, 240]
    Full,       //  Y and chroma in [0, 255]
};


//  Q6 fixed point (except y_offset): the largest scale that keeps the SIMD
//  paths in 16-bit lanes. With cb, cr centered on 0:
//      luma = (Y - y_offset) * y_mult
//      R    = (luma                     + cr*cr_r + 32) >> 6
//      G    = (luma - cb*cb_g - cr*cr_g           + 32) >> 6
//      B    = (luma + cb*cb_b                     + 32) >> 6
//  The adds must saturate: B overflows int16 for BT.601 limited range.
struct YUVCoefficients{
    int16_t y_offset;
    int16_t y_mult;
    int16_t cr_r;
    int16_t cb_g;
    int16_t cr_g;
    int16_t cb_b;
};

YUVCoefficients yuv_coefficients(YUVColorSpace color_space, YUVColorRange range);


//  Packed 4:2:2. Each 4 bytes is 2 pixels.
//      YUY2/YUYV: Y0 Cb Y1 Cr
//      UYVY:      Cb Y0 Cr Y1
//  "width" is in pixels and must be even.
void convert_YUY2_to_RGB32(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
);
void convert_UYVY_to_RGB32(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
);

//  Planar 4:2:0. Chroma planes are half width and half height.
//  "width" and "height" are in pixels and must both be even.
void convert_YUV420P_to_RGB32(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_U, size_t U_bytes_per_row,
    const uint8_t* plane_V, size_t V_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
);

//  Semi-planar 4:2:0. One Y plane, one interleaved chroma plane.
//      NV12: Cb Cr
//      NV21: Cr Cb
void convert_NV12_to_RGB32(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_UV, size_t UV_bytes_per_row,
    bool swap_chroma,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
);



}
}
#endif
