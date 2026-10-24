/*  YUV Conversion (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#ifdef PA_AutoDispatch_x64_13_Haswell

#include <cstring>
#include <immintrin.h>
#include "Kernels_YUVConversion_Core.h"

namespace PokemonAutomation{
namespace Kernels{
namespace{


struct Coefficients256{
    __m256i y_offset;
    __m256i y_mult;
    __m256i cr_r;
    __m256i cb_g;
    __m256i cr_g;
    __m256i cb_b;

    Coefficients256(const YUVCoefficients& c)
        : y_offset(_mm256_set1_epi16(c.y_offset))
        , y_mult  (_mm256_set1_epi16(c.y_mult))
        , cr_r    (_mm256_set1_epi16(c.cr_r))
        , cb_g    (_mm256_set1_epi16(c.cb_g))
        , cr_g    (_mm256_set1_epi16(c.cr_g))
        , cb_b    (_mm256_set1_epi16(c.cb_b))
    {}
};


//  16 pixels: raw Y and raw chroma, each already broadcast to one lane per pixel.
inline void convert16(
    uint32_t* dst, __m256i Y, __m256i Cb, __m256i Cr, const Coefficients256& c
){
    const __m256i luma = _mm256_mullo_epi16(_mm256_sub_epi16(Y, c.y_offset), c.y_mult);
    const __m256i cb = _mm256_sub_epi16(Cb, _mm256_set1_epi16(128));
    const __m256i cr = _mm256_sub_epi16(Cr, _mm256_set1_epi16(128));

    __m256i r = _mm256_adds_epi16(luma, _mm256_mullo_epi16(cr, c.cr_r));
    __m256i g = _mm256_subs_epi16(
        _mm256_subs_epi16(luma, _mm256_mullo_epi16(cb, c.cb_g)),
        _mm256_mullo_epi16(cr, c.cr_g)
    );
    __m256i b = _mm256_adds_epi16(luma, _mm256_mullo_epi16(cb, c.cb_b));

    const __m256i round = _mm256_set1_epi16(32);
    r = _mm256_srai_epi16(_mm256_adds_epi16(r, round), 6);
    g = _mm256_srai_epi16(_mm256_adds_epi16(g, round), 6);
    b = _mm256_srai_epi16(_mm256_adds_epi16(b, round), 6);

    //  packus is per 128-bit lane. The permute2x128 at the end undoes this.
    const __m256i r8 = _mm256_packus_epi16(r, r);
    const __m256i g8 = _mm256_packus_epi16(g, g);
    const __m256i b8 = _mm256_packus_epi16(b, b);
    const __m256i a8 = _mm256_set1_epi8((char)0xff);

    const __m256i bg = _mm256_unpacklo_epi8(b8, g8);
    const __m256i ra = _mm256_unpacklo_epi8(r8, a8);
    const __m256i lo = _mm256_unpacklo_epi16(bg, ra);   //  px 0-3  | px 8-11
    const __m256i hi = _mm256_unpackhi_epi16(bg, ra);   //  px 4-7  | px 12-15

    _mm256_storeu_si256((__m256i*)(dst + 0), _mm256_permute2x128_si256(lo, hi, 0x20));
    _mm256_storeu_si256((__m256i*)(dst + 8), _mm256_permute2x128_si256(lo, hi, 0x31));
}


//  pshufb never crosses lanes, so the same mask works in both.
inline __m256i dual(const __m128i& mask){
    return _mm256_broadcastsi128_si256(mask);
}


template <bool uyvy>
void convert_packed422(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    const Coefficients256 c(coefficients);
    const __m256i mask_cb = dual(_mm_setr_epi8(0, -1, 0, -1, 4, -1, 4, -1, 8, -1, 8, -1, 12, -1, 12, -1));
    const __m256i mask_cr = dual(_mm_setr_epi8(2, -1, 2, -1, 6, -1, 6, -1, 10, -1, 10, -1, 14, -1, 14, -1));
    const size_t vector_width = width & ~(size_t)15;

    for (size_t r = 0; r < height; r++){
        const uint8_t* s = src + r * src_bytes_per_row;
        uint32_t* d = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);

        for (size_t x = 0; x < vector_width; x += 16){
            const __m256i v = _mm256_loadu_si256((const __m256i*)(s + x * 2));
            //  YUY2 has luma in the even bytes, UYVY in the odd ones.
            const __m256i lo = _mm256_and_si256(v, _mm256_set1_epi16(0x00ff));
            const __m256i hi = _mm256_srli_epi16(v, 8);
            const __m256i Y = uyvy ? hi : lo;
            const __m256i chroma = uyvy ? lo : hi;
            convert16(
                d + x,
                Y,
                _mm256_shuffle_epi8(chroma, mask_cb),
                _mm256_shuffle_epi8(chroma, mask_cr),
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



void convert_YUY2_to_RGB32_x64_AVX2(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    convert_packed422<false>(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
}
void convert_UYVY_to_RGB32_x64_AVX2(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    convert_packed422<true>(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
}

void convert_YUV420P_to_RGB32_x64_AVX2(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_U, size_t U_bytes_per_row,
    const uint8_t* plane_V, size_t V_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    const Coefficients256 c(coefficients);
    //  8 chroma samples -> 16 bytes, each duplicated, then widened to 16 lanes.
    const __m128i dup = _mm_setr_epi8(0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7);
    const size_t vector_width = width & ~(size_t)15;

    for (size_t r = 0; r + 1 < height; r += 2){
        const uint8_t* y0 = plane_Y + r * Y_bytes_per_row;
        const uint8_t* y1 = y0 + Y_bytes_per_row;
        const uint8_t* u = plane_U + (r >> 1) * U_bytes_per_row;
        const uint8_t* v = plane_V + (r >> 1) * V_bytes_per_row;
        uint32_t* d0 = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);
        uint32_t* d1 = (uint32_t*)((uint8_t*)d0 + dst_bytes_per_row);

        for (size_t x = 0; x < vector_width; x += 16){
            const __m128i u8 = _mm_loadl_epi64((const __m128i*)(u + (x >> 1)));
            const __m128i v8 = _mm_loadl_epi64((const __m128i*)(v + (x >> 1)));
            const __m256i Cb = _mm256_cvtepu8_epi16(_mm_shuffle_epi8(u8, dup));
            const __m256i Cr = _mm256_cvtepu8_epi16(_mm_shuffle_epi8(v8, dup));
            convert16(d0 + x, _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(y0 + x))), Cb, Cr, c);
            convert16(d1 + x, _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(y1 + x))), Cb, Cr, c);
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

void convert_NV12_to_RGB32_x64_AVX2(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_UV, size_t UV_bytes_per_row,
    bool swap_chroma,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
    const Coefficients256 c(coefficients);
    const __m128i dup_even = _mm_setr_epi8(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    const __m128i dup_odd  = _mm_setr_epi8(1, 1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11, 13, 13, 15, 15);
    const __m128i mask_cb = swap_chroma ? dup_odd : dup_even;
    const __m128i mask_cr = swap_chroma ? dup_even : dup_odd;
    const size_t cb_off = swap_chroma ? 1 : 0;
    const size_t cr_off = swap_chroma ? 0 : 1;
    const size_t vector_width = width & ~(size_t)15;

    for (size_t r = 0; r + 1 < height; r += 2){
        const uint8_t* y0 = plane_Y + r * Y_bytes_per_row;
        const uint8_t* y1 = y0 + Y_bytes_per_row;
        const uint8_t* uv = plane_UV + (r >> 1) * UV_bytes_per_row;
        uint32_t* d0 = (uint32_t*)((uint8_t*)dst + r * dst_bytes_per_row);
        uint32_t* d1 = (uint32_t*)((uint8_t*)d0 + dst_bytes_per_row);

        for (size_t x = 0; x < vector_width; x += 16){
            const __m128i pairs = _mm_loadu_si128((const __m128i*)(uv + x));
            const __m256i Cb = _mm256_cvtepu8_epi16(_mm_shuffle_epi8(pairs, mask_cb));
            const __m256i Cr = _mm256_cvtepu8_epi16(_mm_shuffle_epi8(pairs, mask_cr));
            convert16(d0 + x, _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(y0 + x))), Cb, Cr, c);
            convert16(d1 + x, _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(y1 + x))), Cb, Cr, c);
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
