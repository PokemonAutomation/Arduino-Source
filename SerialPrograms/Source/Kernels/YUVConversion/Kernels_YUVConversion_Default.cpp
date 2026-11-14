/*  YUV Conversion (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_YUVConversion_Core.h"

namespace PokemonAutomation{
namespace Kernels{
namespace{


//  Packed 4:2:2 with configurable byte layout.
//  y0/y1 and cb/cr are byte offsets within each 4-byte pair.
template <int y0_off, int cb_off, int y1_off, int cr_off>
void convert_packed422(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& c
){
    for (size_t r = 0; r < height; r++){
        const uint8_t* s = src + r * src_bytes_per_row;
        uint32_t* d = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);
        for (size_t x = 0; x + 1 < width; x += 2){
            const int32_t Cb = s[cb_off];
            const int32_t Cr = s[cr_off];
            d[0] = yuv_to_rgb32(s[y0_off], Cb, Cr, c);
            d[1] = yuv_to_rgb32(s[y1_off], Cb, Cr, c);
            s += 4;
            d += 2;
        }
    }
}


}



void convert_YUY2_to_RGB32_Default(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    convert_packed422<0, 1, 2, 3>(
        width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients
    );
}
void convert_UYVY_to_RGB32_Default(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    convert_packed422<1, 0, 3, 2>(
        width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients
    );
}

void convert_YUV420P_to_RGB32_Default(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_U, size_t U_bytes_per_row,
    const uint8_t* plane_V, size_t V_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& c
){
    for (size_t r = 0; r + 1 < height; r += 2){
        const uint8_t* y0 = plane_Y + r * Y_bytes_per_row;
        const uint8_t* y1 = y0 + Y_bytes_per_row;
        const uint8_t* u = plane_U + (r >> 1) * U_bytes_per_row;
        const uint8_t* v = plane_V + (r >> 1) * V_bytes_per_row;
        uint32_t* d0 = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);
        uint32_t* d1 = (uint32_t*)((uint8_t*)d0 + dst_bytes_per_row);
        for (size_t x = 0; x + 1 < width; x += 2){
            const int32_t Cb = u[x >> 1];
            const int32_t Cr = v[x >> 1];
            d0[x + 0] = yuv_to_rgb32(y0[x + 0], Cb, Cr, c);
            d0[x + 1] = yuv_to_rgb32(y0[x + 1], Cb, Cr, c);
            d1[x + 0] = yuv_to_rgb32(y1[x + 0], Cb, Cr, c);
            d1[x + 1] = yuv_to_rgb32(y1[x + 1], Cb, Cr, c);
        }
    }
}

void convert_NV12_to_RGB32_Default(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_UV, size_t UV_bytes_per_row,
    bool swap_chroma,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& c
){
    const size_t cb_off = swap_chroma ? 1 : 0;
    const size_t cr_off = swap_chroma ? 0 : 1;
    for (size_t r = 0; r + 1 < height; r += 2){
        const uint8_t* y0 = plane_Y + r * Y_bytes_per_row;
        const uint8_t* y1 = y0 + Y_bytes_per_row;
        const uint8_t* uv = plane_UV + (r >> 1) * UV_bytes_per_row;
        uint32_t* d0 = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);
        uint32_t* d1 = (uint32_t*)((uint8_t*)d0 + dst_bytes_per_row);
        for (size_t x = 0; x + 1 < width; x += 2){
            const int32_t Cb = uv[x + cb_off];
            const int32_t Cr = uv[x + cr_off];
            d0[x + 0] = yuv_to_rgb32(y0[x + 0], Cb, Cr, c);
            d0[x + 1] = yuv_to_rgb32(y0[x + 1], Cb, Cr, c);
            d1[x + 0] = yuv_to_rgb32(y1[x + 0], Cb, Cr, c);
            d1[x + 1] = yuv_to_rgb32(y1[x + 1], Cb, Cr, c);
        }
    }
}



}
}
