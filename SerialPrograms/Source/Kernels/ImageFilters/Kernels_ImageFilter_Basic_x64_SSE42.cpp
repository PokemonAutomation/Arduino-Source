/*  Image Filters Basic (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <stdint.h>
#include <cstddef>
#include <nmmintrin.h>
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{



struct PartialWordMask{
    size_t left;
    PartialWordAccess_x64_SSE41 loader;

    PA_FORCE_INLINE PartialWordMask(size_t p_left)
        : left(p_left)
        , loader(left * sizeof(uint32_t))
    {}
};



class ImageFilter_RgbRange_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = PartialWordMask;

public:
    ImageFilter_RgbRange_x64_SSE42(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(_mm_set1_epi32(replacement))
        , m_invert(invert ? _mm_set1_epi32(-1) : _mm_setzero_si128())
        , m_mins(_mm_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm_set1_epi32(maxs ^ 0x80808080))
        , m_count(_mm_setzero_si128())
    {}

    PA_FORCE_INLINE size_t count() const{
        return reduce32_x64_SSE41(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m128i pixel = _mm_loadu_si128((const __m128i*)in);
        pixel = process_word(pixel);
        _mm_storeu_si128((__m128i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m128i pixel = mask.loader.load(in);
        pixel = process_word(pixel);
        size_t left = mask.left;
        do{
            out[0] = _mm_cvtsi128_si32(pixel);
            pixel = _mm_srli_si128(pixel, 4);
            out++;
        }while(--left);
    }

private:
    PA_FORCE_INLINE __m128i process_word(__m128i pixel){
        __m128i adj = _mm_xor_si128(pixel, _mm_set1_epi8((uint8_t)0x80));
        __m128i cmp0 = _mm_cmpgt_epi8(m_mins, adj);
        __m128i cmp1 = _mm_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm_or_si128(cmp0, cmp1);
        cmp0 = _mm_cmpeq_epi32(cmp0, _mm_setzero_si128());
        m_count = _mm_sub_epi32(m_count, cmp0);
        cmp0 = _mm_xor_si128(cmp0, m_invert);
        return _mm_blendv_epi8(m_replacement, pixel, cmp0);
    }

private:
    const __m128i m_replacement;
    const __m128i m_invert;
    const __m128i m_mins;
    const __m128i m_maxs;
    __m128i m_count;
};



size_t filter_rgb32_range_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool invert
){
    ImageFilter_RgbRange_x64_SSE42 filter(mins, maxs, replacement, invert);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilter_RgbRange_x64_SSE42>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}





class ImageFilter_RgbEuclidean_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = PartialWordMask;

public:
    ImageFilter_RgbEuclidean_x64_SSE42(uint32_t expected, double max_euclidean_distance, uint32_t replacement, bool invert)
        : m_replacement(_mm_set1_epi32(replacement))
        , m_invert(invert ? _mm_set1_epi32(-1) : _mm_setzero_si128())
        , m_expected_g(_mm_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
        , m_count(_mm_setzero_si128())
    {}

    PA_FORCE_INLINE size_t count() const{
        return reduce32_x64_SSE41(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m128i pixel = _mm_loadu_si128((const __m128i*)in);
        pixel = process_word(pixel);
        _mm_storeu_si128((__m128i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m128i pixel = mask.loader.load(in);
        pixel = process_word(pixel);
        size_t left = mask.left;
        do{
            out[0] = _mm_cvtsi128_si32(pixel);
            pixel = _mm_srli_si128(pixel, 4);
            out++;
        }while(--left);
    }

private:
    PA_FORCE_INLINE __m128i process_word(__m128i pixel){
        // _mm_srli_epi16: Shift 16-bit integers in pixels right by 8 while shifting in zeros, 
        // ng: green channels of each pixel, but shifted right by 8 bits
        __m128i ng = _mm_and_si128(_mm_srli_epi16(pixel, 8), _mm_set1_epi32(0x000000ff));
        // rb: the red and blue channels of each pixel
        __m128i rb = _mm_and_si128(pixel, _mm_set1_epi32(0x00ff00ff));
        // g: the difference between input pixel channels and the expected values
        ng = _mm_sub_epi16(ng, m_expected_g);
        rb = _mm_sub_epi16(rb, m_expected_rb);
        // compute square operation: 
        // now each 16-bit region is a squared channel difference
        // here we assume alpha channel from input image is the same as the expected,
        // so the alpha channel difference is always 0, therefore:
        // g: each 32-bit integer contains the green channel squared difference
        __m128i g = _mm_mullo_epi16(ng, ng);
        rb = _mm_mullo_epi16(rb, rb);
        // r: each 32-bit integer contains the red channel squared difference
        __m128i r = _mm_srli_epi32(rb, 16);
        // b: each 32-bit integer contains the blue channel squared difference
        __m128i b = _mm_and_si128(rb, _mm_set1_epi32(0x0000ffff));
        // compute r^2 + g^2 + b^2
        __m128i sum_sqr = _mm_add_epi32(r, g);
        sum_sqr = _mm_add_epi32(sum_sqr, b);

        __m128i cmp = _mm_cmpgt_epi32(m_distance_squared, sum_sqr);

        m_count = _mm_sub_epi32(m_count, cmp);
        cmp = _mm_xor_si128(cmp, m_invert);
        return _mm_blendv_epi8(m_replacement, pixel, cmp);
    }

private:
    const __m128i m_replacement;
    const __m128i m_invert;
    const __m128i m_expected_g;
    const __m128i m_expected_rb;
    const __m128i m_distance_squared;
    __m128i m_count;
};
size_t filter_rgb32_euclidean_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t expected, double max_euclidean_distance,
    uint32_t replacement, bool invert
){
    ImageFilter_RgbEuclidean_x64_SSE42 filter(expected, max_euclidean_distance, replacement, invert);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}








class ToBlackWhite_RgbRange_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = PartialWordMask;

public:
    ToBlackWhite_RgbRange_x64_SSE42(uint32_t mins, uint32_t maxs, bool in_range_black)
        : m_in_range_black(in_range_black ? _mm_set1_epi32(-1) : _mm_setzero_si128())
        , m_mins(_mm_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm_set1_epi32(maxs ^ 0x80808080))
        , m_count(_mm_setzero_si128())
    {}

    PA_FORCE_INLINE size_t count() const{
        return reduce32_x64_SSE41(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m128i pixel = _mm_loadu_si128((const __m128i*)in);
        pixel = process_word(pixel);
        _mm_storeu_si128((__m128i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m128i pixel = mask.loader.load(in);
        pixel = process_word(pixel);
        size_t left = mask.left;
        do{
            out[0] = _mm_cvtsi128_si32(pixel);
            pixel = _mm_srli_si128(pixel, 4);
            out++;
        }while(--left);
    }

private:
    PA_FORCE_INLINE __m128i process_word(__m128i pixel){
        __m128i adj = _mm_xor_si128(pixel, _mm_set1_epi8((uint8_t)0x80));
        __m128i cmp0 = _mm_cmpgt_epi8(m_mins, adj);
        __m128i cmp1 = _mm_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm_or_si128(cmp0, cmp1);
        cmp0 = _mm_cmpeq_epi32(cmp0, _mm_setzero_si128());
        m_count = _mm_sub_epi32(m_count, cmp0);
        cmp0 = _mm_xor_si128(cmp0, m_in_range_black);
        return _mm_or_si128(cmp0, _mm_set1_epi32(0xff000000));
    }

private:
    const __m128i m_in_range_black;
    const __m128i m_mins;
    const __m128i m_maxs;
    __m128i m_count;
};



size_t to_blackwhite_rgb32_range_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    ToBlackWhite_RgbRange_x64_SSE42 filter(mins, maxs, in_range_black);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void to_blackwhite_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    to_blackwhite_rbg32<ToBlackWhite_RgbRange_x64_SSE42>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}








}
}
#endif
