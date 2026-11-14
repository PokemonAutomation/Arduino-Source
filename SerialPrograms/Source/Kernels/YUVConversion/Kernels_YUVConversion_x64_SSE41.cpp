/*  YUV Conversion (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <cstring>
#include <smmintrin.h>
#include "Kernels_YUVConversion_Core.h"

namespace PokemonAutomation{
namespace Kernels{
namespace{


struct Coefficients128{
    __m128i y_offset;
    __m128i y_mult;
    __m128i cr_r;
    __m128i cb_g;
    __m128i cr_g;
    __m128i cb_b;

    Coefficients128(const YUVCoefficients& c)
        : y_offset(_mm_set1_epi16(c.y_offset))
        , y_mult  (_mm_set1_epi16(c.y_mult))
        , cr_r    (_mm_set1_epi16(c.cr_r))
        , cb_g    (_mm_set1_epi16(c.cb_g))
        , cr_g    (_mm_set1_epi16(c.cr_g))
        , cb_b    (_mm_set1_epi16(c.cb_b))
    {}
};


//  8 pixels: raw Y and raw chroma, each already broadcast to one lane per pixel.
inline void convert8(
    uint32_t* dst, __m128i Y, __m128i Cb, __m128i Cr, const Coefficients128& c
){
    const __m128i luma = _mm_mullo_epi16(_mm_sub_epi16(Y, c.y_offset), c.y_mult);
    const __m128i cb = _mm_sub_epi16(Cb, _mm_set1_epi16(128));
    const __m128i cr = _mm_sub_epi16(Cr, _mm_set1_epi16(128));

    __m128i r = _mm_adds_epi16(luma, _mm_mullo_epi16(cr, c.cr_r));
    __m128i g = _mm_subs_epi16(
        _mm_subs_epi16(luma, _mm_mullo_epi16(cb, c.cb_g)),
        _mm_mullo_epi16(cr, c.cr_g)
    );
    __m128i b = _mm_adds_epi16(luma, _mm_mullo_epi16(cb, c.cb_b));

    const __m128i round = _mm_set1_epi16(32);
    r = _mm_srai_epi16(_mm_adds_epi16(r, round), 6);
    g = _mm_srai_epi16(_mm_adds_epi16(g, round), 6);
    b = _mm_srai_epi16(_mm_adds_epi16(b, round), 6);

    const __m128i r8 = _mm_packus_epi16(r, r);
    const __m128i g8 = _mm_packus_epi16(g, g);
    const __m128i b8 = _mm_packus_epi16(b, b);
    const __m128i a8 = _mm_set1_epi8((char)0xff);

    //  Interleave into 0xAARRGGBB (little endian: B G R A).
    const __m128i bg = _mm_unpacklo_epi8(b8, g8);
    const __m128i ra = _mm_unpacklo_epi8(r8, a8);
    _mm_storeu_si128((__m128i*)(dst + 0), _mm_unpacklo_epi16(bg, ra));
    _mm_storeu_si128((__m128i*)(dst + 4), _mm_unpackhi_epi16(bg, ra));
}


//  Duplicate the chroma byte at every other position into one u16 lane per pixel.
//  Applied to a vector of u16 lanes holding [Cb0 Cr0 Cb1 Cr1 ...].
const __m128i PACKED422_CB = _mm_setr_epi8(0, -1, 0, -1, 4, -1, 4, -1, 8, -1, 8, -1, 12, -1, 12, -1);
const __m128i PACKED422_CR = _mm_setr_epi8(2, -1, 2, -1, 6, -1, 6, -1, 10, -1, 10, -1, 14, -1, 14, -1);

//  Same, for a byte vector holding [U0 V0 U1 V1 ...] (NV12 chroma plane).
const __m128i NV12_CB = _mm_setr_epi8(0, -1, 0, -1, 2, -1, 2, -1, 4, -1, 4, -1, 6, -1, 6, -1);
const __m128i NV12_CR = _mm_setr_epi8(1, -1, 1, -1, 3, -1, 3, -1, 5, -1, 5, -1, 7, -1, 7, -1);

//  Duplicate each of 4 bytes into 2 u16 lanes (planar 4:2:0 chroma).
const __m128i PLANAR_DUP = _mm_setr_epi8(0, -1, 0, -1, 1, -1, 1, -1, 2, -1, 2, -1, 3, -1, 3, -1);


template <bool uyvy>
void convert_packed422(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    const Coefficients128 c(coefficients);
    const size_t vector_width = width & ~(size_t)7;

    for (size_t r = 0; r < height; r++){
        const uint8_t* s = src + r * src_bytes_per_row;
        uint32_t* d = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);

        for (size_t x = 0; x < vector_width; x += 8){
            const __m128i v = _mm_loadu_si128((const __m128i*)(s + x * 2));
            //  YUY2 has luma in the even bytes, UYVY in the odd ones.
            const __m128i lo = _mm_and_si128(v, _mm_set1_epi16(0x00ff));
            const __m128i hi = _mm_srli_epi16(v, 8);
            const __m128i Y = uyvy ? hi : lo;
            const __m128i chroma = uyvy ? lo : hi;
            convert8(
                d + x,
                Y,
                _mm_shuffle_epi8(chroma, PACKED422_CB),
                _mm_shuffle_epi8(chroma, PACKED422_CR),
                c
            );
        }

        const int cb_off = uyvy ? 0 : 1;
        const int cr_off = uyvy ? 2 : 3;
        const int y0_off = uyvy ? 1 : 0;
        const int y1_off = uyvy ? 3 : 2;
        for (size_t x = vector_width; x + 1 < width; x += 2){
            const uint8_t* p = s + x * 2;
            const int32_t Cb = p[cb_off];
            const int32_t Cr = p[cr_off];
            d[x + 0] = yuv_to_rgb32(p[y0_off], Cb, Cr, coefficients);
            d[x + 1] = yuv_to_rgb32(p[y1_off], Cb, Cr, coefficients);
        }
    }
}


}



void convert_YUY2_to_RGB32_x64_SSE41(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    convert_packed422<false>(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
}
void convert_UYVY_to_RGB32_x64_SSE41(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    convert_packed422<true>(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
}

void convert_YUV420P_to_RGB32_x64_SSE41(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_U, size_t U_bytes_per_row,
    const uint8_t* plane_V, size_t V_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    const Coefficients128 c(coefficients);
    const size_t vector_width = width & ~(size_t)7;

    for (size_t r = 0; r + 1 < height; r += 2){
        const uint8_t* y0 = plane_Y + r * Y_bytes_per_row;
        const uint8_t* y1 = y0 + Y_bytes_per_row;
        const uint8_t* u = plane_U + (r >> 1) * U_bytes_per_row;
        const uint8_t* v = plane_V + (r >> 1) * V_bytes_per_row;
        uint32_t* d0 = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);
        uint32_t* d1 = (uint32_t*)((uint8_t*)d0 + dst_bytes_per_row);

        for (size_t x = 0; x < vector_width; x += 8){
            uint32_t u4, v4;
            memcpy(&u4, u + (x >> 1), sizeof(u4));
            memcpy(&v4, v + (x >> 1), sizeof(v4));
            const __m128i Cb = _mm_shuffle_epi8(_mm_cvtsi32_si128((int)u4), PLANAR_DUP);
            const __m128i Cr = _mm_shuffle_epi8(_mm_cvtsi32_si128((int)v4), PLANAR_DUP);
            convert8(d0 + x, _mm_cvtepu8_epi16(_mm_loadl_epi64((const __m128i*)(y0 + x))), Cb, Cr, c);
            convert8(d1 + x, _mm_cvtepu8_epi16(_mm_loadl_epi64((const __m128i*)(y1 + x))), Cb, Cr, c);
        }
        for (size_t x = vector_width; x + 1 < width; x += 2){
            const int32_t Cb = u[x >> 1];
            const int32_t Cr = v[x >> 1];
            d0[x + 0] = yuv_to_rgb32(y0[x + 0], Cb, Cr, coefficients);
            d0[x + 1] = yuv_to_rgb32(y0[x + 1], Cb, Cr, coefficients);
            d1[x + 0] = yuv_to_rgb32(y1[x + 0], Cb, Cr, coefficients);
            d1[x + 1] = yuv_to_rgb32(y1[x + 1], Cb, Cr, coefficients);
        }
    }
}

void convert_NV12_to_RGB32_x64_SSE41(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_UV, size_t UV_bytes_per_row,
    bool swap_chroma,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    const Coefficients128 c(coefficients);
    const size_t vector_width = width & ~(size_t)7;
    const __m128i mask_cb = swap_chroma ? NV12_CR : NV12_CB;
    const __m128i mask_cr = swap_chroma ? NV12_CB : NV12_CR;
    const size_t cb_off = swap_chroma ? 1 : 0;
    const size_t cr_off = swap_chroma ? 0 : 1;

    for (size_t r = 0; r + 1 < height; r += 2){
        const uint8_t* y0 = plane_Y + r * Y_bytes_per_row;
        const uint8_t* y1 = y0 + Y_bytes_per_row;
        const uint8_t* uv = plane_UV + (r >> 1) * UV_bytes_per_row;
        uint32_t* d0 = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);
        uint32_t* d1 = (uint32_t*)((uint8_t*)d0 + dst_bytes_per_row);

        for (size_t x = 0; x < vector_width; x += 8){
            const __m128i pairs = _mm_loadl_epi64((const __m128i*)(uv + x));
            const __m128i Cb = _mm_shuffle_epi8(pairs, mask_cb);
            const __m128i Cr = _mm_shuffle_epi8(pairs, mask_cr);
            convert8(d0 + x, _mm_cvtepu8_epi16(_mm_loadl_epi64((const __m128i*)(y0 + x))), Cb, Cr, c);
            convert8(d1 + x, _mm_cvtepu8_epi16(_mm_loadl_epi64((const __m128i*)(y1 + x))), Cb, Cr, c);
        }
        for (size_t x = vector_width; x + 1 < width; x += 2){
            const int32_t Cb = uv[x + cb_off];
            const int32_t Cr = uv[x + cr_off];
            d0[x + 0] = yuv_to_rgb32(y0[x + 0], Cb, Cr, coefficients);
            d0[x + 1] = yuv_to_rgb32(y0[x + 1], Cb, Cr, coefficients);
            d1[x + 0] = yuv_to_rgb32(y1[x + 0], Cb, Cr, coefficients);
            d1[x + 1] = yuv_to_rgb32(y1[x + 1], Cb, Cr, coefficients);
        }
    }
}



}
}
#endif
