/*  Image Filters Basic (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


struct PartialWordMask{
    __mmask16 m;

    PA_FORCE_INLINE PartialWordMask(size_t left)
        : m((__mmask16)1 << left)
    {}
};



class ImageFilterByMask_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    ImageFilterByMask_x64_AVX512(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(_mm512_set1_epi32(replacement))
        , m_invert(invert ? 0xffff : 0)
        , m_mins(_mm512_set1_epi32(mins))
        , m_maxs(_mm512_set1_epi32(maxs))
        , m_count(_mm512_setzero_si512())
    {}

    PA_FORCE_INLINE size_t count() const{
        return _mm512_reduce_add_epi32(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m512i pixel = _mm512_loadu_si512((const __m512i*)in);
        pixel = process_word(pixel);
        _mm512_storeu_si512((__m512i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m512i pixel = _mm512_maskz_loadu_epi32(mask.m, in);
        pixel = process_word(pixel);
        _mm512_mask_storeu_epi32(out, mask.m, pixel);
    }

private:
    PA_FORCE_INLINE __m512i process_word(__m512i pixel){
#if 0
        __mmask64 cmp64A = _mm512_cmpgt_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_cmpgt_epu8_mask(pixel, m_maxs);
        __m512i mask = _mm512_movm_epi8(cmp64A | cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(mask, _mm512_setzero_si512());
#else
        __mmask64 cmp64A = _mm512_cmple_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_mask_cmple_epu8_mask(cmp64A, pixel, m_maxs);
        __m512i mask = _mm512_movm_epi8(cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(mask, _mm512_set1_epi32(-1));
#endif
        m_count = _mm512_mask_sub_epi32(m_count, cmp16, m_count, _mm512_set1_epi32(-1));
        cmp16 ^= m_invert;
        return _mm512_mask_blend_epi32(cmp16, m_replacement, pixel);
    }

private:
    const __m512i m_replacement;
    const __mmask16 m_invert;
    const __m512i m_mins;
    const __m512i m_maxs;
    __m512i m_count;
};



size_t filter_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row, uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert
){
    ImageFilterByMask_x64_AVX512 filter(mins, maxs, replacement, invert);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilterByMask_x64_AVX512>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}





class ImageFilter_RgbEuclidean_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    ImageFilter_RgbEuclidean_x64_AVX512(uint32_t expected, double max_euclidean_distance, uint32_t replacement, bool invert)
        : m_replacement(_mm512_set1_epi32(replacement))
        , m_invert(invert ? 0xffff : 0)
        , m_expected_ag(_mm512_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm512_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm512_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
        , m_count(_mm512_setzero_si512())
    {}

    PA_FORCE_INLINE size_t count() const{
        return _mm512_reduce_add_epi32(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m512i pixel = _mm512_loadu_si512((const __m512i*)in);
        pixel = process_word(pixel);
        _mm512_storeu_si512((__m512i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m512i pixel = _mm512_maskz_loadu_epi32(mask.m, in);
        pixel = process_word(pixel);
        _mm512_mask_storeu_epi32(out, mask.m, pixel);
    }

private:
    PA_FORCE_INLINE __m512i process_word(__m512i pixel){
        __m512i ag = _mm512_and_si512(_mm512_srli_epi16(pixel, 8), _mm512_set1_epi32(0x000000ff));
        __m512i rb = _mm512_and_si512(pixel, _mm512_set1_epi32(0x00ff00ff));

        ag = _mm512_sub_epi16(ag, m_expected_ag);
        rb = _mm512_sub_epi16(rb, m_expected_rb);

        __m512i g = _mm512_mullo_epi16(ag, ag);
        rb = _mm512_mullo_epi16(rb, rb);
        __m512i r = _mm512_srli_epi32(rb, 16);
        __m512i b = _mm512_and_si512(rb, _mm512_set1_epi32(0x0000ffff));

        __m512i sum_sqr = _mm512_add_epi32(r, g);
        sum_sqr = _mm512_add_epi32(sum_sqr, b);

        __mmask16 cmp = _mm512_cmpgt_epi32_mask(m_distance_squared, sum_sqr);

        m_count = _mm512_mask_sub_epi32(m_count, cmp, m_count, _mm512_set1_epi32(-1));
        cmp ^= m_invert;
        return _mm512_mask_blend_epi32(cmp, m_replacement, pixel);
    }

private:
    const __m512i m_replacement;
    const __mmask16 m_invert;
    const __m512i m_expected_ag;
    const __m512i m_expected_rb;
    const __m512i m_distance_squared;
    __m512i m_count;
};
size_t filter_rgb32_euclidean_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t expected, double max_euclidean_distance,
    uint32_t replacement, bool invert
){
    ImageFilter_RgbEuclidean_x64_AVX512 filter(expected, max_euclidean_distance, replacement, invert);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}









class ToBlackWhite_RgbRange_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    ToBlackWhite_RgbRange_x64_AVX512(uint32_t mins, uint32_t maxs, bool in_range_black)
        : m_in_range_black(in_range_black ? 0xffff : 0)
        , m_mins(_mm512_set1_epi32(mins))
        , m_maxs(_mm512_set1_epi32(maxs))
        , m_count(_mm512_setzero_si512())
    {}

    PA_FORCE_INLINE size_t count() const{
        return _mm512_reduce_add_epi32(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m512i pixel = _mm512_loadu_si512((const __m512i*)in);
        pixel = process_word(pixel);
        _mm512_storeu_si512((__m512i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m512i pixel = _mm512_maskz_loadu_epi32(mask.m, in);
        pixel = process_word(pixel);
        _mm512_mask_storeu_epi32(out, mask.m, pixel);
    }

private:
    PA_FORCE_INLINE __m512i process_word(__m512i pixel){
#if 0
        __mmask64 cmp64A = _mm512_cmpgt_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_cmpgt_epu8_mask(pixel, m_maxs);
        __m512i mask = _mm512_movm_epi8(cmp64A | cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(mask, _mm512_setzero_si512());
#else
        __mmask64 cmp64A = _mm512_cmple_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_mask_cmple_epu8_mask(cmp64A, pixel, m_maxs);
        __m512i mask = _mm512_movm_epi8(cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(mask, _mm512_set1_epi32(-1));
#endif
        m_count = _mm512_mask_sub_epi32(m_count, cmp16, m_count, _mm512_set1_epi32(-1));
        cmp16 ^= m_in_range_black;
        return _mm512_mask_blend_epi32(cmp16, _mm512_set1_epi32(0xff000000), _mm512_set1_epi32(-1));
    }

private:
    const __mmask16 m_in_range_black;
    const __m512i m_mins;
    const __m512i m_maxs;
    __m512i m_count;
};



size_t to_blackwhite_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    ToBlackWhite_RgbRange_x64_AVX512 filter(mins, maxs, in_range_black);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void to_blackwhite_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    to_blackwhite_rbg32<ToBlackWhite_RgbRange_x64_AVX512>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}














}
}
#endif
